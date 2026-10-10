#pragma once
#include <string>
namespace cityidentity
{
	enum class Kind
	{
		Ordinary,
		Granary,
		Market,
		Military,
		Fortress,
		Supply
	};
	inline Kind Of(const std::u32string& name)
	{
		if (name == U"成都" || name == U"鄴" || name == U"平原" || name == U"汝南" || name == U"長沙" ||
		    name == U"武陵")
			return Kind::Granary;
		if (name == U"許昌" || name == U"洛陽" || name == U"建業" || name == U"長安" || name == U"北海" ||
		    name == U"会稽")
			return Kind::Market;
		if (name == U"武威" || name == U"天水" || name == U"安定" || name == U"北平" || name == U"南中" ||
		    name == U"雲南")
			return Kind::Military;
		if (name == U"漢中" || name == U"梓潼" || name == U"永安" || name == U"晋陽" || name == U"新野" ||
		    name == U"廬江")
			return Kind::Fortress;
		if (name == U"江州" || name == U"襄陽" || name == U"下邳" || name == U"柴桑" || name == U"江陵" ||
		    name == U"交趾")
			return Kind::Supply;
		return Kind::Ordinary;
	}
	inline const char32_t* Name(Kind k)
	{
		switch (k)
		{
		case Kind::Granary:
			return U"穀倉";
		case Kind::Market:
			return U"商都";
		case Kind::Military:
			return U"軍都";
		case Kind::Fortress:
			return U"要塞";
		case Kind::Supply:
			return U"兵站拠点";
		default:
			return U"一般都市";
		}
	}
	inline const char32_t* Description(Kind k)
	{
		switch (k)
		{
		case Kind::Granary:
			return U"都市の月間兵糧収入 +40%。後方の収穫を前線へ送る。";
		case Kind::Market:
			return U"都市の月間金収入 +40%。内政・軍備の財源になる。";
		case Kind::Military:
			return U"募兵1回の兵力 +500。同じ金・糧を使い、治安は消耗する。";
		case Kind::Fortress:
			return U"攻城・戦法による守備兵の被害 -15%。要衝を守る拠点。";
		case Kind::Supply:
			return U"実効兵站 +20。携行糧と補給効率が上がるが、実際の蓄えが必要。";
		default:
			return U"基本の収入・募兵・防衛能力。";
		}
	}
} // namespace cityidentity
