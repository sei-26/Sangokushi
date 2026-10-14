"""Build the four-page Japanese design specification with embedded fonts."""
from pathlib import Path
from xml.sax.saxutils import escape
from reportlab.pdfgen import canvas
from reportlab.lib.colors import HexColor
from reportlab.lib.pagesizes import A4
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import Paragraph
from reportlab.lib.styles import ParagraphStyle

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "output" / "pdf" / "gunyu-strategist-spec.pdf"
OUT.parent.mkdir(parents=True, exist_ok=True)
pdfmetrics.registerFont(TTFont("JP", "C:/Windows/Fonts/YuGothM.ttc", subfontIndex=0))
pdfmetrics.registerFont(TTFont("JPB", "C:/Windows/Fonts/YuGothB.ttc", subfontIndex=0))
W, H = A4
INK, GOLD, PAPER = "#172837", "#B38943", "#F6F3EA"
C = canvas.Canvas(str(OUT), pagesize=A4)
C.setTitle("群雄戦線 / 軍師戦役 4枚仕様書 v1.0")
C.setAuthor("群雄戦線 開発仕様")
STYLE = ParagraphStyle("body", fontName="JP", fontSize=10.3, leading=16.5,
                       textColor=HexColor(INK), wordWrap="CJK")
SMALL = ParagraphStyle("small", parent=STYLE, fontSize=9.1, leading=14.2)
BOLD = ParagraphStyle("bold", parent=STYLE, fontName="JPB", fontSize=12.5, leading=20)


def para(text, x, top, width, style=STYLE):
    p = Paragraph(escape(text).replace("\n", "<br/>"), style)
    _, height = p.wrap(width, 900)
    p.drawOn(C, x, H - top - height)
    return top + height


def begin(n, title, subtitle):
    C.setFillColor(HexColor(PAPER)); C.rect(0, 0, W, H, fill=1, stroke=0)
    C.setFillColor(HexColor(INK)); C.rect(0, H - 111, W, 111, fill=1, stroke=0)
    C.setFillColor(HexColor(GOLD)); C.rect(30, H - 47, 27, 3, fill=1, stroke=0)
    C.setFont("JPB", 10); C.drawString(67, H - 47, "群雄戦線  /  軍師戦役")
    C.setFillColor(HexColor("#FFFFFF")); C.setFont("JPB", 21)
    C.drawString(30, H - 80, title)
    C.setFont("JP", 9); C.drawString(30, H - 100, subtitle)
    C.setFillColor(HexColor(INK)); C.setFont("JP", 8)
    C.drawString(30, 23, "仕様 v1.0   2026.10.14   /   設計と初期実装対象を定義")
    C.drawRightString(W - 30, 23, f"{n} / 4")


def section(label, body, top):
    C.setFillColor(HexColor(GOLD)); C.rect(30, H - top - 17, 3, 17, fill=1, stroke=0)
    top = para(label, 43, top, W - 73, BOLD) + 7
    return para(body, 43, top, W - 73) + 19


def table(headers, rows, top, widths):
    x = 30
    for j, h in enumerate(headers):
        C.setFillColor(HexColor(INK)); C.rect(x, H - top - 29, widths[j], 29, fill=1, stroke=0)
        style = ParagraphStyle("th", parent=SMALL, fontName="JPB", textColor=HexColor("#FFFFFF"))
        para(h, x + 9, top + 7, widths[j] - 18, style); x += widths[j]
    top += 29
    for i, row in enumerate(rows):
        heights = []
        for j, text in enumerate(row):
            p = Paragraph(escape(text), SMALL); _, hh = p.wrap(widths[j] - 18, 500); heights.append(hh)
        height = max(heights) + 17
        C.setFillColor(HexColor("#FFFFFF" if i % 2 == 0 else "#EAE7DE"))
        C.rect(30, H - top - height, sum(widths), height, fill=1, stroke=0)
        x = 30
        for j, text in enumerate(row):
            para(text, x + 9, top + 8, widths[j] - 18, SMALL); x += widths[j]
        top += height
    return top + 18


