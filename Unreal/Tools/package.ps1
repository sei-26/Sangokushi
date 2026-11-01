param(
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.7',
    [ValidateSet('Development','Shipping')][string]$Configuration = 'Development',
    [string]$OutputRoot,
    [switch]$SkipRulesBuild
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$repoRoot = Split-Path $projectRoot -Parent
$projectFile = Join-Path $projectRoot 'GunyuTactics.uproject'
$uat = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
if (!(Test-Path -LiteralPath $uat)) { throw "Unreal automation tool missing: $uat" }
if (!$OutputRoot) { $OutputRoot = Join-Path $repoRoot "Releases\GunyuSenzen-$Configuration" }
$OutputRoot = [IO.Path]::GetFullPath($OutputRoot)
if (!$SkipRulesBuild) { & (Join-Path $PSScriptRoot 'build.ps1') -EngineRoot $EngineRoot -SkipEditor }
if (!(Test-Path -LiteralPath (Join-Path $projectRoot 'Intermediate\Rules\sangokushi_rules.lib'))) {
    throw 'Build the shared rules before packaging.'
}
$uatArguments = @(
    'BuildCookRun', '-installed', '-nop4', "-project=$projectFile",
    '-unattended', '-utf8output', '-nocompileeditor',
    '-platform=Win64', "-clientconfig=$Configuration", '-build', '-cook',
    '-stage', '-pak', '-prereqs', '-archive', '-nodebuginfo',
    "-archivedirectory=$OutputRoot", '-UbtArgs=-CompilerVersion=14.44.35207'
)
& $uat @uatArguments
if ($LASTEXITCODE -ne 0) { throw "BuildCookRun failed with exit code $LASTEXITCODE" }
$executable = Join-Path $OutputRoot 'Windows\GunyuTactics.exe'
if (!(Test-Path -LiteralPath $executable)) { throw "Packaged executable missing: $executable" }
$launch = @'
@echo off
start "" "%~dp0Windows\GunyuTactics.exe" -fullscreen
'@
[IO.File]::WriteAllText((Join-Path $OutputRoot 'Play.cmd'), $launch, [Text.Encoding]::ASCII)
$guide = @'
群雄戦線 — 名もなき軍師 / 序章・試遊ビルド

Play.cmd でフルスクリーン起動。Unreal Editor は不要です。
フォルダー全体を保ったまま使ってください。

会話：クリック / Enter で送る。軍議では三人を押して提案を聞く。
出陣：採る策と費用を確認し、「この策で、出陣だ！」。
合戦：武将を左クリック、右クリックで移動。「戦法を選ぶ」で単独戦法・連鎖。
「武将たちに任せる」で自動判断。いつでも自分の指揮へ戻せます。
記録：F6。カメラ：ホイールで拡大、中ドラッグで回転、Homeで全景。

今回の範囲は黄巾の乱を背景とする創作序章・三戦です。
長期の歴史戦役、仕官先の選択、完成版の美術は制作途中です。
フォントのライセンスは Windows/GunyuTactics/Content/UI/Fonts/OFL.txt に収録。
'@
[IO.File]::WriteAllText((Join-Path $OutputRoot 'はじめに.txt'), $guide, [Text.UTF8Encoding]::new($true))
Write-Output "PACKAGED_GAME=$executable"
