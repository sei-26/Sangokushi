#include "../NewGame/Campaign.hpp"
#include <cassert>
#include <iostream>
using namespace frontline;
int main()
{
	Campaign game;
	game.Reset(0);
	assert(game.commands == 7 && game.commandBudget == 7);
	for (int count = 0; count <= 30; ++count)
	{
		for (int i = 0; i < 30; ++i)
			game.cities[i].owner = i < count ? 0 : 1;
		assert(game.CommandLimit(0) == 3 + std::max(0, count - 1) / 2);
		assert(game.CommandLimit(1) == 3 + std::max(0, 29 - count) / 2);
		assert(game.CommandLimit(2) == 3);
	}
	game.Reset(0);
	game.cities[0].owner = game.cities[1].owner = 1;
	assert(game.CommandLimit(0) == 6 && game.commandBudget == 7);
	game.commands = 1;
	game.day = 9;
	game.AdvanceDay();
	assert(game.day == 10 && game.commands == 6 && game.commandBudget == 6);
	game.commands = 1;
	game.BeginTurn();
	assert(game.commands == 1); // Replanning cannot refill the player's budget.
	const auto remaining = game.aiCommands;
	game.BeginTurn();
	assert(game.aiCommands == remaining);
	game.ResetLegacy(0);
	assert(game.commands == 4 && game.commandBudget == 4);
	std::cout << "City-scaled budgets, period refresh and planning idempotence passed.\n";
}
