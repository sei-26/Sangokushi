#include "Campaign.hpp"
namespace frontline
{
	bool Campaign::ReturnToCity(int index, int city)
	{
		if (result != 0 || index < 0 || index >= static_cast<int>(armies.size()) || city < 0 ||
		    city >= static_cast<int>(cities.size()))
			return false;
		auto& a = armies[index];
		if (a.troops <= 0 || cities[city].owner != a.faction)
			return false;
		if (a.tile == cities[city].tile)
		{
			Return(a, city);
			return true;
		}
		if (!Order(index, cities[city].tile, true))
			return false;
		Note(generals[a.general].name + U"隊が" + cities[city].name + U"への帰還を開始。");
		return true;
	}
} // namespace frontline
