#pragma once
#include <Siv3D.hpp>

class SceneBase
{
protected:
	bool m_isEnd = false;      // このシーンが終了したか？
	String m_nextScene = U"";  // 次のシーン名

public:
	virtual ~SceneBase() = default;
	virtual void update() = 0;
	virtual void draw() const = 0;

	bool isSceneEnd() const { return m_isEnd; }
	String nextSceneName() const { return m_nextScene; }
};
