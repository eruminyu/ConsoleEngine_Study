#pragma once

#include <Level/Level.h>

class __declspec(dllexport) TestLevel : public Craft::Level
{
public:
	virtual void OnInitialized() override;
};
