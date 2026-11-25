// Editable four-slide core pitch. Uses project-owned art and actual runtime captures.
// Arguments: repository root, bundled dependency root, presentations skill directory.
import fs from 'node:fs/promises';
import {readFileSync} from 'node:fs';
import path from 'node:path';
import {createRequire} from 'node:module';
import {pathToFileURL} from 'node:url';

const [repo, runtime, skill] = process.argv.slice(2);
if (!repo || !runtime || !skill) throw new Error('Pass repository, runtime dependency root and presentations skill directory.');
process.env.RUNTIME_NODE_MODULES=path.join(runtime,'node/node_modules');
const work = path.join(repo, 'Intermediate/core-pitch-build');
await fs.mkdir(work, {recursive:true});
const require = createRequire(path.join(work, 'loader.cjs'));
const {Presentation, PresentationFile} = await import(pathToFileURL(require.resolve('@oai/artifact-tool')).href);
const p = Presentation.create({slideSize:{width:1280,height:720}});
const font='Noto Sans JP';
const ink='#17272B', cream='#F4EFE5', gold='#B38545', pale='#E3C792';
function txt(s,name,text,x,y,w,h,size=20,color=ink,bold=false) {
  const t=s.shapes.add({geometry:'textbox',name,position:{left:x,top:y,width:w,height:h},fill:'none',line:{fill:'none',width:0}});
  t.text=text;
  t.text.style={typeface:font,fontSize:`${size}pt`,bold,color,wrap:'square',autoFit:'none',insets:0,verticalAlignment:'top',lineSpacing:1.18};
  return t;
}
function pic(s,relative,x,y,w,h,fit='cover') {
  return s.images.add({blob:new Uint8Array(readFileSync(path.join(repo,relative))),contentType:'image/png',alt:relative,fit,position:{left:x,top:y,width:w,height:h}});
}
function footer(s,n,dark=false) {
  txt(s,`footer-${n}`,`群雄戦線 ｜ コア企画書　2026.11.26`,48,675,1080,30,12,dark?'#C5BDB0':'#596467');
  txt(s,`page-${n}`,`${n} / 4`,1160,675,76,30,12,dark?'#C5BDB0':'#596467');
}

