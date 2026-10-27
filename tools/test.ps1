param(
    [string]$BuildDir = "build_x64",
    [string]$Config = "",
    [string]$Filter = "",
    [switch]$NoBuild,
    [switch]$KnownBugs,
    [switch]$All
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
# The build tree lives next to the CMake source dir (repo root), not inside it:
#   <repoRoot>       = K:\game_wrk\rh\src
#   <repoRoot>\..\build_x64 = K:\game_wrk\rh\build_x64
$repoParent = Split-Path -Parent $repoRoot
if ([System.IO.Path]::IsPathRooted($BuildDir)) {
    $buildPath = $BuildDir
} else {
    $buildPath = Join-Path $repoParent $BuildDir
}

if (-not (Test-Path $buildPath)) {
    throw "Build directory not found: $buildPath"
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    throw "vswhere.exe not found. Install Visual Studio Build Tools."
}
$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$vcvars = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvars)) {
    throw "vcvars64.bat not found at $vcvars"
}

$targets = @("rh_unit_tests")
if (Test-Path (Join-Path $repoRoot "Tests\GpuTests\CMakeLists.txt")) {
    $targets += "rh_gpu_tests"
}

if (-not $NoBuild) {
    $buildCommand = "call `"$vcvars`" >nul && cd /D `"$repoRoot`" && cmake --build `"$buildPath`" --target $($targets -join ' ')"
    cmd /c $buildCommand
    if ($LASTEXITCODE -ne 0) { throw "Build failed" }
}

if ($KnownBugs) {
    $unitExe = Join-Path $buildPath "Tests\UnitTests\rh_unit_tests.exe"
    & $unitExe "-tc=known_bug/*"
    $gpuExe = Join-Path $buildPath "Tests\GpuTests\rh_gpu_tests.exe"
    if (Test-Path $gpuExe) { & $gpuExe "-tc=known_bug/*" }
    exit 0
}

$ctestArgs = @("--test-dir", $buildPath, "--output-on-failure")
if ($Config) { $ctestArgs += @("-C", $Config) }
if ($Filter) { $ctestArgs += @("-R", $Filter) }
ctest @ctestArgs
$ctestExit = $LASTEXITCODE

if ($All) {
    $unitExe = Join-Path $buildPath "Tests\UnitTests\rh_unit_tests.exe"
    & $unitExe "-tc=shader/all_*"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

if ($ctestExit -ne 0) { exit $ctestExit }
