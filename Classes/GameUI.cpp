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
    return JudgeReturn::judgeNextObject;
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
			return JudgeReturn::judgeNextObject;
		float now = PAINT.clock();
		LiveObjPtr kid = LIVE.api_kidGet();
		if (kid != nullptr) {
			if (LIVE.api_getActionLock(kid) > now)
				return JudgeReturn::judgeEnd;
			else {
				BlockPos dist = LIVE.api_getScenePtr()->getStepDist(BlockPos::cmdToDir(gcmd));
				auto iswalk = BASE.cmdWalkOrRun(gcmd);
				if (iswalk == BASE.WALK) {
					LIVE.api_kidWalk(dist);
					LIVE.api_setActionLock(kid, now + LIVE.api_getScenePtr()->getKidWalkTime(dist));
				}
				else if (iswalk == BASE.RUN) {
					LIVE.api_kidRun(dist);
					LIVE.api_setActionLock(kid, now + LIVE.api_getScenePtr()->getKidRunTime(dist));
				}
			}
		}
	}
	return JudgeReturn::judgeEnd;
}

LiveCode ToolUI::start() {
	LiveCode paper = PAINT.nodeNew();
	toolPaint(paper);
	return paper;
}

void ToolUI::toolPaint(LiveCode paper) {
	if (paper) {
		PAINT.nodeRemoveAllChildren(paper);
		PxPos sceneSize = LIVE.api_getSceneSize();
		PxPos hudSize = PxPos(80, 80);
		if (sceneSize > hudSize) {
			LiveCode back = PAINT.objAddToObj(paper, "ToolUIBack.csb", PxPos::zero);
			GameLiveHuman* human;
			if ((human = LIVE.api_kidHumanGet()) != nullptr) {
				auto tool = human->toolGet();
				if (tool > toolStart && tool < toolEnd) {
					string csbfile = BASE.getStuffCSB(tool);
					PAINT.objAddToObj(back, csbfile, PxPos::zero);
				}
			}
			paper->setPosition(sceneSize.x - 80, 0);
		}
	}
}

