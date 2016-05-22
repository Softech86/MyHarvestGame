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

	inline static GameCalculate& getCalculate() {
        return *(inst->gclc);
    }

	inline static GamePaint& getPaint() {
        return *(inst->gpat);
    }

    inline static GameLive& getLive() {
        return *(inst->gdyd);
    }

	inline static GameBase& getBase() {
        return *(inst->gstd);
    }
};
