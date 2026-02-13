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
		while (reader.readLine(line))
		{
			if (line.isEmpty()) continue;

			// CSV解析
			Array<String> columns = line.split(U',');

			if (columns.size() < 12)
			{
				Print << U"[WARNING] 不正な行をスキップ: " << line;
				continue;
			}

			CityData city;
			city.name = columns[0].trimmed();
			city.owner = columns[1].trimmed();

			// 座標
			int x = Parse<int>(columns[2].trimmed());
			int y = Parse<int>(columns[3].trimmed());
			city.pos = Point(x, y);

			// パラメータ
			city.troops = Parse<int>(columns[4].trimmed());
			city.gold = Parse<int>(columns[5].trimmed());
			city.food = Parse<int>(columns[6].trimmed());
			city.agriculture = Parse<int>(columns[7].trimmed());
			city.commerce = Parse<int>(columns[8].trimmed());
			city.barracks = Parse<int>(columns[9].trimmed());
			city.order = Parse<int>(columns[10].trimmed());

			// 勢力色（RGBを16進数で指定）
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

		// データ行を読み込む
		String line;
		while (reader.readLine(line))
		{
			if (line.isEmpty()) continue;

			// CSV解析
			Array<String> columns = line.split(U',');

			if (columns.size() < 5)
			{
				Print << U"[WARNING] 不正な行をスキップ: " << line;
				continue;
			}

			String officerName = columns[0].trimmed();
			String faction = columns[1].trimmed();
			String cityName = columns[2].trimmed();
			int leadership = Parse<int>(columns[3].trimmed());
			int power = Parse<int>(columns[4].trimmed());

			// 該当する都市を探して武将を追加
			for (auto& city : cities)
			{
				if (city.name == cityName && city.owner == faction)
				{
					Officer officer;
					officer.name = officerName;
					officer.leadership = leadership;
					officer.power = power;
					officer.war = power;  // 既存コードとの互換性
					city.officers.push_back(officer);
					officerCount++;
					break;
				}
			}
		}

		Print << U"[INFO] 武将データを読み込みました: " << officerCount << U" 人";
	}

private:
	// 16進数カラーコードをColorに変換
	static Color ParseColor(const String& hexColor)
	{
		String hex = hexColor;

		// "#" を削除
		if (hex.starts_with(U'#'))
		{
			hex = hex.substr(1);
		}

		// デフォルト色
		if (hex.length() != 6)
		{
			return Color(128, 128, 128);
		}

		try
		{
			int r = ParseInt<int>(hex.substr(0, 2), Arg::radix = 16);
			int g = ParseInt<int>(hex.substr(2, 2), Arg::radix = 16);
			int b = ParseInt<int>(hex.substr(4, 2), Arg::radix = 16);

			return Color(r, g, b);
		}
		catch (...)
		{
			return Color(128, 128, 128);
		}
	}
};