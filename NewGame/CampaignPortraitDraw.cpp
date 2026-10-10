#include "CampaignVisuals.hpp"
#include "CampaignUI.hpp"
#include "PortraitCatalog.hpp"

namespace campaignvisual
{
	void OfficerCard(const RectF& r, const frontline::General& officer, const Texture& faces)
	{
		static const std::array<std::u32string, 6> names{
		    {U"劉備", U"関羽", U"張飛", U"趙雲", U"諸葛亮", U"黄忠"}};
		r.draw(ColorF(.07, .115, .095));
		int who = -1;
		for (int i = 0; i < 6; ++i)
			if (officer.name == names[i])
				who = i;
		static const std::array<Texture, 3> portraits = [] {
			std::array<Texture, 3> textures;
			const String paths[]{U"OfficerArt/shu-portraits-v1.png", U"OfficerArt/wei-portraits-v1.png",
			                     U"OfficerArt/wu-portraits-v1.png"};
			for (int i = 0; i < 3; ++i)
				if (FileSystem::Exists(paths[i]))
					textures[i] = Texture(paths[i], TextureDesc::Mipped);
			return textures;
		}();
		const int index = PortraitIndex(officer.name);
		if (index >= 0 && portraits[index / 24])
		{
			const auto& atlas = portraits[index / 24];
			const auto crop = PortraitCrop(index, atlas.width(), atlas.height());
			atlas(Rect(crop[0], crop[1], crop[2], crop[3])).resized(r.w, r.h).draw(r.pos);
		}
		else if (who >= 0 && faces)
		{
			const int w = faces.width() / 3, h = faces.height() / 2;
			faces(Rect(w * (who % 3), h * (who / 3), w, h)).resized(r.w, r.h).draw(r.pos);
		}
		else
		{
			Circle(r.center().movedBy(0, -10), 13).draw(ColorF(.48, .48, .32));
			Triangle(r.center().movedBy(0, 0), r.bl().movedBy(5, -4), r.br().movedBy(-5, -4))
			    .draw(campaignui::FactionColor(officer.faction));
			FontAsset(U"campaignSmall")(String(officer.name.substr(0, 1).c_str()))
			    .drawAt(r.center().movedBy(0, -10), ColorF(.96, .87, .64));
		}
		r.drawFrame(1.5, ColorF(.74, .61, .35));
	}
} // namespace campaignvisual
