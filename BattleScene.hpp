#pragma once
#include "SceneBase.hpp"
#include "GameManager.hpp"
#include "CityData.hpp"
#include "Officer.hpp"
#include "BattleSystem.hpp"

class BattleScene : public SceneBase
{
private:
	std::unique_ptr<BattleSystem> system;  // ★これだけ

public:
	BattleScene(const CityData& atkCity,
				const CityData& defCity,
				const Officer& leader,
				int soldiers);

	void update() override;
	void draw() const override;
};
