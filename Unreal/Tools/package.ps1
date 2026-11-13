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
出陣後は武将カードで軍議の担当を確認できます。同時進行で自由に動かせる隊が一人なら、その隊を選択して始めます。
担当地点へ / P：選んだ隊の軍議の行き先を見る。目標へ / U：現在の任務の目標を見る。
担当の札では、残りの行軍・進路待ち・到着を確認できます。原因の隊を確認できた時は「道をふさぐ隊を見る」。命令を止めた隊には「担当へ行軍を指示」で出し直せます。
「行軍が重なる地点を見る」は、複数の味方が同じ次のマスへ進もうとすると表示されます。先に進める隊を選び、ほかの指示を止めるか行き先を変えましょう。
軍議で託した固守・伏兵が戦場で初めて成立すると、演出の後に武将が短く報告します。
二つの烽火：自由な隊を選び、北・南の援護を比較。選ぶと行軍→固守を指示し、再開で進みます。
援護に着いた隊：近くの捕捉敵へ、その武将の一手を比較。士気・損耗と烽火の隣接敵数の予測を確認できます。
守備役も攻める二連携は、今の固守が使えなくなります。別発動の追撃は1コンボ100％です。
烽火の札：現在の固守待ち・指示と前回の実判定を確認。前回×でも、行動回復後の守備を立て直せます。
戦後：結果の一行と「本幕を読む / H」で、連鎖と守備を担った武将、残した代償を振り返れます。
護送・河港の戦後：軍議で任せた固守・伏兵の結果が並び、「本幕」で担当武将が一言振り返ります。
再挑戦：行動を使い固守できなかった記録があると、本人を援護へ回す案を軍議と地図で比較できます。採用した時だけ担当を変えます。
再挑戦の援護：役を替えた隊で単独の援護カードを使い、次の全軍更新で烽火を守れたか見届けましょう。
武将を左クリック、右クリックで移動。「詳しい采配」で戦法・連鎖を組めます。
上部に今の狙いを表示。「目標へ / U」で車の前路や守る場所を見る。
「護送を任せる / 武将たちに任せる」で自動判断。いつでも自分の指揮へ戻せます。
合戦の速さ：右の速度ボタンで0.5・1・2・3倍。戦法の演出中も右下で変更。
記録：F6。カメラ：ホイールで拡大、中ドラッグで回転、Homeで全景。

序章の三戦を終えたら「第一章　黄巾の乱へ」。広宗郊外の戦いに出陣します。
黄巾の乱の結末を読むと「第二章　反董卓連合へ」。酸棗の軍議で1・2を選ぶと、曹操の前哨攻めか張邈の街道守備へ出陣できます。
護送の街道・南の林道、河港の南渡し・北橋、酸棗の二つの作戦はそれぞれ別の戦場です。出陣前に軍議の地図で道と目標を確認できます。
長期の歴史戦役、仕官先の選択、完成版の美術は制作途中です。
フォントのライセンスは Windows/GunyuTactics/Content/UI/Fonts/OFL.txt に収録。
'@
[IO.File]::WriteAllText((Join-Path $OutputRoot 'はじめに.txt'), $guide, [Text.UTF8Encoding]::new($true))
Write-Output "PACKAGED_GAME=$executable"
