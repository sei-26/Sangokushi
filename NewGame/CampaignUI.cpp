#include "CampaignUI.hpp"

namespace campaignui
{
	ColorF FactionColor(int faction)
	{
		return faction == 0   ? ColorF(.32, .63, .43)
		       : faction == 1 ? ColorF(.38, .57, .78)
		       : faction == 2 ? ColorF(.78, .36, .28)
		                      : ColorF(0.40, 0.43, 0.42);
	}
	void DrawButton(const Rect& rect, const String& label, bool enabled)
	{
		rect.movedBy(0, 2).rounded(3).draw(ColorF(.015, .025, .02, .6));
		rect.rounded(3).draw(enabled && rect.mouseOver() ? ColorF(.22, .29, .23) : ColorF(.105, .16, .135));
		rect.rounded(3).drawFrame(1, enabled ? ColorF(.63, .55, .35) : ColorF(.24, .28, .24));
		Line(rect.x + 4, rect.y + 2, rect.rightX() - 4, rect.y + 2)
		    .draw(1, ColorF(.91, .81, .52, enabled ? .2 : .05));
		FontAsset(U"campaignBody")(label).drawAt(rect.center(),
		                                         enabled ? ColorF(0.94, 0.93, 0.86) : ColorF(0.45));
	}
} // namespace campaignui
