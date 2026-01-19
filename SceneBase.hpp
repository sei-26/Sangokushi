#pragma once
#include <Siv3D.hpp>

// シーンの基底クラス
class SceneBase
{
public:
	virtual ~SceneBase() = default;

	virtual void update() = 0;
	virtual void draw() const = 0;

	// ★ これらを追加！
	// シーンが終わったかどうか
	bool isSceneEnd() const { return m_sceneEnd; }

	// 次のシーン名を取得
	String getNextScene() const { return m_nextScene; }

protected:
	// 遷移管理用フラグ
	bool m_sceneEnd = false;
	String m_nextScene;
};
