#pragma once
#include <Siv3D.hpp>

// すべてのシーン共通のインターフェース
class SceneBase
{
public:
	virtual ~SceneBase() = default;
	virtual void update() = 0;
	virtual void draw() const = 0;
};
