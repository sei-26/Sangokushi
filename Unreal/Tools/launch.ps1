param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.7', [switch]$Editor, [switch]$BattlePreview, [switch]$MapPreview, [switch]$RealtimePreview, [switch]$CouncilPreview)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$projectFile = Join-Path $projectRoot 'GunyuTactics.uproject'
if (!(Test-Path -LiteralPath (Join-Path $projectRoot 'Binaries\Win64\UnrealEditor-GunyuTactics.dll'))) {
    throw 'Run Unreal/Tools/build.ps1 first.'
}
if (!(Test-Path -LiteralPath (Join-Path $projectRoot 'Content\Maps\TacticalTrial.umap'))) {
    throw 'Run Tools/bootstrap.py in the Unreal Editor first; see Unreal/README.md.'
}
$arguments = @(('"' + $projectFile + '"'))
if (!$Editor) { $arguments += @('-game', '-fullscreen') }
if (!$Editor -and ($BattlePreview -or $MapPreview -or $RealtimePreview)) { $arguments += '-GunyuBattlePreview' }
if (!$Editor -and $MapPreview) { $arguments += '-GunyuMapOverview' }
if (!$Editor -and $RealtimePreview) { $arguments += '-GunyuRealtime' }
if (!$Editor -and $CouncilPreview) { $arguments += @('-GunyuCouncilPreview','-GunyuRealtime') }
# This script launches the interactive editor/game for the user.
Start-Process -FilePath (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe') -ArgumentList $arguments -WorkingDirectory $projectRoot -WindowStyle Normal
