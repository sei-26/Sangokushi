#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"

// ======================================================
//   侵攻先選択シーン（CityData コピーなし完全版）
// ======================================================
class AttackSelectScene : public SceneBase
{
private:
	// ★ コピー禁止：ポインタに変更
	CityData* m_fromCity = nullptr;
	Array<CityData>* m_allCities = nullptr;

	// ★ 侵攻先もインデックスで管理（安全）
	Optional<int> m_targetIndex;

public:
	// ★ 全て参照（ポインタ）渡しへ
	AttackSelectScene(int fromIndex, Array<CityData>* allCities);

	void update() override;
	void draw() const override;

	// ★ シーン遷移用：インデックスで返す
	int getFromIndex() const { return m_fromCityIndex; }
	int getTargetIndex() const { return m_targetIndex.value(); }

private:
	int m_fromCityIndex = -1;
};
