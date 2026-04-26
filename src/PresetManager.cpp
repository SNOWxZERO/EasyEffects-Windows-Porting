#include "PresetManager.h"
#include "PluginProcessor.h"
#include "dsp/EffectRegistry.h"

namespace eeval {

PresetManager::PresetManager(EasyEffectsAudioProcessor& p) : processor(p) {}

juce::File PresetManager::getPresetFolder(bool isGlobal) {
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("EasyEffects")
        .getChildFile(isGlobal ? "Presets" : "ModulePresets");
    
    if (!dir.exists()) dir.createDirectory();
    return dir;
}

void PresetManager::saveGlobalPreset(const std::string& name) {
    if (name.empty()) return;
    auto file = getPresetFolder(true).getChildFile(name + ".json");
    
    // Overwrite: delete existing file first to avoid appending
    if (file.existsAsFile()) file.deleteFile();
    
    auto json = serializeChain();
    if (auto stream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream())) {
        juce::JSON::writeToStream(*stream, json);
    }
    currentPresetName = name;
    dirty = false;
}

void PresetManager::loadGlobalPreset(const std::string& name) {
    auto file = getPresetFolder(true).getChildFile(name + ".json");
    if (!file.existsAsFile()) return;

    auto json = juce::JSON::parse(file);
    if (!json.isVoid()) {
        deserializeChain(json);
        currentPresetName = name;
        dirty = false;
    }
}

void PresetManager::deleteGlobalPreset(const std::string& name) {
    auto file = getPresetFolder(true).getChildFile(name + ".json");
    if (file.existsAsFile()) file.deleteFile();
    if (currentPresetName == name) {
        currentPresetName.clear();
        dirty = false;
    }
}

juce::var PresetManager::serializeChain() {
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty("version", 1);

    juce::Array<juce::var> slotsJson;
    auto slots = processor.getActiveSlots();

    for (const auto& slot : slots) {
        juce::DynamicObject::Ptr sObj = new juce::DynamicObject();
        sObj->setProperty("typeId", juce::String(slot.typeId));
        
        juce::DynamicObject::Ptr paramsObj = new juce::DynamicObject();
        std::string prefix = EffectRegistry::slotPrefix(slot.slotIndex);
        
        // Get all params for this type
        const auto* desc = EffectRegistry::findType(slot.typeId);
        if (desc) {
            for (const auto& p : desc->params) {
                std::string id = prefix + "." + slot.typeId + "." + p.suffix;
                if (auto* val = processor.parameters.getRawParameterValue(id))
                    paramsObj->setProperty(juce::String(p.suffix), val->load());
            }
            for (const auto& c : desc->choices) {
                std::string id = prefix + "." + slot.typeId + "." + c.suffix;
                if (auto* val = processor.parameters.getRawParameterValue(id))
                    paramsObj->setProperty(juce::String(c.suffix), val->load());
            }
        }
        
        // Slot mix
        std::string mixId = prefix + ".mix";
        if (auto* val = processor.parameters.getRawParameterValue(mixId))
            paramsObj->setProperty("mix", val->load());

        sObj->setProperty("params", paramsObj.get());
        slotsJson.add(sObj.get());
    }

    root->setProperty("slots", slotsJson);
    return root.get();
}

void PresetManager::deserializeChain(const juce::var& json) {
    auto* root = json.getDynamicObject();
    if (!root) return;

    auto slots = root->getProperty("slots").getArray();
    
    // 1. Clear current chain robustly (since removeEffect compacts the chain)
    while (processor.getActiveSlots().size() > 0) {
        processor.removeEffect(0);
    }

    // 2. Rebuild
    for (int i = 0; i < slots->size(); ++i) {
        auto s = slots->getReference(i);
        std::string typeId = s.getProperty("typeId", "").toString().toStdString();
        
        int newIdx = processor.addEffect(typeId);
        if (newIdx != -1) {
            auto params = s.getProperty("params", juce::var());
            if (auto* pObj = params.getDynamicObject()) {
                std::string prefix = EffectRegistry::slotPrefix(newIdx);
                
                for (auto& prop : pObj->getProperties()) {
                    std::string suffix = prop.name.toString().toStdString();
                    std::string fullId;
                    if (suffix == "mix") fullId = prefix + ".mix";
                    else fullId = prefix + "." + typeId + "." + suffix;

                    if (auto* p = processor.parameters.getParameter(fullId))
                        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1((float)prop.value));
                }
            }
        }
    }
}

