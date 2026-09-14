$ErrorActionPreference = "Stop"

$showaRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$showaBuildDir = Join-Path $showaRoot "Build-v142-lpxml"
$showaToolsFile = Join-Path $showaRoot "Tools\CMakeCommon.cmake"

$showaCmakeCandidates = @(
    "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
    "C:\Program Files\CMake\bin\cmake.exe"
)
$showaCmake = $showaCmakeCandidates |
    Where-Object { Test-Path $_ } |
    Select-Object -First 1

if (-not $showaCmake) {
    throw "CMake was not found. Install the Visual Studio CMake component first."
}

if (-not (Test-Path $showaToolsFile)) {
    throw "Tools\CMakeCommon.cmake is missing. Extract this package over the existing ShowaAddonTemplate-ac25 clone so its Tools submodule is preserved."
}

$showaDevKit = "C:/Program Files/GRAPHISOFT/API Development Kit 25.3002/Support"
$showaArchicad = "C:/Program Files/GRAPHISOFT/ARCHICAD 25"

if (-not (Test-Path $showaDevKit)) {
    throw "Archicad 25 API Development Kit was not found at: $showaDevKit"
}

if (-not (Test-Path (Join-Path $showaArchicad "LP_XMLConverter.exe"))) {
    throw "LP_XMLConverter.exe was not found at: $showaArchicad"
}

& $showaCmake -S $showaRoot -B $showaBuildDir `
    -G "Visual Studio 18 2026" `
    -A x64 `
    -T v142 `
    "-DAC_API_DEVKIT_DIR=$showaDevKit" `
    -DAC_VERSION=25 `
    "-DLP_XML_CONVERTER_FOLDER=$showaArchicad"

if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed with exit code $LASTEXITCODE."
}

& $showaCmake --build $showaBuildDir --config Debug
if ($LASTEXITCODE -ne 0) {
    throw "CMake build failed with exit code $LASTEXITCODE."
}

$showaApx = Get-ChildItem $showaBuildDir -Filter "ShowaBridge.apx" -Recurse |
    Select-Object -First 1
if (-not $showaApx) {
    throw "Build finished but ShowaBridge.apx was not found."
}

Write-Host "BUILD OK: $($showaApx.FullName)"