// 1. The promise: one hero image, one clear player fantasy.
{
 const s=p.slides.add(); s.background.fill='#000000';
 pic(s,'Unreal/ArtSource/Novel/strategist-standing-v1.png',800,0,480,720);
 txt(s,'cover-category','物語で歴史を体験する、戦略シミュレーション',64,66,730,48,18,pale);
 txt(s,'cover-title','群雄戦線',60,151,720,98,54,'#F4EFE5',true);
 txt(s,'cover-subtitle','名もなき軍師',64,253,700,54,24,pale);
 txt(s,'cover-promise','英傑と策を交わし、\n自分の一手で歴史を動かす。',64,343,760,144,31,'#F4EFE5',true);
 txt(s,'cover-deliver','届けたいのは「自分が、この戦いを変えた」実感。\n人物の志に触れ、三国志をもっと知りたくなる体験。',64,530,735,103,18,'#DED8CD');
 txt(s,'cover-audience','三国志が好きな人へ ／ PC・一人用 ／ 開発中',64,660,765,32,15,pale);
 s.speakerNotes.text='作品の目的は、歴史の面白さをプレイヤー自身の判断と人間関係から伝えること。主人公は既存の英傑を置き換える支配者ではなく、従軍する若い軍師。本資料は完成像と2026-11-26時点の実装を区別する。素材: Unreal/ArtSource/Novel/strategist-standing-v1.png（既存プロジェクト素材）。方針根拠: CORE_DESIGN.md。';
}
// 2. One concrete dilemma makes the core assessable.
{
 const s=p.slides.add();s.background.fill=cream;
 txt(s,'core-eyebrow','遊びのコア',48,35,1160,36,18,gold,true);
 txt(s,'core-title','誰を救い、どこへ主力を出すか。',48,82,1170,69,32,ink,true);
 pic(s,'Intermediate/command-read-core-opening/Saved/trial.png',48,176,810,456,'contain');
 txt(s,'relief-label','北へ、救援を送る',898,182,336,48,22,ink,true);
 txt(s,'relief-body','孤立した左翼を救う。\n中央の攻め手は減る。',898,242,336,105,20);
 txt(s,'central-label','中央を、突破する',898,378,336,48,22,ink,true);
 txt(s,'central-body','本隊の進路を開く。\n左翼には耐えてもらう。',898,438,336,105,20);
 txt(s,'core-rule','配置を考え、単独で打つか、仲間の戦法をつなぐかを決める。',48,632,1184,38,18,ink,true);
 footer(s,2);
 s.speakerNotes.text='画像は現行Unreal実行画面・広宗の開戦時。画面右の二案と同じ選択を説明しており、完成予想CGではない。救援は北陣の敵への対処に加えて主力一隊の到達が必要。中央突破は前陣を維持して皇甫嵩本隊を通す。左翼救援と中央攻勢の時間・配置配分が判断。単独戦法は一人で発動できる。連鎖は最大3手、段階補正100/125/150%。自動指揮から手動へ引き取ることも可能。実装根拠: NewGame/JourneyGrandApproach.cpp、JourneyGrandBattle.cpp、TacticChain.cpp、Unreal/Source/GunyuTactics/TacticalManualTactics.cpp。';
}
// 3. Battle consequences are the reason to inhabit a protagonist.
{
 const s=p.slides.add();s.background.fill='#10191B';
 pic(s,'Unreal/ArtSource/Novel/liu-conversation-v1.png',0,0,420,720);
 txt(s,'payoff-eyebrow','このゲームで届けたい体験',466,39,760,39,18,pale,true);
 txt(s,'payoff-title','勝ち方が、人と次の戦いを変える。',466,91,760,110,30,'#F4EFE5',true);
 txt(s,'payoff-intro','英傑の物語に、自分の判断が刻まれる。',466,211,760,45,20,'#E9DDC9');
 txt(s,'payoff-choice-label','01　自分で選ぶ',466,291,735,39,20,pale,true);
 txt(s,'payoff-choice','広宗で、中央への攻勢を遅らせて左翼を救う。',466,339,735,55,19,'#F4EFE5');
 txt(s,'payoff-response-label','02　人物が、その勝ち方に応える',466,411,735,40,20,pale,true);
 txt(s,'payoff-response','救援と生還の成否を、戦後の人物との会話に返す。',466,459,735,56,19,'#F4EFE5');
 txt(s,'payoff-next-label','03　次の戦場に、選択が残る',466,531,735,40,20,pale,true);
 txt(s,'payoff-next','救った左翼の案内で、救援した武将が先行配置。\n次戦では士気＋10の支援も得られる。',466,579,735,80,19,'#F4EFE5');
 txt(s,'payoff-status','実装例：広宗 → 反董卓連合',466,677,760,29,12,'#C5BDB0');
 s.speakerNotes.text='このスライドの因果は実装済み。ただし救援に成功しても左翼が生還しなければ次戦の支援は得られない。条件: history[YellowTurbanMission].won、review.grand.relievedOn>0、leftReturned>0、reliefOfficer>=0。会話: NewGame/JourneyGrandStrategyDrama.cpp。次戦配置と士気: NewGame/JourneyPreparation.cpp。現行の汴水大会戦では救援武将の開始位置は(6,4)、支援士気+10は上限100。UI: TacticalCampaignCarryover.cpp。今回は戦後概要も実結果と一致するよう更新。肖像は既存会話素材であり、この文面を実際の劉備の台詞として引用してはいない。';
}
// 4. Keep current proof separate from the product vision.
{
 const s=p.slides.add();s.background.fill=cream;
 txt(s,'scope-eyebrow','実装と完成像',48,35,1160,36,18,gold,true);
 txt(s,'scope-title','一戦の判断を、歴史を旅する体験へ。',48,82,1184,70,32,ink,true);
 txt(s,'now-label','現在、遊べるもの',48,192,550,48,24,ink,true);
 txt(s,'now-1','序章 → 黄巾 → 反董卓連合\n陽人・洛陽へ続く戦役',48,263,550,83,20);
 txt(s,'now-2','単独戦法／最大3手の連鎖\n手動指揮・自動戦闘・最大5倍速',48,377,550,83,20);
 txt(s,'now-3','勝ち方に応じた会話・信頼の変化\n救援結果を次戦の配置と士気へ反映',48,491,550,83,20);
 txt(s,'vision-label','これから育てる完成像',666,192,570,48,24,gold,true);
 txt(s,'vision-1','仕える勢力を選び、英傑と歩む長編\n洛陽の先へ広がる歴史と分岐',666,263,570,83,20);
 txt(s,'vision-2','志の対立や共闘を積み重ね、\n自分だけの仲間との関係を描く',666,377,570,83,20);
 txt(s,'vision-3','内政・外交などは将来の検討項目。\nまず、戦場と物語のつながりを磨く',666,491,570,83,20);
 txt(s,'scope-criterion','面白さの基準：策を選べた。戦場が変わった。あの武将と、次の戦いへ行きたい。',48,614,1184,50,17,ink,true);
 footer(s,4);
 s.speakerNotes.text='実装済みと構想を明示。現在の主人公は義勇軍の軍師を軸にした進行で、自由な勢力加入・全国統一・長期外交・異国貿易は完成していない。従来の三国志14風の機能や旧Siv3D試作を、現行UE本編の完成済み機能と数えない。技術: Unreal Engine 5.7、C++/Slate UI、エンジン非依存のNewGame共通ルール、CMake/CTest、Git/Git LFS。保存仕様26の互換性テストあり。狙う対象は三国志と戦術判断に興味のある人。面白さの基準は制作上の仮説であり、初見ユーザーの試遊による検証が必要。';
}

for(let i=0;i<p.slides.items.length;i++){
 const blob=await p.export({slide:p.slides.items[i],format:'png',scale:1});
 await fs.writeFile(path.join(work,`slide-${i+1}.png`),new Uint8Array(await blob.arrayBuffer()));
}
const candidate=path.join(work,'core-pitch-candidate.pptx');
await (await PresentationFile.exportPptx(p)).save(candidate);
const {finalizePresentation}=await import(pathToFileURL(path.join(skill,'container_tools/artifact_tool_utils.mjs')).href);
const final=path.join(repo,'Docs/企画書/群雄戦線_コア企画書_20261126_v2.pptx');
await fs.mkdir(path.dirname(final),{recursive:true});
await finalizePresentation({workspaceDir:repo,candidatePath:candidate,finalPath:final,
 explicitTotalSlideCount:4,requiredNativeTableOwnerSlides:[],requiredNativeChartOwnerSlides:[],
 pythonExecutable:path.join(runtime,'python/python.exe'),
 integrityValidatorPath:path.join(skill,'container_tools/inspect_presentation_package_integrity.py'),
 layoutValidatorPath:path.join(skill,'container_tools/inspect_presentation_layout_geometry.py'),
 layoutArgs:['--expected-slide-size-emu','12192000,6858000','--validate-bullet-geometry','--validate-heading-fit'],
 fontPolicy:{basis:'design',families:[font]},verifyArtifactToolImport:true,
 receiptPath:path.join(work,`${path.basename(final)}.validation.json`)});
console.log(final);
