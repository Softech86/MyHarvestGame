#pragma once
#include "GameUI.h"
#include "GameLive.h"

class SeedSellerLinker : public GameLinker {
public:
	SHCP_OVERRIDE(GameLinker, SeedSellerLinker);
	virtual LinkerReturn link(GameCommand, GameObjectJudge&) override;
};