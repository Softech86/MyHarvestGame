#include "GameUI.h"
#include "GameBase.h"
#include "GameLive.h"
#include "GamePrincipal.h"
#include "HelloScene.h"
#include "GamePaint.h"

GameUI GameUI::origin;

LiveCode GameUI::start() {
	LiveCode result = cocos2d::Node::create();
	return result;
}

#if defined(LINUX) || defined(__APPLE__) || defined(APPLE)
JudgeReturn GameUI::action(__attribute__((unused)) LiveCode node, __attribute__((unused)) float* keyarray) {
#endif
#ifdef WIN32
JudgeReturn GameUI::action(LiveCode node, float* keyarray) {
	UNREFERENCED_PARAMETER(keyarray);
	UNREFERENCED_PARAMETER(node);
#endif
    return JudgeReturn::judgeEnd;
}

string GameUI::save() {
    return "test";
}

void GameUI::stop() {
    
}

LiveCode StartPageUI::start() {
    LiveCode result = nullptr; 
	result = Hello::create();
    return result;
}

JudgeReturn StartPageUI::action(LiveCode node, float* keyarray) {
    GameCommand gcmd = this->control()->translate(keyarray);
	dynamic_cast<Hello*>(node)->comeOn(node,gcmd);
    return JudgeReturn::judgeEnd;
}

LiveCode KidMoveUI::start() {
	LiveCode result = cocos2d::Node::create();
	return result;
}

JudgeReturn KidMoveUI::action(LiveCode node, float* keyarray) {
	if (LIVE.api_hasScene()) {
		GameCommand gcmd = this->control()->translate(keyarray);
		if (gcmd == GameCommand::emptyCmd)
			return JudgeReturn::judgeEnd;
		float now = PAINT.clock();
		if (_lockUntil > now)
			return JudgeReturn::judgeEnd;
		else {
			BlockPos dist = LIVE.api_getScenePtr()->getStepDist(BlockPos::cmdToDir(gcmd));

			LIVE.api_kidWalk(dist);
			_lockUntil = now + LIVE.api_getScenePtr()->getKidWalkTime(dist);
		}
	}
	return JudgeReturn::judgeEnd;
}


// 像这种Cycle和Just都是有种算时间差的感觉的，就是你只要一组按钮不是同一个时间差的话，是不会同时判定的吧
GameCommand BasicMenuTranslator::translate(float* keyarray) {
	if (LIVE.keyCyclePushedOnly(keyarray, GameKeyPress::buttonUp, KEY_CYCLE_SEC))
		return selectUp;
	else if (LIVE.keyCyclePushedOnly(keyarray, GameKeyPress::buttonDown, KEY_CYCLE_SEC))
		return selectDown;
	else if (LIVE.keyCyclePushedOnly(keyarray, GameKeyPress::buttonLeft, KEY_CYCLE_SEC))
		return selectLeft;
	else if (LIVE.keyCyclePushedOnly(keyarray, GameKeyPress::buttonRight, KEY_CYCLE_SEC))
		return selectRight;
	else if (LIVE.keyJustPushedOnly(keyarray, GameKeyPress::buttonA))
		return confirm;
	else if (LIVE.keyJustPushedOnly(keyarray, GameKeyPress::buttonB))
		return cancel;
	else if (LIVE.keyJustPushedOnly(keyarray, GameKeyPress::buttonSpace))
		return menu;
	else if (LIVE.keyJustPushedOnly(keyarray, GameKeyPress::buttonStart))
		return detail;
    return GameCommand::emptyCmd;
}

GameCommand BasicMoveTranslator::translate(float* keyarray) {
	// 这里的锁什么的交给运动UI部分自己去想
	if (LIVE.keyPushedOnly(keyarray, buttonUp))
		return walkEight;
	else if (LIVE.keyPushedOnly(keyarray, buttonDown))
		return walkTwo;
	else if (LIVE.keyPushedOnly(keyarray, buttonLeft))
		return walkFour;
	else if (LIVE.keyPushedOnly(keyarray, buttonRight))
		return walkSix;

	vector<GameKeyPress> vgkp = { buttonUp, buttonLeft };
	if (LIVE.keyPushedOnly(keyarray, vgkp))
		return walkSeven;
	vgkp[1] = buttonRight;
	if (LIVE.keyPushedOnly(keyarray, vgkp))
		return walkNine;
	vgkp[0] = buttonDown;
	if (LIVE.keyPushedOnly(keyarray, vgkp))
		return walkThree;
	vgkp[1] = buttonLeft;
	if (LIVE.keyPushedOnly(keyarray, vgkp))
		return walkOne;

	return GameCommand::emptyCmd;
}

GameCommand BasicObjectTranslator::translate(float* keyarray) {
	if(LIVE.keyPushedOnly(keyarray, GameKeyPress::buttonA))
		return GameCommand::pick;
	//TODO

	return GameCommand::emptyCmd;
}

GameCommand HandTranslator::translate(float* keyarray) {
	if (LIVE.keyPushedOnly(keyarray, GameKeyPress::buttonA))
		return GameCommand::drop;

	//TODO
	return GameCommand::emptyCmd;
}


bool StartGameEvent::start(LiveObjPtr obj) {
	LIVE.api_sceneICD(farmSceneCode, BlockPos(200, 200), BlockPos(PxPos(960, 640)));
	LIVE.api_kidSet(KidCode, BigBlockPos(3, 1), true);
	/*
	LIVE.api_kidWalk(BigBlockPos(3, 18));
	LIVE.api_kidWalk(BigBlockPos(12, -15));
	LIVE.api_kidWalk(BigBlockPos(-1, 15));
	LIVE.api_kidWalk(BigBlockPos(-12, -15));


	LIVE.api_kidWalk(BigBlockPos(0, -1));
	LIVE.api_kidWalk(BigBlockPos(-1, 0));
	LIVE.api_kidWalk(BigBlockPos(0, -1));
	LIVE.api_kidWalk(BigBlockPos(-1, 0));
	LIVE.api_kidWalk(BigBlockPos(0, -1));
	LIVE.api_kidWalk(BigBlockPos(-1, 0));
	LIVE.api_kidWalk(BigBlockPos(0, 1));
	LIVE.api_kidWalk(BigBlockPos(1, 0));
	LIVE.api_kidWalk(BigBlockPos(2, 0));
	LIVE.api_kidWalk(BigBlockPos(0, 2));
	LIVE.api_kidWalk(BigBlockPos(0, 1));
	LIVE.api_kidWalk(BigBlockPos(1, 0));
	LIVE.api_kidWalk(BigBlockPos(2, 0));
	LIVE.api_kidWalk(BigBlockPos(0, 2));
	LIVE.api_kidWalk(BigBlockPos(0, 1));
	LIVE.api_kidWalk(BigBlockPos(1, 0));
	LIVE.api_kidWalk(BigBlockPos(2, 0));
	LIVE.api_kidWalk(BigBlockPos(0, 2));
	LIVE.api_kidWalk(BigBlockPos(0, 1));
	LIVE.api_kidWalk(BigBlockPos(1, 0));
	LIVE.api_kidWalk(BigBlockPos(7, 0));
	LIVE.api_kidWalk(BigBlockPos(-18, -11));*/

	LIVE.api_stopUIJudgedNow();
	LIVE.api_UIStart(kidMoveUICode);

	return true;
}

