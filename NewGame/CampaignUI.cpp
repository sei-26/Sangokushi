#include "CampaignUI.hpp"

namespace campaignui
{
	ColorF FactionColor(int faction)
	{
		return faction == 0   ? ColorF(0.25, 0.72, 0.53)
		       : faction == 1 ? ColorF(0.31, 0.53, 0.91)
		       : faction == 2 ? ColorF(0.88, 0.37, 0.30)
		                      : ColorF(0.40, 0.43, 0.42);
	}
	void DrawButton(const Rect& rect, const String& label, bool enabled)
	{
		rect.rounded(5).draw(enabled && rect.mouseOver() ? ColorF(0.30, 0.39, 0.42)
		                                                 : ColorF(0.16, 0.21, 0.23));
		rect.rounded(5).drawFrame(1, enabled ? ColorF(0.49, 0.60, 0.58) : ColorF(0.25));
		FontAsset(U"campaignBody")(label).drawAt(rect.center(),
		                                         enabled ? ColorF(0.94, 0.93, 0.86) : ColorF(0.45));
	}
} // namespace campaignui