JudgeReturn ToolUI::action(LiveCode node, float* keyarray) {
	if (LIVE.api_hasScene()) {
		GameCommand gcmd = this->control()->translate(keyarray);
		if (gcmd == useTool) {
			// 动作锁的检测
			float now = PAINT.clock();
			LiveObjPtr kid = LIVE.api_kidGet();
			if (kid != nullptr) {
				if (LIVE.api_getActionLock(kid) > now)
					return JudgeReturn::judgeEnd;
				else
					LIVE.api_setActionLock(kid, now + BASE.USE_TOOL_TIME);
				auto cmdtemp = LIVE.api_kidHumanGet()->toolUse();
				if (cmdtemp != emptyCmd)
					LIVE.api_setCommandCache(cmdtemp);
				return judgeNextLayer;
			}
		}
		else if (gcmd == switchTool) {
			// 动作锁的检测
			float now = PAINT.clock();
			LiveObjPtr kid = LIVE.api_kidGet();
			if (kid != nullptr) {
				if (LIVE.api_getActionLock(kid) > now)
					return JudgeReturn::judgeEnd;
				else
					LIVE.api_setActionLock(kid, now + BASE.USE_TOOL_TIME);

				int toolusing = LIVE.api_kidHumanGet()->toolGet();
				if (toolusing + 1 == toolEnd)
					LIVE.api_kidHumanGet()->toolSet(toolStart);
				else
					LIVE.api_kidHumanGet()->toolSet(toolusing + 1);
				toolPaint(node);
			}
			return judgeEnd;
		}
		else
			return judgeNextObject;
	}
	return judgeNextObject;
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


vector<GameKeyPress> vgkp7 = { buttonUp, buttonLeft };
vector<GameKeyPress> vgkp9 = { buttonUp, buttonRight };
vector<GameKeyPress> vgkp3 = { buttonDown, buttonRight };
vector<GameKeyPress> vgkp1 = { buttonDown, buttonLeft };
vector<GameKeyPress> vgkpr7 = { buttonUp, buttonLeft, buttonSpace };
vector<GameKeyPress> vgkpr9 = { buttonUp, buttonRight, buttonSpace };
vector<GameKeyPress> vgkpr3 = { buttonDown, buttonRight, buttonSpace };
vector<GameKeyPress> vgkpr1 = { buttonDown, buttonLeft, buttonSpace };
vector<GameKeyPress> vgkpr8 = { buttonUp, buttonSpace };
vector<GameKeyPress> vgkpr6 = { buttonRight, buttonSpace };
vector<GameKeyPress> vgkpr2 = { buttonDown, buttonSpace };
vector<GameKeyPress> vgkpr4 = { buttonLeft, buttonSpace };

GameCommand BasicMoveTranslator::translate(float* keyarray) {
	// 这里的锁什么的交给运动UI部分自己去想--已完成
	if (LIVE.keyPushedOnly(keyarray, buttonUp))
		return walkEight;
	else if (LIVE.keyPushedOnly(keyarray, buttonDown))
		return walkTwo;
	else if (LIVE.keyPushedOnly(keyarray, buttonLeft))
		return walkFour;
	else if (LIVE.keyPushedOnly(keyarray, buttonRight))
		return walkSix;

	if (LIVE.keyPushedOnly(keyarray, vgkp7))
		return walkSeven;
	if (LIVE.keyPushedOnly(keyarray, vgkp9))
		return walkNine;
	if (LIVE.keyPushedOnly(keyarray, vgkp3))
		return walkThree;
	if (LIVE.keyPushedOnly(keyarray, vgkp1))
		return walkOne;

	if (LIVE.keyPushedOnly(keyarray, vgkpr8))
		return runEight;
	if (LIVE.keyPushedOnly(keyarray, vgkpr2))
		return runTwo;
	if (LIVE.keyPushedOnly(keyarray, vgkpr4))
		return runFour;
	if (LIVE.keyPushedOnly(keyarray, vgkpr6))
		return runSix;

	if (LIVE.keyPushedOnly(keyarray, vgkpr9))
		return runNine;
	if (LIVE.keyPushedOnly(keyarray, vgkpr3))
		return runThree;
	if (LIVE.keyPushedOnly(keyarray, vgkpr1))
		return runOne;
	if (LIVE.keyPushedOnly(keyarray, vgkpr7))
		return runSeven;

	return GameCommand::emptyCmd;
}

GameCommand BasicObjectTranslator::translate(float* keyarray) {
	// 我迟早得写一个keyNotPush的判定啊
	if(LIVE.keyPushed(keyarray, GameKeyPress::buttonA))
		return GameCommand::pickNLook;
	if (LIVE.keyPushed(keyarray, GameKeyPress::buttonB))
		return GameCommand::useTool;
	if (LIVE.keyPushed(keyarray, GameKeyPress::buttonC))
		return GameCommand::switchTool;
	//TODO

	return GameCommand::emptyCmd;
}

GameCommand HandTranslator::translate(float* keyarray) {
	if (LIVE.keyPushedOnly(keyarray, GameKeyPress::buttonA))
		return GameCommand::drop;

	//TODO
	return GameCommand::emptyCmd;
}

LinkerReturn SoilLinker::link(GameCommand gcmd) {
	LinkerReturn result;
	result.eve = nullptr;
	result.judge = judgeNextObject;
	auto objCode = LIVE.api_getObjectBaseCodeJudgedNow();
	if (gcmd == useHoe && objCode == soilOriginCode) {
		auto sche = [](float dt) {
			LIVE.api_replaceObject(LIVE.api_getObjectPtrJudgedNow(), BASE.getStuff(soilHoedCode));
		};
		LIVE.api_delayTime(sche, BASE.USE_TOOL_TIME, "soilHoe" + std::to_string((int)objCode));
		result.judge = judgeEnd;
	}
	else if (gcmd == useWaterCan && objCode == soilHoedCode) {
		auto sche = [](float dt) {
			LIVE.api_replaceObject(LIVE.api_getObjectPtrJudgedNow(), BASE.getStuff(soilWateredCode));
		};
		LIVE.api_delayTime(sche, BASE.USE_TOOL_TIME, "soilWater" + std::to_string((int)objCode));
		result.judge = judgeEnd;
	}
	else if (gcmd == usePotatoSeed && (objCode == soilHoedCode || objCode == soilWateredCode)
		&& !LIVE.api_hasPlant(LIVE.api_getSceneCode(), LIVE.api_getObjectPtrJudgedNow()->MP())) {
		auto sche = [](float dt) {
			LIVE.api_addObject(stuffPotatoSeed, LIVE.api_getObjectPtrJudgedNow()->MP());
			LIVE.api_toolLose(kidHumanCode);
		};
		LIVE.api_delayTime(sche, BASE.USE_TOOL_TIME, "soilWater" + std::to_string((int)objCode));
		result.judge = judgeEnd;
	}
	//TODO 别的工具未完待续
	return result;
}

bool StartGameEvent::start(LiveObjPtr obj) {
	LIVE.api_sceneICD(farmSceneCode, BlockPos(400, 400), BlockPos(PxPos(960, 640)));
	LIVE.api_kidSet(kidNormalCode, BigBlockPos(3, 1), true);
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

	LIVE.api_UIStop(startPageCode);
	LIVE.api_UIStart(kidMoveUICode);
	LIVE.api_UIStart(toolUICode);
	return true;
}
