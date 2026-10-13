#include "CampaignUI.hpp"

namespace campaignui
{
	ColorF FactionColor(int faction)
	{
		return faction == 0   ? ColorF(.13, .86, .46)
		       : faction == 1 ? ColorF(.22, .52, 1.)
		       : faction == 2 ? ColorF(1., .32, .17)
		                      : ColorF(.59, .61, .59);
	}
	void Panel(const RectF& rect, bool raised)
	{
		rect.movedBy(0, 3).draw(ColorF(.01, .025, .05, .45));
		rect.draw(Arg::top = ColorF(.105, .17, .24, .97), Arg::bottom = ColorF(.035, .075, .12, .97));
		rect.drawFrame(1, ColorF(.59, .62, .60, .85));
		Line(rect.x, rect.y + 1, rect.rightX(), rect.y + 1).draw(1, ColorF(.79, .73, .56, .80));
		if (raised)
			RectF(rect.x + 1, rect.y + 2, 3, rect.h - 4).draw(ColorF(.80, .66, .36));
	}
	void DrawButton(const Rect& rect, const String& label, bool enabled)
	{
		const bool hover = enabled && rect.mouseOver();
		rect.draw(Arg::top = hover ? ColorF(.24, .34, .44) : ColorF(.15, .23, .31),
		          Arg::bottom = hover ? ColorF(.10, .19, .29) : ColorF(.055, .11, .18));
		rect.drawFrame(1, enabled ? ColorF(.59, .62, .59) : ColorF(.24, .30, .34));
		Line(rect.x + 2, rect.y + 1, rect.rightX() - 2, rect.y + 1)
		    .draw(1, ColorF(.90, .84, .67, enabled ? .38 : .08));
		FontAsset(rect.h < 30 ? U"campaignSmall" : U"campaignBody")(label).drawAt(
		    rect.center(), enabled ? ColorF(.96, .95, .88) : ColorF(.46, .52, .55));
	}
} // namespace campaignui
