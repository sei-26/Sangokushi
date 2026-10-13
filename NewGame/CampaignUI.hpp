#pragma once
#include <Siv3D.hpp>

namespace campaignui
{
	ColorF FactionColor(int faction);
	void Panel(const s3d::RectF& rect, bool raised = false);
	void DrawButton(const Rect& rect, const String& label, bool enabled = true);
} // namespace campaignui
