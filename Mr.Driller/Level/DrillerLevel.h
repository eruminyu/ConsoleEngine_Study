#pragma once

#include <Level/Level.h>

// 미스터 드릴러 게임 전체를 관리할 학습용 레벨.
class DrillerLevel : public Craft::Level
{
	TYPE_DECLARATIONS(DrillerLevel, Level)

public:
	virtual void OnInitialized() override;
	virtual void Draw() override;
};
