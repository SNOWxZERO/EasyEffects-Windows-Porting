#include "ConvolverModule.h"

namespace eeval {

ConvolverModule::ConvolverModule() {}

void ConvolverModule::prepare(const juce::dsp::ProcessSpec& spec) {
    convolverNode.prepare(spec);
    prepareBuffers(spec);
}

void ConvolverModule::processInternal(juce::AudioBuffer<float>& buffer) {
    if (!hasIRLoaded) return; // Pass through if no IR loaded

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    convolverNode.process(context);
}

void ConvolverModule::reset() {
    convolverNode.reset();
}

void ConvolverModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

void ConvolverModule::loadImpulseResponse(const juce::File& file) {
    if (!file.existsAsFile()) {
        // File missing - handle gracefully
        irPath.clear();
        irName.clear();
        hasIRLoaded = false;
        
        // Only notify if they actually tried to load a specific file (not when clearing)
        if (file != juce::File()) {
            juce::MessageManager::callAsync([file]() {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Impulse Response Missing",
                    "Could not find IR file: " + file.getFullPathName() + "\nConvolver bypassed."
                );
            });
        }
        return;
    }
    
    irPath = file.getFullPathName().toStdString();
    irName = file.getFileName().toStdString();

    // Async off-thread loading to prevent UI blocking
    juce::Thread::launch([this, file]() {
        convolverNode.loadImpulseResponse(
            file,
            juce::dsp::Convolution::Stereo::yes,
            juce::dsp::Convolution::Trim::yes,
            0,
            juce::dsp::Convolution::Normalise::yes
        );
        hasIRLoaded = true;
    });
}

juce::var ConvolverModule::getState() {
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("ir_path", juce::String(irPath));
    obj->setProperty("ir_name", juce::String(irName));
    return juce::var(obj.get());
}

void ConvolverModule::setState(const juce::var& state) {
    if (auto* obj = state.getDynamicObject()) {
        juce::String path = obj->getProperty("ir_path").toString();
        if (path.isNotEmpty()) {
            loadImpulseResponse(juce::File(path));
        } else {
            irPath.clear();
            irName.clear();
            hasIRLoaded = false;
        }
    }
}

const std::string& ConvolverModule::getModuleId() const { return moduleId; }
const std::string& ConvolverModule::getName() const { return name; }

} // namespace eeval
