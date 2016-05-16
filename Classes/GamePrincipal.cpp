#include "GamePrincipal.h"
#include "GameBase.h"
#include "GameCalculate.h"
#include "GameCommand.h"
#include "GameLive.h"
#include "GamePaint.h"
#include "GamePaintPool.h"

GamePrincipal* GamePrincipal::inst = nullptr;

GamePrincipal& GamePrincipal::init() {
	if (GamePrincipal::inst == nullptr) {
		GamePrincipal::inst = new GamePrincipal;
		inst->gcmd = new GameCommand;
		inst->gclc = new GameCalculate;
		inst->gpat = new GamePaint;
		inst->gdyd = new GameLive;
		inst->gstd = new GameBase;
		inst->gppl = new GamePaintPool;
	}
	return *(inst);
}

void GamePrincipal::clear() {
	if (inst != nullptr) {
		if (inst->gcmd != nullptr)
			delete inst->gcmd;
		if (inst->gclc != nullptr)
			delete inst->gclc;
		if (inst->gpat != nullptr)
			delete inst->gpat;
		if (inst->gdyd != nullptr)
			delete inst->gdyd;
		if (inst->gstd != nullptr)
			delete inst->gstd;
		if (inst->gppl != nullptr)
			delete inst->gppl;
	}
}