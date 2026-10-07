#pragma once
#include <Siv3D.hpp>

namespace campaignui
{
	ColorF FactionColor(int faction);
	void DrawButton(const Rect& rect, const String& label, bool enabled = true);
} // namespace campaignui
