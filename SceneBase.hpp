#pragma once
#include <Siv3D.hpp>

class SceneBase
{
protected:
	bool m_isEnd = false;
	String m_nextScene;

public:
	// デストラクタ（仮想）
	virtual ~SceneBase() = default;

	// 更新と描画
	virtual void update() = 0;
	virtual void draw() const = 0;

	// ★ Scene 終了判定
	bool isSceneEnd() const { return m_isEnd; }

	// ★ 次のシーン名
	String nextSceneName() const { return m_nextScene; }

	// （必要ならデータ受け渡しもここに）
};
