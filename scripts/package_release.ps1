param(
    [string]$Version = "v0.1.0-alpha",
    [switch]$NoZip
)

$RootDir = (Get-Item -Path "$PSScriptRoot\..").FullName
$ReleaseFolder = Join-Path $RootDir "releases"
if (-not (Test-Path $ReleaseFolder)) {
    New-Item -ItemType Directory -Force -Path $ReleaseFolder | Out-Null
}

$ReleaseName = "EasyEffects-Windows-$Version"
$ReleaseContentDir = Join-Path $ReleaseFolder $ReleaseName
$ZipPath = Join-Path $ReleaseFolder "$ReleaseName.zip"

Write-Host "Packaging Release: $Version"

# 1. Clean previous release folders
if (Test-Path $ReleaseContentDir) {
    Remove-Item -Recurse -Force $ReleaseContentDir
}
if (Test-Path $ZipPath) {
    Remove-Item -Force $ZipPath
}

New-Item -ItemType Directory -Force -Path $ReleaseContentDir | Out-Null

# 2. Define source paths
$ExeSource = Join-Path $RootDir "build\EasyEffects_artefacts\Release\Standalone\EasyEffects.exe"
$Vst3Source = Join-Path $RootDir "build\EasyEffects_artefacts\Release\VST3\EasyEffects.vst3"
$QuickStart = Join-Path $RootDir "QUICK_START.md"
$License = Join-Path $RootDir "LICENSE"

# 3. Check if builds exist
if (-not (Test-Path $ExeSource)) {
    Write-Error "Standalone executable not found at $ExeSource. Please build in Release mode first."
    exit 1
}

if (-not (Test-Path $Vst3Source)) {
    Write-Warning "VST3 plugin not found at $Vst3Source. It will be skipped."
}

# 4. Copy files to release dir
Write-Host "Copying files to $ReleaseContentDir..."
Copy-Item $ExeSource -Destination $ReleaseContentDir
if (Test-Path $Vst3Source) {
    Copy-Item -Recurse $Vst3Source -Destination $ReleaseContentDir
}
Copy-Item $QuickStart -Destination $ReleaseContentDir
Copy-Item $License -Destination $ReleaseContentDir

# 5. Create Zip Archive (if requested)
if (-not $NoZip) {
    Write-Host "Creating Zip Archive..."
    Compress-Archive -Path "$ReleaseContentDir\*" -DestinationPath $ZipPath -Force
    
    # 6. Cleanup temp release folder if zipping
    Remove-Item -Recurse -Force $ReleaseContentDir
    Write-Host "Success! Release packaged at: $ZipPath"
} else {
    Write-Host "Success! Release folder prepared at: $ReleaseContentDir"
}
