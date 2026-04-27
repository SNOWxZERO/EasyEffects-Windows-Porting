param(
    [string]$Version = "v0.1.0-alpha"
)

$RootDir = (Get-Item -Path ".\").FullName
$ReleaseFolder = Join-Path $RootDir "releases"
if (-not (Test-Path $ReleaseFolder)) {
    New-Item -ItemType Directory -Force -Path $ReleaseFolder | Out-Null
}

$ReleaseTempDir = Join-Path $RootDir "release_temp"
$ZipName = "EasyEffects-Windows-$Version.zip"
$ZipPath = Join-Path $ReleaseFolder $ZipName

Write-Host "Packaging Release: $Version"

# 1. Clean previous release temp folder
if (Test-Path $ReleaseTempDir) {
    Remove-Item -Recurse -Force $ReleaseTempDir
}
if (Test-Path $ZipPath) {
    Remove-Item -Force $ZipPath
}

New-Item -ItemType Directory -Force -Path $ReleaseTempDir | Out-Null

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
Write-Host "Copying files..."
Copy-Item $ExeSource -Destination $ReleaseTempDir
if (Test-Path $Vst3Source) {
    Copy-Item -Recurse $Vst3Source -Destination $ReleaseTempDir
}
Copy-Item $QuickStart -Destination $ReleaseTempDir
Copy-Item $License -Destination $ReleaseTempDir

# 5. Create Zip Archive
Write-Host "Creating Zip Archive..."
Compress-Archive -Path "$ReleaseTempDir\*" -DestinationPath $ZipPath -Force

# 6. Cleanup temp release folder
Remove-Item -Recurse -Force $ReleaseTempDir

Write-Host "Success! Release packaged at: $ZipPath"
