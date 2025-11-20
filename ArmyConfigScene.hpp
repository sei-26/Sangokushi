#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "Officer.hpp"

class ArmyConfigScene : public SceneBase
{
private:
	// ★ コピー禁止！本物の参照を保持する
	CityData* m_fromCity = nullptr;
	CityData* m_targetCity = nullptr;
	Array<CityData>* m_allCities = nullptr;

	int m_fromIndex = -1;
	int m_targetIndex = -1;

	int m_selectedOfficerIndex = 0;
	int m_soldierAllocation = 500;

public:
	ArmyConfigScene(int fromIndex, int targetIndex, Array<CityData>* allCities);

	void update() override;
	void draw() const override;

	// ★ コピー禁止 → インデックスで返す
	int getFromIndex() const { return m_fromIndex; }
	int getTargetIndex() const { return m_targetIndex; }

	Officer getSelectedOfficer() const;
	int getSoldierAllocation() const;
};
