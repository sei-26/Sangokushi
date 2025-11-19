#pragma once
#include <Siv3D.hpp>

class SceneBase
{
protected:
	bool m_sceneEnd = false;     // ← これが全Sceneで使われる
	String m_nextScene = U"";    // ← 次のシーン名

public:
	virtual ~SceneBase() = default;

	// 派生クラスが実装する
	virtual void update() = 0;
	virtual void draw() const = 0;

	// SceneManager が呼ぶ
	bool isSceneEnd() const { return m_sceneEnd; }
	String nextSceneName() const { return m_nextScene; }
};
