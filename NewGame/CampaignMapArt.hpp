#pragma once
#include <Siv3D.hpp>
#include "CampaignTypes.hpp"

namespace campaignvisual
{
	// Raster art is presentation only; the simulation continues to use Campaign::tiles.
	class MapArt
	{
	public:
		MapArt();
		bool Ground(const s3d::RectF& rect, frontline::Terrain terrain, int tile, double pitch) const;
		bool Relief(s3d::Vec2 center, double width, frontline::Terrain terrain, int tile) const;
		bool City(s3d::Vec2 center, double cell, int faction, bool selected) const;
		bool Town(s3d::Vec2 center, double cell, int faction) const;

	private:
		s3d::Texture m_sprites;
		s3d::Texture m_materials;
		void Sprite(int index, s3d::Vec2 center, double size, double baseline) const;
	};
} // namespace campaignvisual