def finish(top):
    if top > H - 52:
        raise ValueError(f"Page content reaches footer: {top:.1f}")
    C.showPage()


begin(1, "軍師として、歴史を動かす", "01  企画の目的 / コア体験 / 独自性")
y = section("企画の一文", "三国志の大きな戦いに従軍軍師として参加し、英傑たちと作戦を実行する戦略シミュレーション。戦い方と救った人物が、その後の歴史の可能性を変える。", 135)
C.setFillColor(HexColor("#E7DFCD")); C.roundRect(30, H - y - 91, W - 60, 91, 7, fill=1, stroke=0)
para("コア", 44, y + 10, W - 88, SMALL)
para("敵の判断を読み、戦場の状況を仕込み、武将たちを動かして、自分が狙った勝ち方を成立させる。", 44, y + 30, W - 88, BOLD)
y += 112
y = section("誰に、どんな面白さを届けるか", "三国志・歴史のIFを好み、自分で戦略と戦術を組み立てたい人が対象。作戦が連動する達成感、劣勢を判断で変える手応え、役割を任せた武将への愛着を狙う。遊んだ後に人物や実際の歴史を調べたくなる体験を目指す。", y)
y = table(["設計の柱", "生まれる判断"], [
    ("情報と欺瞞", "偵察へ兵を割くか。主力を隠すか。敵に何を見せて誘うか。"),
    ("配置と拘束", "誰に敵を引きつけさせるか。正面を薄くして予備隊を残すか。"),
    ("指揮と連動", "合図を待つか。救援に回すか。どこで作戦を中止するか。")], y, [118, W - 178])
y = section("仕様を増やすときの判断基準", "新機能ごとに、増える判断・支払う代償・敵の対抗手段・結果の見せ方を定める。独自性は、条件付き作戦と不完全な情報、武将の任務適性が互いに影響する構造から作る。機能数や演出量を面白さの証明にしない。", y)
finish(y)

begin(2, "軍議から、次の戦役へ", "02  主人公の立場 / ゲーム進行 / 歴史と武将")
y = section("主人公は従軍軍師", "架空の軍師が英傑と出会い、任された方面と兵力で作戦を遂行する。戦前に任務・借りられる部隊・他方面との協力条件を合意する。味方が何を引き受けたかを表示し、理由のない命令無視に頼らない。", 135)
y = table(["段階", "操作・判断", "次へ返るもの"], [
    ("軍議", "目的、敵情、地形を確認して方針を選ぶ", "作戦の目的と使える兵力"),
    ("準備", "担当、経路、開始・中止条件、代替行動を設定", "役割分担と残す予備隊"),
    ("開戦", "戦況を観察。確認時刻と報告を読んで進行", "新情報と作戦の成否"),
    ("再軍議", "次の命令期に救援・迂回・撤退を再計画", "失う機会と守る部隊"),
    ("戦後", "目的達成、兵・糧の損耗、判断の経過を確認", "次の戦役、功績と関係")], y, [65, 276, W - 401])
y = section("武将を知ると、選べる作戦が広がる", "持久、偵察、突破、補給、撤退などに適性を持たせる。武将を入れ替えたら任務か作戦が変わることが基準。信頼と共闘の成果は、将来の共同任務や任せてもらえる兵力へ返す。数値を積むだけの好感度にしない。", y)
y = section("歴史の条件を、プレイヤーの悩みにする", "兵力差、地形、補給、味方の目的を戦いの条件として提示する。史実・演義・創作は明示する。勝利だけでなく、救援・撤退・損耗の結果からIFを分岐させる。歴史説明と会話は、その戦いの判断を理解する材料にする。", y)
y = section("初期版の範囲", "まず架空の護送戦で軍議→準備→戦闘→戦果を実装。指揮権の成長、人物ごとの交渉、歴史に基づく戦役の連鎖は後続。全国地図・自由戦略・旧英雄譚の素材とルールは再利用できる土台として保持する。", y)
finish(y)

