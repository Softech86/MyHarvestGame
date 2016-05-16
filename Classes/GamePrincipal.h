#pragma once

class GameCommand;
class GameCalculate;
class GamePaint;
class GameLive;
class GameBase;
class GamePaintPool;

// Run in Single Instance Mode
class GamePrincipal {
private:
	static GamePrincipal* inst;
	GameCommand* gcmd = nullptr;
	GameCalculate* gclc = nullptr;
	GamePaint* gpat = nullptr;
	GameLive* gdyd = nullptr;
	GameBase* gstd = nullptr;
	GamePaintPool* gppl = nullptr;
public:
	static GamePrincipal& init();
	static void clear();

	static GameCommand& getCommand() {
		return *(inst->gcmd);
	}
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
	static GamePaintPool& getPool() {
		return *(inst->gppl);
	}
};
