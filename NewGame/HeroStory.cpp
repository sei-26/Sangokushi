#include "HeroStory.hpp"

namespace hero
{
	void Story::Reset()
	{
		*this = Story{};
		Record(U"劉備の英雄譚を開始。どんな旗を掲げるかは、あなたが選ぶ。");
	}

	bool Story::Tactical() const
	{
		return chapter != 1 && chapter != 2;
	}

	int Story::TurnLimit() const
	{
		return chapter == 3 ? (VowRequired() ? 52 : 40) : 24;
	}

	bool Story::Valid(int x, int y)
	{
		return x >= 0 && x < W && y >= 0 && y < H;
	}

	int Story::Distance(const Unit& a, const Unit& b)
	{
		return std::abs(a.x - b.x) + std::abs(a.y - b.y);
	}

	void Story::Record(const std::u32string& text)
	{
		journal.push_back(text);
		if (journal.size() > 100)
			journal.erase(journal.begin());
	}

	int Story::At(int x, int y) const
	{
		for (int i = 0; i < static_cast<int>(units.size()); ++i)
			if (units[i].hp > 0 && units[i].x == x && units[i].y == y)
				return i;
		return -1;
	}

	int Story::Companion() const
	{
		return chapter == 2 || chapter == 4 ? 4 : chapter == 3 ? 3 : chapter == 5 ? 5 : 1;
	}

	int Story::Unlocked() const
	{
		return chapter < 3 ? 3 : chapter < 5 ? 5 : 6;
	}

	void Story::Clamp()
	{
		virtue = std::clamp(virtue, 0, 100);
		resolve = std::clamp(resolve, 0, 100);
		food = std::clamp(food, 0, 999);
		gold = std::clamp(gold, 0, 999);
		for (auto& b : bonds)
			b = std::clamp(b, 0, 100);
	}

	std::u32string Story::Ending() const
	{
		return virtue >= 80 ? U"人を守る旗" : resolve >= 80 ? U"乱世を渡る旗" : U"仲間と歩む旗";
	}
} // namespace hero
