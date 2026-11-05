param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.7', [switch]$SkipEditor)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$repoRoot = Split-Path $projectRoot -Parent
$projectFile = Join-Path $projectRoot 'GunyuTactics.uproject'
$vsRoot = 'C:\Program Files\Microsoft Visual Studio\18\Community'
$cmakeExe = Join-Path $vsRoot 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$vcvars = Join-Path $vsRoot 'VC\Auxiliary\Build\vcvars64.bat'
foreach ($required in @($cmakeExe, $vcvars, (Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat'))) {
    if (!(Test-Path -LiteralPath $required)) { throw "Required tool missing: $required" }
}
$rulesDir = Join-Path $projectRoot 'Intermediate\Rules'
New-Item -ItemType Directory -Force -Path $rulesDir | Out-Null
# Keep UE and the shared library on the same native class layout, even if a cached include dependency is missing.
$headerState = (Get-ChildItem -LiteralPath (Join-Path $repoRoot 'NewGame') -File -Recurse |
    Where-Object { $_.Extension -in '.hpp','.h','.inl' } | Sort-Object FullName |
    ForEach-Object { $_.FullName.Substring($repoRoot.Length + 1) + ':' + (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash }) -join "`n"
$headerStamp = Join-Path $rulesDir 'native-headers.txt'
$headersChanged = !(Test-Path -LiteralPath $headerStamp)
if (!$headersChanged) { $headersChanged = [IO.File]::ReadAllText($headerStamp) -cne $headerState }
$cleanRules = if ($headersChanged) { ' --clean-first' } else { '' }
Write-Output "RULES_HEADER_CLEAN=$headersChanged"
# Use the same supported MSVC toolset for the rules and Unreal's STL boundary.
$batch = @"
@echo off
call "$vcvars" -vcvars_ver=14.44 >nul
if errorlevel 1 exit /b 1
"$cmakeExe" -S "$repoRoot" -B "$rulesDir" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL
if errorlevel 1 exit /b 1
"$cmakeExe" --build "$rulesDir"$cleanRules
exit /b %errorlevel%
"@
$batchPath = Join-Path $rulesDir 'build-rules.cmd'
[IO.File]::WriteAllText($batchPath, $batch, [Text.Encoding]::Default)
& cmd.exe /d /c "`"$batchPath`""
if ($LASTEXITCODE -ne 0) { throw 'Shared rule build failed' }
[IO.File]::WriteAllText($headerStamp, $headerState, [Text.UTF8Encoding]::new($false))
if (!$SkipEditor) {
    & (Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat') GunyuTacticsEditor Win64 Development "-Project=$projectFile" -WaitMutex -NoHotReloadFromIDE '-CompilerVersion=14.44.35207'
    if ($LASTEXITCODE -ne 0) { throw 'Unreal Editor module build failed' }
}
