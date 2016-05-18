#include "GamePaint.h"

cocos2d::Scene* GamePaint::mainsc = nullptr;
LiveCode GamePaint::nodeNew() {
    //TODO
	auto background = cocos2d::Sprite::create("tollgateBG.jpg");
	if (!background)
		return nullptr;
	return background;
}

bool GamePaint::nodeDisplay(LiveCode needDisplayed) {
    //TODO
	GamePaint::mainsc->addChild(needDisplayed);

    return true;
}

void GamePaint::nodeClear(LiveCode needRemoved) {
	GamePaint::mainsc->removeChild(needRemoved);
}

LiveCode GamePaint::objAddToObj(LiveCode parent, const string& picture, const PxPos& pos, float scale, float alpha) {
    //TODO
    return nullptr;
}

LiveCode GamePaint::objMove(LiveCode object, const PxPos& oldpos, const PxPos& newpos, MoveType type, float timeSec){
    //TODO
    return nullptr;
}

LiveCode GamePaint::objRotate(LiveCode object, float olddegree, float newdegree, float timeSec){
    //TODO
    return nullptr;
}

LiveCode GamePaint::objAlpha(LiveCode object, float oldalpha, float newalpha, float timeSec) {
    //TODO
    return nullptr;
}

void GamePaint::objErase(LiveCode object) {
    //TODO
}
