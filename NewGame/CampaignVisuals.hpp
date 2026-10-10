#pragma once
#include <Siv3D.hpp>
#include "CampaignTypes.hpp"

namespace campaignvisual
{
	Polygon TileShape(Vec2 center, double width, bool hex, double pitch = 1.);
	void TerrainTile(const RectF& rect, frontline::Terrain terrain, int seed, bool hex = false,
	                 double pitch = 1.);
	void TerrainRelief(Vec2 center, double width, frontline::Terrain terrain, int seed);
	void MapLabel(Vec2 center, const String& label, int faction, bool selected = false, bool small = false);
	void MapLight(const RectF& area);
	void Clash(Vec2 center, double cell, int seed);
	Line TileEdge(Vec2 center, double width, double pitch, Vec2 direction);
	void Shore(Vec2 center, double width, double pitch, Vec2 direction, bool sea);
	void Fields(Vec2 center, double width, int seed);
	void CityIcon(Vec2 center, double cell, int faction, bool selected);
	void ArmyIcon(Vec2 center, double cell, const frontline::Army& army, bool selected,
	              const frontline::General& general, const Texture& faces);
	void OfficerCard(const RectF& rect, const frontline::General& officer, const Texture& faces);
} // namespace campaignvisual
