#pragma once

class GameCalculate;
class GamePaint;
class GameLive;
class GameBase;

// Run in Single Instance Mode

class GamePrincipal {
private:
    static GamePrincipal* inst;
    GameCalculate* gclc = nullptr;
    GamePaint* gpat = nullptr;
    GameLive* gdyd = nullptr;
    GameBase* gstd = nullptr;
public:
    static GamePrincipal& init();
    static void GAMEMAIN();
    static void clear();

    static GameCalculate& getCalculate() {
        return *(inst->gclc);
    }

    static GamePaint& getPaint() {
        return *(inst->gpat);
    }

    static GameLive& getLive() {
        return *(inst->gdyd);
    }

    static GameBase& getBase() {
        return *(inst->gstd);
    }
};
