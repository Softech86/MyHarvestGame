#include "GamePrincipal.h"
#include "GameBase.h"
#include "GameCalculate.h"
#include "GameLive.h"
#include "GamePaint.h"

GamePrincipal* GamePrincipal::inst = nullptr;

void GamePrincipal::GAMEMAIN() {
	GamePrincipal::init();
	BASE.init();
	PAINT.init();
	LIVE.init();
}

GamePrincipal& GamePrincipal::init() {
    if (GamePrincipal::inst == nullptr) {
        GamePrincipal::inst = new GamePrincipal;
        inst->gclc = new GameCalculate;
        inst->gpat = new GamePaint;
        inst->gdyd = new GameLive;
        inst->gstd = new GameBase;
    }
    return *(inst);
}

void GamePrincipal::clear() {
    if (inst != nullptr) {
        if (inst->gclc != nullptr)
            delete inst->gclc;
        if (inst->gpat != nullptr)
            delete inst->gpat;
        if (inst->gdyd != nullptr)
            delete inst->gdyd;
        if (inst->gstd != nullptr)
            delete inst->gstd;
    }
}