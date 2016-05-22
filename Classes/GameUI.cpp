#include "GameUI.h"
#include "GameBase.h"
#include "GameLive.h"
#include "GamePrincipal.h"
#include "HelloScene.h"

GameUI GameUI::origin;

LiveCode GameUI::start() {
    return nullptr;
}

#if defined(LINUX) || defined(__APPLE__) || defined(APPLE)
JudgeReturn GameUI::action(__attribute__((unused)) float* keyarray) {
#endif
#ifdef WIN32
JudgeReturn GameUI::action(float* keyarray) {
	UNREFERENCED_PARAMETER(keyarray);
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

JudgeReturn StartPageUI::action(float* keyarray) {
    GameCommand gcmd = this->control()->translate(keyarray);

    // TODO some code
    return JudgeReturn::judgeEnd;
}

GameCommand BasicMenuTranslator::translate(float* keyarray) {
    //TODO
    return GameCommand::emptyCmd;
}

GameCommand BasicMoveTranslator::translate(float* keyarray) {
	//TODO
	return GameCommand::emptyCmd;
}

GameCommand BasicObjectTranslator::translate(float* keyarray) {
	if(LIVE.keyPushedOnly(keyarray, GameKeyPress::buttonA))
		return GameCommand::pick;
	

	return GameCommand::emptyCmd;
}

GameCommand HandTranslator::translate(float* keyarray) {
	if (LIVE.keyPushedOnly(keyarray, GameKeyPress::buttonA))
		return GameCommand::drop;

	return GameCommand::emptyCmd;
}


bool StartGameEvent::start(LiveObjPtr obj) {
	LIVE.api_sceneICD(farmSceneCode, BlockPos(100, 100), BlockPos(PxPos(960, 640)));
	LIVE.api_kidSet(KidCode, BigBlockPos(3, 1), true);
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
	LIVE.api_kidWalk(BigBlockPos(-18, -11));
	return true;
}