begin(3, "作戦を組み、報告から判断する", "03  戦闘ルール / 情報 / 命令 / 操作")
y = table(["対象", "ルール"], [
    ("地形と補給", "地形・選択・移動・射線・支配・補給は同じHEX。既存の兵糧消費、輸送、包囲と同時損害計算を利用する。"),
    ("観測情報", "地形は既知。敵は現在確認・過去の目撃・未発見を区別する。位置と情報の確認日を記録する。"),
    ("公平な敵判断", "敵も観測情報を用いる。プレイヤーの作戦や隠れた部隊の実位置を読まない。戦闘と移動の物理解決は実状態で行う。"),
    ("進行", "既存の1日単位・10日の同時進行を維持。命令期で編集し、進行中は一時停止して閲覧できる。"),
    ("作戦の単位", "部隊ごとに主任務、開始条件、中止条件、代替地点を設定。通常命令と定型作戦から始められる。")], 135, [103, W - 163])
y = table(["初期の命令要素", "選択肢"], [
    ("任務・場所", "指定地点へ移動／守備／後退。実際の部隊による偵察と救援。"),
    ("開始条件", "即時／指定日／味方到着の合図／指定範囲で敵を確認。"),
    ("中止と代替", "兵数・士気の下限、待機期限。成立しなければ待機か指定地点へ後退。")], y, [126, W - 186])
y = section("解決と表示の約束", "開始・中止条件は同じ日の情報から評価し、中止を優先する。当日発生した到着合図は翌日の評価から使う。待機、実行、中止、到達不能の理由を表示する。部隊の処理順で作戦結果が変わらないようにする。", y)
y = section("読み合いを崩さない", "誘引は目撃した配置と移動への反応で成立させる。敵には攻勢・警戒・補給襲撃の方針を用意する。隠れた敵の情報を経路表示、助言、戦況ログから漏らさない。初期版の未実装項目は明示する。", y)
finish(y)

begin(4, "一戦で、面白さを確かめる", "04  最初の試作 / 実装順 / 合格条件")
y = section("初期戦役：渡河・兵糧護送", "歴史の再現ではない架空の検証戦場。川、二つの渡河点、林の迂回路、後方拠点と前線拠点を配置する。主人公は護送方面を任される。敵の全滅ではなく、積荷の到着と味方の温存を目的にする。", 135)
y = table(["初期調整値", "内容"], [
    ("戦場と兵力", "24×18 HEX。味方戦闘4隊×3,000、輸送護衛1,000。敵4隊×3,500。敵は10日間の集結後に進軍。"),
    ("成功条件", "45日以内に兵糧3,500以上を前線へ配送し、味方戦闘兵7,500以上を温存。初期積荷は5,000。"),
    ("試せる方針", "渡河点を押さえる／林へ誘う／予備隊で護送路を守る。敵の方針と配置で有効性を比較する。")], y, [107, W - 167])
y = table(["実装段階", "成果物と確認"], [
    ("第1段階", "軍師戦役の入口、軍議、同じHEXの検証戦場、個別命令、護送と戦果。既存戦闘との基準比較。"),
    ("第2段階", "観測と敵判断、条件・合図・中止・代替行動。待機理由と作戦の経過を表示。"),
    ("第3段階", "武将の任務適性、共同任務と信頼、根拠を確認した歴史戦役、戦果による分岐。")], y, [107, W - 167])
y = section("合格条件", "①敵の方針が変わると作戦を変える理由がある。②少数側が目的を達成でき、敵にも対抗策がある。③武将の交換が担当や作戦を変える。④敗因と次に変える判断を説明できる。⑤操作速度ではなく判断の質が結果に返る。", y)
y = section("検証・保留するもの", "自動試験で観測の公平性、同時評価、条件、輸送の実収支と既存セーブを確認。プレイ記録で理解と再挑戦の動機を確認する。伝令遅延、自由な条件式、大規模な全国統合、戦役間の保存と歴史分岐は、最初の一戦を検証してから拡張する。", y)
finish(y)
C.save()
print(OUT)
