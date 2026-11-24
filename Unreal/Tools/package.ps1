param(
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.7',
    [ValidateSet('Development','Shipping')][string]$Configuration = 'Development',
    [string]$OutputRoot,
    [switch]$SkipRulesBuild,
    [ValidateRange(1,64)][int]$MaxParallelActions = 4
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$repoRoot = Split-Path $projectRoot -Parent
$projectFile = Join-Path $projectRoot 'GunyuTactics.uproject'
$uat = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
if (!(Test-Path -LiteralPath $uat)) { throw "Unreal automation tool missing: $uat" }
if (!$OutputRoot) { $OutputRoot = Join-Path $repoRoot "Releases\GunyuSenzen-$Configuration" }
$OutputRoot = [IO.Path]::GetFullPath($OutputRoot)
if (!$SkipRulesBuild) { & (Join-Path $PSScriptRoot 'build.ps1') -EngineRoot $EngineRoot -SkipEditor -MaxParallelActions $MaxParallelActions }
if (!(Test-Path -LiteralPath (Join-Path $projectRoot 'Intermediate\Rules\sangokushi_rules.lib'))) {
    throw 'Build the shared rules before packaging.'
}
$uatArguments = @(
    'BuildCookRun', '-installed', '-nop4', "-project=$projectFile",
    '-unattended', '-utf8output', '-nocompileeditor',
    '-platform=Win64', "-clientconfig=$Configuration", '-build', '-cook',
    '-stage', '-pak', '-prereqs', '-archive', '-nodebuginfo',
    "-archivedirectory=$OutputRoot", "-UbtArgs=-CompilerVersion=14.44.35207 -MaxParallelActions=$MaxParallelActions"
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
初陣の退路戦：最初の1・2で「関羽の伏兵で先鋒を崩す / 劉備を出口へ、二人で援護」を直接採用。準備と三人の担当を一緒に選び、出陣までは兵と糧を使いません。採用後も地図で変更できます。
出口へ送る策は、張飛や関羽の援護が大切。敵を全部倒す必要はありませんが、出口の隣に敵が残る間は確保できません。
序章の軍議：武将ごとにその戦での役目を聞けます。「作戦図 / M」で話した地点へ。配置案は採用するまで担当を変えません。
軍議の会話中：1・2は画面の二つの返答。
出陣：採る策と費用を確認し、「この策で、出陣だ！」。
本編の基本出陣には兵と兵糧が必要です。軍資金は林道の案内人や募兵などに使います。出陣できない場合や敗戦後も、結果を受け入れて物語を進められます。
合戦：見えている敵を選び、右の「次の一手」で効果・代償を比べて発動。
出陣後は武将カードで軍議の担当を確認できます。同時進行で自由に動かせる隊が一人なら、その隊を選択して始めます。
序章の開戦直後は「軍議の一手」から、話した地点を見たり、最初の移動先を選んだりできます。指示済みの隊は、地点を見てからSpaceで進めます。
担当地点へ / P：選んだ隊の軍議の行き先を見る。目標へ / U：現在の任務の目標を見る。
担当の札では、残りの行軍・進路待ち・到着を確認できます。原因の隊を確認できた時は「道をふさぐ隊を見る」。命令を止めた隊には「担当へ行軍を指示」で出し直せます。
「行軍が重なる地点を見る」は、複数の味方が同じ次のマスへ進もうとすると表示されます。先に進める隊を選び、ほかの指示を止めるか行き先を変えましょう。
軍議で託した固守・伏兵が戦場で初めて成立すると、演出の後に武将が短く報告します。
二つの烽火：自由な隊を選び、北・南の援護を比較。選ぶと行軍→固守を指示し、再開で進みます。
援護に着いた隊：近くの捕捉敵へ、その武将の一手を比較。士気・損耗と烽火の隣接敵数の予測を確認できます。
守備役も攻める二連携は、今の固守が使えなくなります。別発動の追撃は1コンボ100％です。
烽火の札：現在の固守待ち・指示と前回の実判定を確認。前回×でも、行動回復後の守備を立て直せます。
護送車の前進は車輪が通った道に光が出ます。烽火は守備が成立した側が金色に、途切れた側が朱色に光ります。
戦後：結果の一行と「本幕を読む / H」で、連鎖と守備を担った武将、残した代償を振り返れます。
初陣の戦後は、実際の先鋒崩し・出口確保・護送・敗戦に人物が応えます。「三人と初陣を振り返る / H」は任意。読まなくても黄巾の本戦へ進めます。
護送・河港の戦後：軍議で任せた固守・伏兵の結果が並び、「本幕」で担当武将が一言振り返ります。
再挑戦：行動を使い固守できなかった記録があると、本人を援護へ回す案を軍議と地図で比較できます。採用した時だけ担当を変えます。
再挑戦の援護：役を替えた隊で単独の援護カードを使い、次の全軍更新で烽火を守れたか見届けましょう。
武将を左クリック、右クリックで移動。手動では右上に戦法・対象・予約・発動を常設。「戦法▼ / F2」で通常・固有戦法を選べます。
予約後は「単独／連鎖を今発動 / C」。一人でも使え、二手目125％・三手目150％へ上昇。停止中は演出後も停止へ戻ります。
行動済みなどの理由も同じ欄に表示。「1周期進める / .」は行軍・交戦・糧の消費を含む通常の進行です。固守中なら必要に応じて「固守を解除」。
合図待ちの連鎖は合図を解除してから直接発動。「連鎖の順番・詳しい采配」で順序と条件を確認できます。
上部に今の狙いを表示。「目標へ / U」で車の前路や守る場所を見る。
「護送を任せる / 武将たちに任せる」で自動判断。いつでも自分の指揮へ戻せます。
危険時停止がONなら、護送路の遮断や河港の占領機などで自動指揮が止まります。「変化の地点へ / U」で見る、「ここから自分で指揮」で引き取る、Spaceで任せたまま再開。
合戦の速さ：右の速度ボタンで0.5・1・2・3・4・5倍。戦法の演出中も右下で変更。
記録：F6。カメラ：ホイールで拡大、中ドラッグで回転、Homeで全景。

新規開始は初陣の一戦後に「第一章 黄巾の本戦へ」。道中を短い物語でつなぎ、広宗の大合戦へ進みます。「序章の別働任務も遊ぶ」で従来の三戦も選べます。未出陣の任務を勝利扱いにはしません。
広宗の開戦では「北の仲間を救う / 中央を一気に突破する」を選びます。北へ回れば左翼を救援し、中央を選ぶと本隊が兵糧と士気を使って強行軍。青い矢印は本隊の進軍予定です。
前陣を開いて本隊と合流し、黄巾軍本陣を撃破・敗走させれば決着。北陣を退けて実際に左翼を救い、生還させた武将は、次の酸棗で先行配置と士気支援を得ます。会話にもその働きが残ります。
黄巾の乱の結末を読むと「第二章　反董卓連合へ」。軍議で「曹操と対岸を押す / 張邈の援軍を救い、連合軍を連れ帰す」を選び、二つの橋を持つ汴水の11部隊の会戦へ進みます。
攻勢は曹操の前陣へ旗を入れ、徐栄軍の本陣を撃破・捕捉中の士気25以下へ崩して決着。救出は南の包囲隊を崩して生きた援軍の隣へ実際に届き、曹操と援軍が南橋の西へ戻るまで守ります。主力は橋の出口を空けて、西の目標を確保してください。救出では本陣掃討は不要です。
汴水で実際に救援し生還した兵は、次の陽人で孫堅の先行配置を支えます。史実では曹操が徐栄に敗れた戦いを背景に、義勇軍の采配でIFの結果を作ります。護送の街道・南の林道と、河港の南渡し・北橋も別の戦場です。
河港は北橋から側面へ、南橋から指揮所に近い正面へ進めます。橋名と土の道を目印に、渡る隊と旗を立てる隊を決めましょう。
河港では最後の守備隊を退け、安全に指揮所を占領すれば勝利。同じ隊でも勝てます。別の隊でその指揮と後の２指揮以内に旗を立てると、次の烽火の守備は３指揮から２指揮になります。
汴水を終えたら「陽人の戦いへ」。11部隊と専用地形で、森で先鋒騎兵を崩すか、孫堅と一緒に押すかを選びます。辻を確保し孫堅と合流すると、呂布の親衛騎兵が反撃。孫堅を守り、反撃隊を撃破するか捕捉中に士気25以下へ崩すと決着します。敵本陣の掃討は不要です。
守り抜いた孫堅本隊の兵は会話と信頼に残り、次の洛陽で任せられる作戦に関わります。旧セーブの進行中の戦闘は従来の地形と勝利条件を保ち、軍議へ戻った後の出陣で新構成へ移ります。
両会戦は最初に全景で表示します。前陣の突破役と味方の損耗が戦後の掛け合いに残ります。進行中の旧記録の戦いは保ち、次の対象戦の出陣から新構成を使います。
突破・救援・共同攻勢・反撃などの実成果を、肖像付きの掛け声・旗からの光・広いカメラで知らせます。実際の行動の再生後にだけ表示。「采配へ」、F2やカメラ操作ですぐ閉じられます。演出Oで切り替え。
「南門を開く」で本隊を通すか、陽人で孫堅の信頼を4以上得た場合は「西の荷駄隊を捕らえる」を選べます。西では車の１マス東を塞ぎ、護衛を崩すと兵糧1,000を得ます。洛陽の任務を達成すると第二章完結です。
長期の歴史戦役、仕官先の選択、完成版の美術は制作途中です。
フォントのライセンスは Windows/GunyuTactics/Content/UI/Fonts/OFL.txt に収録。
'@
[IO.File]::WriteAllText((Join-Path $OutputRoot 'はじめに.txt'), $guide, [Text.UTF8Encoding]::new($true))
Write-Output "PACKAGED_GAME=$executable"