juce::StringArray PresetManager::getGlobalPresetList() {
    juce::StringArray list;
    auto folder = getPresetFolder(true);
    auto files = folder.findChildFiles(juce::File::findFiles, false, "*.json");
    for (auto& f : files) list.add(f.getFileNameWithoutExtension());
    return list;
}

void PresetManager::saveModulePreset(int slotIndex, const std::string& name) {
    auto slots = processor.getActiveSlots();
    for (const auto& slot : slots) {
        if (slot.slotIndex == slotIndex) {
            juce::DynamicObject::Ptr sObj = new juce::DynamicObject();
            sObj->setProperty("typeId", juce::String(slot.typeId));
            
            juce::DynamicObject::Ptr paramsObj = new juce::DynamicObject();
            std::string prefix = EffectRegistry::slotPrefix(slot.slotIndex);
            
            const auto* desc = EffectRegistry::findType(slot.typeId);
            if (desc) {
                for (const auto& p : desc->params) {
                    std::string id = prefix + "." + slot.typeId + "." + p.suffix;
                    if (auto* val = processor.parameters.getRawParameterValue(id))
                        paramsObj->setProperty(juce::String(p.suffix), val->load());
                }
                for (const auto& c : desc->choices) {
                    std::string id = prefix + "." + slot.typeId + "." + c.suffix;
                    if (auto* val = processor.parameters.getRawParameterValue(id))
                        paramsObj->setProperty(juce::String(c.suffix), val->load());
                }
            }
            
            // Slot mix
            std::string mixId = prefix + ".mix";
            if (auto* val = processor.parameters.getRawParameterValue(mixId))
                paramsObj->setProperty("mix", val->load());

            sObj->setProperty("params", paramsObj.get());

            auto file = getPresetFolder(false).getChildFile(slot.typeId).getChildFile(name + ".json");
            if (!file.getParentDirectory().exists()) file.getParentDirectory().createDirectory();

            if (auto stream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream())) {
                juce::JSON::writeToStream(*stream, juce::var(sObj.get()));
            }
            break;
        }
    }
}

void PresetManager::loadModulePreset(int slotIndex, const std::string& name) {
    auto slots = processor.getActiveSlots();
    for (const auto& slot : slots) {
        if (slot.slotIndex == slotIndex) {
            auto file = getPresetFolder(false).getChildFile(slot.typeId).getChildFile(name + ".json");
            if (!file.existsAsFile()) return;

            auto json = juce::JSON::parse(file);
            if (auto* pObj = json.getDynamicObject()) {
                auto params = pObj->getProperty("params");
                if (auto* paramData = params.getDynamicObject()) {
                    std::string prefix = EffectRegistry::slotPrefix(slotIndex);
                    for (auto& prop : paramData->getProperties()) {
                        std::string suffix = prop.name.toString().toStdString();
                        std::string fullId;
                        if (suffix == "mix") fullId = prefix + ".mix";
                        else fullId = prefix + "." + slot.typeId + "." + suffix;

                        if (auto* p = processor.parameters.getParameter(fullId))
                            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1((float)prop.value));
                    }
                }
            }
            break;
        }
    }
}

void PresetManager::deleteModulePreset(int slotIndex, const std::string& name) {
    auto slots = processor.getActiveSlots();
    for (const auto& slot : slots) {
        if (slot.slotIndex == slotIndex) {
            auto file = getPresetFolder(false).getChildFile(slot.typeId).getChildFile(name + ".json");
            if (file.existsAsFile()) file.deleteFile();
            break;
        }
    }
}

juce::StringArray PresetManager::getModulePresetList(int slotIndex) {
    auto slots = processor.getActiveSlots();
    for (const auto& slot : slots) {
        if (slot.slotIndex == slotIndex) {
            juce::StringArray list;
            auto folder = getPresetFolder(false).getChildFile(slot.typeId);
            auto files = folder.findChildFiles(juce::File::findFiles, false, "*.json");
            for (auto& f : files) list.add(f.getFileNameWithoutExtension());
            return list;
        }
    }
    return {};
}

} // namespace eeval
