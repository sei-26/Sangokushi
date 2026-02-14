#pragma once
#include <Siv3D.hpp>
#include "Officer.hpp"
#include "CityData.hpp"

// CSVからデータを読み込むクラス
class CSVDataLoader
{
public:
	// 都市データを読み込む
	static Array<CityData> LoadCities(const FilePath& csvPath)
	{
		Array<CityData> cities;

		TextReader reader(csvPath);
		if (!reader)
		{
			Print << U"[ERROR] CSVファイルが見つかりません: " << csvPath;
			return cities;
		}

		// ヘッダー行をスキップ
		String header;
		reader.readLine(header);

		// データ行を読み込む
		String line;
		int lineNum = 1;
		while (reader.readLine(line))
		{
			lineNum++;
			if (line.isEmpty()) continue;

			// CSV解析
			Array<String> columns = line.split(U',');

			if (columns.size() < 12)
			{
				Print << U"[WARNING] 行" << lineNum << U": 列数不足をスキップ";
				continue;
			}

			CityData city;
			city.name = columns[0].trimmed();
			city.owner = columns[1].trimmed();

			// ★ 安全な数値解析（Optional使用）
			if (auto x = ParseOpt<int>(columns[2].trimmed()))
			{
				if (auto y = ParseOpt<int>(columns[3].trimmed()))
				{
					city.pos = Point(*x, *y);
				}
			}

			if (auto troops = ParseOpt<int>(columns[4].trimmed()))
				city.troops = *troops;
			if (auto gold = ParseOpt<int>(columns[5].trimmed()))
				city.gold = *gold;
			if (auto food = ParseOpt<int>(columns[6].trimmed()))
				city.food = *food;
			if (auto agriculture = ParseOpt<int>(columns[7].trimmed()))
				city.agriculture = *agriculture;
			if (auto commerce = ParseOpt<int>(columns[8].trimmed()))
				city.commerce = *commerce;
			if (auto barracks = ParseOpt<int>(columns[9].trimmed()))
				city.barracks = *barracks;
			if (auto order = ParseOpt<int>(columns[10].trimmed()))
				city.order = *order;

			// 勢力色
			String colorHex = columns[11].trimmed();
			city.color = ParseColor(colorHex);

			cities.push_back(city);
		}

		Print << U"[INFO] 都市データを読み込みました: " << cities.size() << U" 都市";
		return cities;
	}

	// 武将データを読み込んで都市に配置
	static void LoadOfficers(Array<CityData>& cities, const FilePath& csvPath)
	{
		TextReader reader(csvPath);
		if (!reader)
		{
			Print << U"[ERROR] CSVファイルが見つかりません: " << csvPath;
			return;
		}

		// ヘッダー行をスキップ
		String header;
		reader.readLine(header);

		int officerCount = 0;
		int lineNum = 1;

		// データ行を読み込む
		String line;
		while (reader.readLine(line))
		{
			lineNum++;
			if (line.isEmpty()) continue;

			// CSV解析
			Array<String> columns = line.split(U',');

			if (columns.size() < 7)
			{
				Print << U"[WARNING] 行" << lineNum << U": 列数不足（" << columns.size() << U"列）";
				continue;
			}

			String officerName = columns[0].trimmed();
			String faction = columns[1].trimmed();
			String cityName = columns[2].trimmed();

			// ★ ParseOptで安全に解析
			int leadership = 50;
			int power = 50;
			int intelligence = 50;
			int politics = 50;

			if (auto val = ParseOpt<int>(columns[3].trimmed()))
				leadership = *val;
			if (auto val = ParseOpt<int>(columns[4].trimmed()))
				power = *val;
			if (auto val = ParseOpt<int>(columns[5].trimmed()))
				intelligence = *val;
			if (auto val = ParseOpt<int>(columns[6].trimmed()))
				politics = *val;

			// 該当する都市を探して武将を追加
			bool placed = false;
			for (auto& city : cities)
			{
				if (city.name == cityName && city.owner == faction)
				{
					Officer officer;
					officer.name = officerName;
					officer.leadership = leadership;
					officer.power = power;
					officer.war = power;
					officer.intelligence = intelligence;
					officer.politics = politics;
					city.officers.push_back(officer);
					officerCount++;
					placed = true;
					break;
				}
			}

			if (!placed)
			{
				Print << U"[WARNING] 武将 " << officerName << U" の配置先が見つかりません（都市:" << cityName << U", 勢力:" << faction << U"）";
			}
		}

		Print << U"[INFO] 武将データを読み込みました: " << officerCount << U" 人";
	}

private:
	// 16進数カラーコードをColorに変換
	static Color ParseColor(const String& hexColor)
	{
		String hex = hexColor.trimmed();

		// "#" を削除
		if (hex.starts_with(U'#'))
		{
			hex = hex.substr(1);
		}

		// 長さチェック
		if (hex.length() != 6)
		{
			return Color(128, 128, 128);
		}

		// ★ 1文字ずつ解析
		auto parseHexDigit = [](char32 c) -> int {
			if (c >= U'0' && c <= U'9') return c - U'0';
			if (c >= U'a' && c <= U'f') return c - U'a' + 10;
			if (c >= U'A' && c <= U'F') return c - U'A' + 10;
			return 0;
			};

		int r = parseHexDigit(hex[0]) * 16 + parseHexDigit(hex[1]);
		int g = parseHexDigit(hex[2]) * 16 + parseHexDigit(hex[3]);
		int b = parseHexDigit(hex[4]) * 16 + parseHexDigit(hex[5]);

		return Color(r, g, b);
	}
};
