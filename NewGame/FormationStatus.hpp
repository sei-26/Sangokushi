#pragma once
#include "Campaign.hpp"
namespace formationstatus
{
	enum class State
	{
		Active,
		OutOfRange,
		Retreat,
		LowMorale,
		Starving,
		Transport,
		LowAffinity,
		CivilRole,
		ReceiverUnavailable
	};
	struct Member
	{
		int army = -1, affinity = 0, distance = 0;
		officer::Link actual, potential;
		State state = State::Active;
	};
	inline bool Any(const officer::Link& x)
	{
		return x.attack || x.defense || x.morale || x.supply;
	}
	inline const char32_t* Reason(State s)
	{
		switch (s)
		{
		case State::Active:
			return U"連携中";
		case State::OutOfRange:
			return U"2マス以内へ接近すると連携";
		case State::Retreat:
			return U"帰還中の部隊は連携不可";
		case State::LowMorale:
			return U"士気30以上で連携";
		case State::Starving:
			return U"補給か携行糧の回復が必要";
		case State::Transport:
			return U"輸送隊は連携の支援を出せない";
		case State::LowAffinity:
			return U"親密40以上で連携";
		case State::CivilRole:
			return U"内政・交渉役は戦場の連携補正なし";
		default:
			return U"選択部隊の帰還・士気・兵糧を確認";
		}
	}
	inline std::vector<Member> Members(const frontline::Campaign& game, int index)
	{
		std::vector<Member> out;
		if (index < 0 || index >= static_cast<int>(game.armies.size()) || game.armies[index].troops <= 0)
			return out;
		const auto& a = game.armies[index];
		for (int i = 0; i < static_cast<int>(game.armies.size()); ++i)
		{
			const auto& b = game.armies[i];
			if (i == index || b.troops <= 0 || b.faction != a.faction)
				continue;
			Member m;
			m.army = i;
			m.distance = game.MapDistance(a.tile, b.tile);
			m.affinity = game.Affinity(a.general, b.general);
			m.actual = game.FormationLink(index, i);
			m.potential = officer::Contribution(officer::RoleOf(game.generals[a.general].name),
			                                    officer::RoleOf(game.generals[b.general].name), m.affinity);
			if (a.retreat || a.morale < 30 || (!a.supplied && a.food <= 0))
				m.state = State::ReceiverUnavailable;
			else if (b.arm == frontline::Arm::Transport)
				m.state = State::Transport;
			else if (b.retreat)
				m.state = State::Retreat;
			else if (b.morale < 30)
				m.state = State::LowMorale;
			else if (!b.supplied && b.food <= 0)
				m.state = State::Starving;
			else if (m.affinity < 40)
				m.state = State::LowAffinity;
			else if (!Any(m.potential))
				m.state = State::CivilRole;
			else if (m.distance > 2)
				m.state = State::OutOfRange;
			out.push_back(m);
		}
		std::stable_sort(out.begin(), out.end(), [](const Member& a, const Member& b) {
			if (Any(a.actual) != Any(b.actual))
				return Any(a.actual) > Any(b.actual);
			if (a.distance != b.distance)
				return a.distance < b.distance;
			return a.affinity > b.affinity;
		});
		return out;
	}
} // namespace formationstatus
