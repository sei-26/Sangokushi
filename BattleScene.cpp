#include "BattleScene.hpp"
#include "BattleSystem.hpp"
#include "GameManager.hpp"

BattleScene::BattleScene(const CityData& atkCity,
						 const CityData& defCity,
						 const Officer& leader,
						 int soldiers)
{
	BattleSetup st;
	st.attackerCity = new CityData(atkCity);
	st.defenderCity = new CityData(defCity);
	st.attackerLeader = new Officer(leader);

	if (!defCity.officers.isEmpty())
		st.defenderLeader = new Officer(defCity.officers[0]);
	else
		st.defenderLeader = new Officer(-1, U"名無し", 0);

	st.attackerSoldiers = soldiers;
	st.defenderSoldiers = defCity.troops;

	system = std::make_unique<BattleSystem>(st);
}

void BattleScene::update()
{
	auto& mgr = system->GetManager();
	mgr.UpdatePKBattle();

	if (system->IsFinished())
	{
		system->ApplyResultToCities();
		m_sceneEnd = true;
		m_nextScene = U"WorldMapScene";
	}
}


void BattleScene::draw() const
{
	Scene::SetBackground(ColorF(0.1));

	auto& mgr = system->GetManager();

	// ★ SRPGマップ＋ユニット描画
	mgr.DrawSRPGBoard();

	// ★ 8PK式 HPバーUI
	mgr.DrawPKUI();
}
