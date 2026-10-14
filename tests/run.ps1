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
        'NewGame\TacticChain.cpp',
        'NewGame\StrategistJourney.cpp',
        'NewGame\StrategistJourneyAI.cpp',
        'NewGame\StrategistBattle.cpp',
        'NewGame\BattleIntelligence.cpp',
        'NewGame\StrategistBattleIntelligence.cpp',
        'NewGame\StrategistBattleOperations.cpp',
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
        'NewGame\CampaignTraining.cpp',
        'NewGame\CampaignGovernance.cpp',
        'NewGame\CampaignDevelopment.cpp',
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
    $taskPreview = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\PreviewModeTests.cpp /Fe:Intermediate\PreviewModeTests.exe /Fo:Intermediate\PreviewModeTests.obj && Intermediate\PreviewModeTests.exe"
    & cmd /c $taskPreview
    if ($LASTEXITCODE -ne 0) { throw 'Preview mode tests failed.' }
    $taskCampaign = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\CampaignTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\CampaignTests.exe /Fo:Intermediate\CampaignTests.obj /link /STACK:8388608 && Intermediate\CampaignTests.exe"
    & cmd /c $taskCampaign
    if ($LASTEXITCODE -ne 0) { throw 'Campaign tests failed.' }
    $taskStrategist = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\StrategistBattleTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\StrategistBattleTests.exe /Fo:Intermediate\StrategistBattleTests.obj /link /STACK:8388608 && Intermediate\StrategistBattleTests.exe"
    & cmd /c $taskStrategist
    if ($LASTEXITCODE -ne 0) { throw 'Strategist battle tests failed.' }
    $taskOperations = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\StrategistOperationsTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\StrategistOperationsTests.exe /Fo:Intermediate\StrategistOperationsTests.obj /link /STACK:8388608 && Intermediate\StrategistOperationsTests.exe"
    & cmd /c $taskOperations
    if ($LASTEXITCODE -ne 0) { throw 'Strategist intelligence and operation tests failed.' }
    $taskChains = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\TacticChainTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\TacticChainTests.exe /Fo:Intermediate\TacticChainTests.obj /link /STACK:8388608 && Intermediate\TacticChainTests.exe"
    & cmd /c $taskChains
    if ($LASTEXITCODE -ne 0) { throw 'Tactic chain and journey continuity tests failed.' }
    $taskBudget = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\CommandBudgetTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\CommandBudgetTests.exe /Fo:Intermediate\CommandBudgetTests.obj /link /STACK:8388608 && Intermediate\CommandBudgetTests.exe"
    & cmd /c $taskBudget
    if ($LASTEXITCODE -ne 0) { throw 'Command budget tests failed.' }
    $taskDevelopment = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\DevelopmentTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\DevelopmentTests.exe /Fo:Intermediate\DevelopmentTests.obj /link /STACK:8388608 && Intermediate\DevelopmentTests.exe"
    & cmd /c $taskDevelopment
    if ($LASTEXITCODE -ne 0) { throw 'Development and policy tests failed.' }

    $taskHex = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\HexTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\HexTests.exe /Fo:Intermediate\HexTests.obj /link /STACK:8388608 && Intermediate\HexTests.exe"
    & cmd /c $taskHex
    if ($LASTEXITCODE -ne 0) { throw 'Hex tests failed.' }
    $taskAtlas = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\ChinaAtlasTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\ChinaAtlasTests.exe /Fo:Intermediate\ChinaAtlasTests.obj /link /STACK:8388608 && Intermediate\ChinaAtlasTests.exe"
    & cmd /c $taskAtlas
    if ($LASTEXITCODE -ne 0) { throw 'China atlas projection tests failed.' }
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
    $taskBattlefieldMoments = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\BattlefieldMomentsTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\BattlefieldMomentsTests.exe /Fo:Intermediate\BattlefieldMomentsTests.obj /link /STACK:8388608 && Intermediate\BattlefieldMomentsTests.exe"
    & cmd /c $taskBattlefieldMoments
    if ($LASTEXITCODE -ne 0) { throw 'Battlefield presentation event tests failed.' }
    $taskFormation = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\FormationTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\FormationTests.exe /Fo:Intermediate\FormationTests.obj /link /STACK:8388608 && Intermediate\FormationTests.exe"
    & cmd /c $taskFormation
    if ($LASTEXITCODE -ne 0) { throw 'Formation contribution tests failed.' }
    $taskTraining = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\TrainingTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\TrainingTests.exe /Fo:Intermediate\TrainingTests.obj /link /STACK:8388608 && Intermediate\TrainingTests.exe"
    & cmd /c $taskTraining
    if ($LASTEXITCODE -ne 0) { throw 'Training tests failed.' }
    $taskInformation = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\InformationTests.cpp Intermediate\tests\CampaignCore.lib /Fe:Intermediate\InformationTests.exe /Fo:Intermediate\InformationTests.obj /link /STACK:8388608 && Intermediate\InformationTests.exe"
    & cmd /c $taskInformation
    if ($LASTEXITCODE -ne 0) { throw 'Information tests failed.' }
    $taskStory = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\StoryTests.cpp Intermediate\tests\StoryCore.lib /Fe:Intermediate\StoryTests.exe /Fo:Intermediate\StoryTests.obj && Intermediate\StoryTests.exe"
    & cmd /c $taskStory
    if ($LASTEXITCODE -ne 0) { throw 'Story tests failed.' }
    $taskMusic = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++17 /MT /utf-8 tests\MusicCueTests.cpp /Fe:Intermediate\MusicCueTests.exe /Fo:Intermediate\MusicCueTests.obj && Intermediate\MusicCueTests.exe"
    & cmd /c $taskMusic
    if ($LASTEXITCODE -ne 0) { throw 'Music cue tests failed.' }
    $taskSave = "call `"$taskVcVars`" >nul && cl /nologo /EHsc /std:c++latest /MT /utf-8 /D_ENABLE_EXTENDED_ALIGNED_STORAGE /I `"$taskSdk\include`" /I `"$taskSdk\include\ThirdParty`" tests\SaveDataTests.cpp NewGame\CampaignSave.cpp NewGame\CampaignLoad.cpp NewGame\StorySave.cpp NewGame\StoryLoad.cpp Intermediate\tests\CampaignCore.lib Intermediate\tests\StoryCore.lib /Fe:Intermediate\SaveDataTests.exe /Fo:Intermediate\tests\save\ /link /STACK:8388608 /LIBPATH:`"$taskSdk\lib\Windows`" advapi32.lib shell32.lib gdi32.lib user32.lib ole32.lib winmm.lib && Intermediate\SaveDataTests.exe"
    & cmd /c $taskSave
    if ($LASTEXITCODE -ne 0) { throw 'Save tests failed.' }
}
finally { Pop-Location }
