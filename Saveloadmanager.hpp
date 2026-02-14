#pragma once
#include <Siv3D.hpp>
#include "CityData.hpp"
#include "Faction.hpp"

// セーブデータ構造
struct SaveData
{
	int year = 200;
	int month = 1;
	String playerFactionName;
	Array<CityData> cities;
	DateTime saveDateTime;
	int playTimeSeconds = 0;

	// JSONに変換
	JSON toJSON() const
	{
		JSON json;
		json[U"year"] = year;
		json[U"month"] = month;
		json[U"playerFactionName"] = playerFactionName;
		json[U"saveDateTime"] = saveDateTime.format(U"yyyy-MM-dd HH:mm:ss");
		json[U"playTimeSeconds"] = playTimeSeconds;

		// 都市データ
		Array<JSON> citiesJSON;
		for (const auto& city : cities)
		{
			JSON cityJSON;
			cityJSON[U"name"] = city.name;
			cityJSON[U"owner"] = city.owner;
			cityJSON[U"posX"] = city.pos.x;
			cityJSON[U"posY"] = city.pos.y;
			cityJSON[U"troops"] = city.troops;
			cityJSON[U"gold"] = city.gold;
			cityJSON[U"food"] = city.food;
			cityJSON[U"agriculture"] = city.agriculture;
			cityJSON[U"commerce"] = city.commerce;
			cityJSON[U"barracks"] = city.barracks;
			cityJSON[U"order"] = city.order;
			cityJSON[U"colorR"] = city.color.r;
			cityJSON[U"colorG"] = city.color.g;
			cityJSON[U"colorB"] = city.color.b;

			// 武将データ
			Array<JSON> officersJSON;
			for (const auto& officer : city.officers)
			{
				JSON officerJSON;
				officerJSON[U"name"] = officer.name;
				officerJSON[U"leadership"] = officer.leadership;
				officerJSON[U"power"] = officer.power;
				officerJSON[U"intelligence"] = officer.intelligence;
				officerJSON[U"politics"] = officer.politics;
				officersJSON.push_back(officerJSON);
			}
			cityJSON[U"officers"] = officersJSON;

			citiesJSON.push_back(cityJSON);
		}
		json[U"cities"] = citiesJSON;

		return json;
	}

	// JSONから復元
	static Optional<SaveData> fromJSON(const JSON& json)
	{
		SaveData data;

		if (!json.hasElement(U"year") || !json.hasElement(U"cities"))
		{
			return none;
		}

		data.year = json[U"year"].get<int>();
		data.month = json[U"month"].get<int>();
		data.playerFactionName = json[U"playerFactionName"].getString();
		data.playTimeSeconds = json[U"playTimeSeconds"].get<int>();

		// 都市データを復元
		const auto& citiesJSON = json[U"cities"].arrayView();
		for (const auto& cityJSON : citiesJSON)
		{
			CityData city;
			city.name = cityJSON[U"name"].getString();
			city.owner = cityJSON[U"owner"].getString();
			city.pos.x = cityJSON[U"posX"].get<int>();
			city.pos.y = cityJSON[U"posY"].get<int>();
			city.troops = cityJSON[U"troops"].get<int>();
			city.gold = cityJSON[U"gold"].get<int>();
			city.food = cityJSON[U"food"].get<int>();
			city.agriculture = cityJSON[U"agriculture"].get<int>();
			city.commerce = cityJSON[U"commerce"].get<int>();
			city.barracks = cityJSON[U"barracks"].get<int>();
			city.order = cityJSON[U"order"].get<int>();
			city.color = Color(
				cityJSON[U"colorR"].get<int>(),
				cityJSON[U"colorG"].get<int>(),
				cityJSON[U"colorB"].get<int>()
			);

			// 武将データを復元
			if (cityJSON.hasElement(U"officers"))
			{
				const auto& officersJSON = cityJSON[U"officers"].arrayView();
				for (const auto& officerJSON : officersJSON)
				{
					Officer officer;
					officer.name = officerJSON[U"name"].getString();
					officer.leadership = officerJSON[U"leadership"].get<int>();
					officer.power = officerJSON[U"power"].get<int>();
					officer.war = officer.power;
					officer.intelligence = officerJSON[U"intelligence"].get<int>();
					officer.politics = officerJSON[U"politics"].get<int>();
					city.officers.push_back(officer);
				}
			}

			data.cities.push_back(city);
		}

		return data;
	}
};

// セーブ・ロード管理
class SaveLoadManager
{
public:
	// セーブファイルのパス
	static FilePath GetSaveFilePath(int slotIndex)
	{
		return U"save{}.json"_fmt(slotIndex);
	}

	// セーブ
	static bool Save(const SaveData& data, int slotIndex)
	{
		try
		{
			FilePath path = GetSaveFilePath(slotIndex);
			JSON json = data.toJSON();
			json.save(path);

			Print << U"[セーブ] スロット" << slotIndex << U": " << path;
			return true;
		}
		catch (...)
		{
			Print << U"[エラー] セーブ失敗: スロット" << slotIndex;
			return false;
		}
	}

	// ロード
	static Optional<SaveData> Load(int slotIndex)
	{
		try
		{
			FilePath path = GetSaveFilePath(slotIndex);

			if (!FileSystem::Exists(path))
			{
				Print << U"[ロード] セーブファイルが存在しません: " << path;
				return none;
			}

			JSON json = JSON::Load(path);
			auto data = SaveData::fromJSON(json);

			if (data)
			{
				Print << U"[ロード] スロット" << slotIndex << U": " << path;
			}

			return data;
		}
		catch (...)
		{
			Print << U"[エラー] ロード失敗: スロット" << slotIndex;
			return none;
		}
	}

	// セーブファイルの存在確認
	static bool HasSaveFile(int slotIndex)
	{
		return FileSystem::Exists(GetSaveFilePath(slotIndex));
	}

	// セーブファイル情報を取得
	static String GetSaveFileInfo(int slotIndex)
	{
		if (!HasSaveFile(slotIndex))
		{
			return U"--- 空き ---";
		}

		try
		{
			FilePath path = GetSaveFilePath(slotIndex);
			JSON json = JSON::Load(path);

			int year = json[U"year"].get<int>();
			int month = json[U"month"].get<int>();
			String playerName = json[U"playerFactionName"].getString();
			String dateTime = json[U"saveDateTime"].getString();

			return U"{} 年 {} 月 - {} ({})"_fmt(year, month, playerName, dateTime);
		}
		catch (...)
		{
			return U"--- 破損 ---";
		}
	}

	// オートセーブ
	static bool AutoSave(const SaveData& data)
	{
		return Save(data, 0);  // スロット0をオートセーブ用
	}
};
