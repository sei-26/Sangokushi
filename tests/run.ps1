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
    New-Item -ItemType Directory -Path 'Intermediate/tests/core','Intermediate/tests/save' -Force | Out-Null
    $taskCampaignFiles = @(
        'NewGame\Campaign.cpp',
 'NewGame\CampaignRoster.cpp',
        'NewGame\CampaignRegions.cpp',
        'NewGame\CampaignAI.cpp',
        'NewGame\CampaignAIPlanning.cpp',
        'NewGame\CampaignAIArmies.cpp',
        'NewGame\CampaignAIOperations.cpp',
        'NewGame\CampaignAssignments.cpp',
        'NewGame\CampaignLogistics.cpp',
        'NewGame\CampaignCombat.cpp',
        'NewGame\CampaignBattleRules.cpp',
        'NewGame\CampaignBattlefield.cpp',
        'NewGame\CampaignDailyCombat.cpp',
        'NewGame\CampaignDiplomacy.cpp',
        'NewGame\CampaignEconomy.cpp',
        'NewGame\CampaignGovernance.cpp',
        'NewGame\CampaignOfficers.cpp',
        'NewGame\CampaignOrders.cpp',
        'NewGame\CampaignReturn.cpp',
        'NewGame\CampaignSupply.cpp',
        'NewGame\CampaignTurn.cpp'
    )
    $taskCampaignSources = $taskCampaignFiles -join ' '
    $taskCampaignObjects = ($taskCampaignFiles | ForEach-Object { 'Intermediate\tests\core\' + [IO.Path]::GetFileNameWithoutExtension($_) + '.obj' }) -join ' '
    $taskStoryFiles = @(
        'NewGame\HeroStory.cpp',
        'NewGame\HeroStoryBattle.cpp',
        'NewGame\HeroStoryAI.cpp',
        'NewGame\HeroStoryProgression.cpp',
        'NewGame\HeroStoryRelationships.cpp',
        'NewGame\HeroStorySkills.cpp',
        'NewGame\HeroStoryObjectives.cpp'
    )
    $taskStorySources = $taskStoryFiles -join ' '
    $taskStoryObjects = ($taskStoryFiles | ForEach-Object { 'Intermediate\tests\core\' + [IO.Path]::GetFileNameWithoutExtension($_) + '.obj' }) -join ' '
    $taskBuildCore = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 /c $taskCampaignSources $taskStorySources /Fo:Intermediate\tests\core\ && lib /nologo /OUT:Intermediate\tests\CampaignCore.lib $taskCampaignObjects && lib /nologo /OUT:Intermediate\tests\StoryCore.lib $taskStoryObjects"
    & cmd /c $taskBuildCore
    if ($LASTEXITCODE -ne 0) { throw 'Simulation library compilation failed.' }
    $taskCampaign = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\CampaignTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\CampaignTests.exe /Fo:Intermediate\CampaignTests.obj /link /STACK:8388608 && Intermediate\CampaignTests.exe"
    & cmd /c $taskCampaign
    if ($LASTEXITCODE -ne 0) { throw 'Campaign tests failed.' }
    $taskHex = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\HexTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\HexTests.exe /Fo:Intermediate\HexTests.obj /link /STACK:8388608 && Intermediate\HexTests.exe"
    & cmd /c $taskHex
    if ($LASTEXITCODE -ne 0) { throw 'Hex tests failed.' }
    $taskRegions = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\RegionTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\RegionTests.exe /Fo:Intermediate\RegionTests.obj /link /STACK:8388608 && Intermediate\RegionTests.exe"
    & cmd /c $taskRegions
    if ($LASTEXITCODE -ne 0) { throw 'Region tests failed.' }
    $taskBattlefield = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\BattlefieldTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\BattlefieldTests.exe /Fo:Intermediate\BattlefieldTests.obj /link /STACK:8388608 && Intermediate\BattlefieldTests.exe"
    & cmd /c $taskBattlefield
    if ($LASTEXITCODE -ne 0) { throw 'Battlefield tests failed.' }
    $taskAI = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\AITests.cpp Intermediate\tests\CampaignCore.lib Intermediate\tests\StoryCore.lib /Fe:Intermediate\AITests.exe /Fo:Intermediate\AITests.obj /link /STACK:8388608 && Intermediate\AITests.exe"
    & cmd /c $taskAI
    if ($LASTEXITCODE -ne 0) { throw 'AI tests failed.' }
    $taskWorld = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\WorldMapTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\WorldMapTests.exe /Fo:Intermediate\WorldMapTests.obj && Intermediate\WorldMapTests.exe"
    & cmd /c $taskWorld
    if ($LASTEXITCODE -ne 0) { throw 'World map tests failed.' }
    $taskGovernance = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\GovernanceTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\GovernanceTests.exe /Fo:Intermediate\GovernanceTests.obj /link /STACK:8388608 && Intermediate\GovernanceTests.exe"
    & cmd /c $taskGovernance
    if ($LASTEXITCODE -ne 0) { throw 'Governance tests failed.' }
    $taskInformation = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\InformationTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\InformationTests.exe /Fo:Intermediate\InformationTests.obj /link /STACK:8388608 && Intermediate\InformationTests.exe"
    & cmd /c $taskInformation
    if ($LASTEXITCODE -ne 0) { throw 'Information tests failed.' }
    $taskStory = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\StoryTests.cpp Intermediate\tests\StoryCore.lib /Fe:Intermediate\StoryTests.exe /Fo:Intermediate\StoryTests.obj && Intermediate\StoryTests.exe"
    & cmd /c $taskStory
    if ($LASTEXITCODE -ne 0) { throw 'Story tests failed.' }
    $taskSave = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++latest /MT /utf-8 /D_ENABLE_EXTENDED_ALIGNED_STORAGE /I `"$taskSdk\include`" /I `"$taskSdk\include\ThirdParty`" tests\SaveDataTests.cpp NewGame\CampaignSave.cpp NewGame\CampaignLoad.cpp NewGame\StorySave.cpp NewGame\StoryLoad.cpp Intermediate\tests\CampaignCore.lib Intermediate\tests\StoryCore.lib /Fe:Intermediate\SaveDataTests.exe /Fo:Intermediate\tests\save\ /link /STACK:8388608 /LIBPATH:`"$taskSdk\lib\Windows`" advapi32.lib shell32.lib gdi32.lib user32.lib ole32.lib winmm.lib && Intermediate\SaveDataTests.exe"
    & cmd /c $taskSave
    if ($LASTEXITCODE -ne 0) { throw 'Save tests failed.' }
}
finally { Pop-Location }
