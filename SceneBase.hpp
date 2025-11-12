#pragma once
#include <Siv3D.hpp>

class SceneBase
{
protected:
	bool m_isEnd = false;
	String m_nextScene; // 例: "WorldMapScene", "City:新野"

public:
	virtual ~SceneBase() = default;
	virtual void update() = 0;
	virtual void draw() const = 0;

	bool isSceneEnd() const { return m_isEnd; }
	const String& nextSceneName() const { return m_nextScene; }
};
