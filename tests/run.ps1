$ErrorActionPreference = 'Stop'
$taskRoot = Split-Path $PSScriptRoot -Parent
$taskSdk = $env:SIV3D_0_6_16
if (-not $taskSdk -or -not (Test-Path -LiteralPath "$taskSdk/include/Siv3D.hpp")) {
    throw 'Set SIV3D_0_6_16 to the OpenSiv3D 0.6.16 SDK directory.'
}
$taskVsWhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$taskVs = & $taskVsWhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $taskVs) { throw 'Visual Studio C++ tools were not found.' }
$taskVcVars = Join-Path $taskVs 'VC/Auxiliary/Build/vcvars64.bat'
Push-Location $taskRoot
try {
    New-Item -ItemType Directory -Path 'Intermediate' -Force | Out-Null
    $taskCore = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 tests\TerritoryMapTests.cpp /Fe:Intermediate\TerritoryMapTests.exe /Fo:Intermediate\TerritoryMapTests.obj && Intermediate\TerritoryMapTests.exe"
    & cmd /c $taskCore
    if ($LASTEXITCODE -ne 0) { throw 'Territory tests failed.' }
    $taskCampaign = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 tests\CampaignTests.cpp /Fe:Intermediate\CampaignTests.exe /Fo:Intermediate\CampaignTests.obj && Intermediate\CampaignTests.exe"
    & cmd /c $taskCampaign
    if ($LASTEXITCODE -ne 0) { throw 'Campaign tests failed.' }
    $taskSave = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++latest /MT /D_ENABLE_EXTENDED_ALIGNED_STORAGE /I `"$taskSdk\include`" /I `"$taskSdk\include\ThirdParty`" tests\SaveDataTests.cpp /Fe:Intermediate\SaveDataTests.exe /Fo:Intermediate\SaveDataTests.obj /link /LIBPATH:`"$taskSdk\lib\Windows`" advapi32.lib shell32.lib gdi32.lib user32.lib ole32.lib winmm.lib && Intermediate\SaveDataTests.exe"
    & cmd /c $taskSave
    if ($LASTEXITCODE -ne 0) { throw 'Save tests failed.' }
}
finally { Pop-Location }
