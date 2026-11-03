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
軍議：← / 前の台詞で読み返す。「文字：一括 / 順次」で文字送りを変更。
軍議の会話中：1・2は画面の二つの返答。配置案は地図で確認し、採用するまで担当を変えません。
出陣：採る策と費用を確認し、「この策で、出陣だ！」。
合戦：見えている敵を選び、右の「次の一手」で効果・代償を比べて発動。
二つの烽火：自由な隊を選び、北・南の援護を比較。選ぶと行軍→固守を指示し、再開で進みます。
武将を左クリック、右クリックで移動。「詳しい采配」で戦法・連鎖を組めます。
上部に今の狙いを表示。「目標へ / U」で車の前路や守る場所を見る。
「護送を任せる / 武将たちに任せる」で自動判断。いつでも自分の指揮へ戻せます。
合戦の速さ：右の速度ボタンで0.5・1・2・3倍。戦法の演出中も右下で変更。
記録：F6。カメラ：ホイールで拡大、中ドラッグで回転、Homeで全景。

今回の範囲は黄巾の乱を背景とする創作序章・三戦です。
長期の歴史戦役、仕官先の選択、完成版の美術は制作途中です。
フォントのライセンスは Windows/GunyuTactics/Content/UI/Fonts/OFL.txt に収録。
'@
[IO.File]::WriteAllText((Join-Path $OutputRoot 'はじめに.txt'), $guide, [Text.UTF8Encoding]::new($true))
Write-Output "PACKAGED_GAME=$executable"
