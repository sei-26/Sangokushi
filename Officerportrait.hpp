#pragma once
#include <Siv3D.hpp>
#include "Officer.hpp"

// 武将の顔アイコンを自動生成
class OfficerPortrait
{
public:
	// 武将の顔アイコンを描画
	static void Draw(const Officer& officer, const Vec2& pos, double size = 80.0)
	{
		// 武将名からハッシュ値を生成して色を決定
		uint32 hash = officer.name.hash();

		// 基本色を決定（統率・武力・知力・政治から）
		double hue = (officer.leadership + officer.power * 2 + officer.intelligence * 3) % 360;
		Color baseColor = HSV(hue, 0.6, 0.8);
		Color darkColor = HSV(hue, 0.7, 0.5);

		// 顔の背景円
		Circle(pos, size / 2).draw(baseColor);

		// 顔の輪郭
		Circle(pos, size / 2).drawFrame(size / 20, ColorF(0.2, 0.2, 0.2, 0.8));

		// 目（武力が高いほど鋭い目）
		double eyeY = pos.y - size * 0.1;
		double eyeWidth = size * 0.15;
		double eyeHeight = size * 0.08 + (officer.power / 100.0) * size * 0.05;

		// 左目
		Ellipse(pos.x - size * 0.2, eyeY, eyeWidth, eyeHeight).draw(Palette::White);
		Circle(pos.x - size * 0.2, eyeY, eyeHeight).draw(ColorF(0.1, 0.1, 0.1));

		// 右目
		Ellipse(pos.x + size * 0.2, eyeY, eyeWidth, eyeHeight).draw(Palette::White);
		Circle(pos.x + size * 0.2, eyeY, eyeHeight).draw(ColorF(0.1, 0.1, 0.1));

		// 口（知力が高いほど微笑み）
		double mouthY = pos.y + size * 0.15;
		double mouthCurve = -0.2 + (officer.intelligence / 100.0) * 0.4;
		Bezier2 mouth(
			Vec2(pos.x - size * 0.2, mouthY),
			Vec2(pos.x, mouthY + size * mouthCurve),
			Vec2(pos.x + size * 0.2, mouthY)
		);
		mouth.draw(size / 30, ColorF(0.2, 0.1, 0.1));

		// 髭・髪飾り（統率が高いほど豪華）
		if (officer.leadership >= 80)
		{
			// 豪華な髭
			for (int i = -1; i <= 1; ++i)
			{
				Bezier2 beard(
					Vec2(pos.x + size * 0.3 * i, pos.y + size * 0.3),
					Vec2(pos.x + size * 0.4 * i, pos.y + size * 0.45),
					Vec2(pos.x + size * 0.35 * i, pos.y + size * 0.5)
				);
				beard.draw(size / 25, darkColor);
			}
		}

		// 政治が高い場合は冠を表示
		if (officer.politics >= 80)
		{
			RectF(pos.x - size * 0.3, pos.y - size * 0.5, size * 0.6, size * 0.1)
				.draw(HSV(40, 0.8, 0.9));
			Triangle(
				pos.x, pos.y - size * 0.5,
				pos.x - size * 0.15, pos.y - size * 0.4,
				pos.x + size * 0.15, pos.y - size * 0.4
			).draw(HSV(40, 0.9, 1.0));
		}

		// 能力値に応じたオーラ
		if (officer.GetCombatPower() >= 180 || officer.GetAdministrationPower() >= 180)
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			Circle(pos, size / 2 + 5 + sin(Scene::Time() * 3) * 3)
				.drawFrame(2, ColorF(baseColor, 0.5));
		}
	}

	// 武将の名前プレートを描画
	static void DrawNamePlate(const Officer& officer, const Vec2& pos, double size = 80.0)
	{
		// 名前背景
		RectF nameRect(Arg::center(pos.x, pos.y + size * 0.6), size * 1.2, size * 0.3);
		nameRect.draw(ColorF(0, 0, 0, 0.7));
		nameRect.drawFrame(1, ColorF(0.8, 0.7, 0.5));

		// 名前
		FontAsset(U"menu")(officer.name)
			.drawAt(nameRect.center(), ColorF(1.0, 0.95, 0.8));
	}

	// 武将の詳細情報を表示
	static void DrawDetailPanel(const Officer& officer, const Vec2& pos, double width = 300.0)
	{
		// パネル背景
		RectF panel(pos, width, 200);
		panel.draw(ColorF(0.1, 0.08, 0.06, 0.95));
		panel.drawFrame(3, ColorF(0.8, 0.7, 0.5));

		// 顔アイコン
		Draw(officer, Vec2(pos.x + 50, pos.y + 60), 80);

		// 名前
		FontAsset(U"title")(officer.name)
			.draw(pos.x + 110, pos.y + 10, ColorF(1.0, 0.95, 0.8));

		// 能力値
		int statX = static_cast<int>(pos.x + 110);
		int statY = static_cast<int>(pos.y + 50);

		FontAsset(U"menu")(U"統率: {}"_fmt(officer.leadership))
			.draw(statX, statY, Palette::Skyblue);
		FontAsset(U"menu")(U"武力: {}"_fmt(officer.power))
			.draw(statX, statY + 30, Palette::Red);
		FontAsset(U"menu")(U"知力: {}"_fmt(officer.intelligence))
			.draw(statX, statY + 60, Palette::Purple);
		FontAsset(U"menu")(U"政治: {}"_fmt(officer.politics))
			.draw(statX, statY + 90, Palette::Gold);

		// 総合評価
		int total = officer.leadership + officer.power + officer.intelligence + officer.politics;
		String rank = U"C";
		if (total >= 360) rank = U"S";
		else if (total >= 340) rank = U"A";
		else if (total >= 320) rank = U"B";

		FontAsset(U"title")(U"総合: " + rank)
			.draw(statX + 130, statY + 30,
				  rank == U"S" ? Palette::Gold : rank == U"A" ? Palette::Silver : Palette::White);
	}

	// 小さいアイコン（マップ用）
	static void DrawSmall(const Officer& officer, const Vec2& pos, double size = 30.0)
	{
		uint32 hash = officer.name.hash();
		double hue = (officer.leadership + officer.power * 2) % 360;
		Color baseColor = HSV(hue, 0.6, 0.8);

		Circle(pos, size / 2).draw(baseColor);
		Circle(pos, size / 2).drawFrame(size / 10, ColorF(0.2, 0.2, 0.2, 0.8));

		// イニシャル表示
		if (officer.name.length() > 0)
		{
			String initial = officer.name.substr(0, 1);
			FontAsset(U"small")(initial).drawAt(pos, Palette::White);
		}
	}
};
