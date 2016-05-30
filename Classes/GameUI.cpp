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
JudgeReturn GameUI::action(__attribute__((unused)) LiveCode node, __attribute__((unused)) float* keyarray, __attribute__((unused)) GameObjectJudge& judge) {
#endif
#ifdef WIN32
JudgeReturn GameUI::action(LiveCode node, float* keyarray, GameObjectJudge& judge) {
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

JudgeReturn StartPageUI::action(LiveCode node, float* keyarray, GameObjectJudge& judge) {
    GameCommand gcmd = this->control()->translate(keyarray);
	dynamic_cast<Hello*>(node)->comeOn(node,gcmd);
    return JudgeReturn::judgeEnd;
}

LiveCode KidMoveUI::start() {
	LiveCode result = cocos2d::Node::create();
	return result;
}

JudgeReturn KidMoveUI::action(LiveCode node, float* keyarray, GameObjectJudge& judge) {
	if (LIVE.api_hasScene()) {
		GameCommand gcmd = this->control()->translate(keyarray);
		if (gcmd == GameCommand::emptyCmd)
			return JudgeReturn::judgeNextObject;
		auto iswalk = BASE.cmdWalkOrRun(gcmd);
		if (iswalk == BASE.WALK)
			LIVE.api_kidWalkStep(BlockPos::cmdToDir(gcmd));
		else if (iswalk == BASE.RUN)
			LIVE.api_kidRunStep(BlockPos::cmdToDir(gcmd));
		else
			return judgeEnd;
		/*float now = PAINT.clock();
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
		}*/
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
			if ((human = LIVE.api_getKidHuman()) != nullptr) {
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

JudgeReturn ToolUI::action(LiveCode node, float* keyarray, GameObjectJudge& judge) {
	if (LIVE.api_hasScene()) {
		GameCommand gcmd = this->control()->translate(keyarray);
		if (gcmd == useTool) {
			LiveObjPtr kid = LIVE.api_kidGet();
			if (kid == nullptr)
				return judgeEnd;
			CocoFunc sch = [&judge](float dt) {
				auto cmdtemp = LIVE.api_getKidHuman()->toolUse();
				if (cmdtemp != emptyCmd)
					judge.setCmdCache(cmdtemp);
			};
			if (LIVE.api_autoActionLock(kid, BASE.USE_TOOL_TIME, doNothing, sch, "useTool"))
				return judgeNextLayer;
			else
				return judgeEnd;
		}
		else if (gcmd == switchTool) {
			LiveObjPtr kid = LIVE.api_kidGet();
			if (kid == nullptr)
				return judgeEnd;
			CocoFunc sch = [this, node](float dt) {
				int toolusing = LIVE.api_getKidHuman()->toolGet();
				if (toolusing + 1 == toolEnd)
					LIVE.api_getKidHuman()->toolSet(toolStart);
				else
					LIVE.api_getKidHuman()->toolSet(toolusing + 1);
				toolPaint(node);
			}; 
			LIVE.api_autoActionLock(kid, BASE.USE_TOOL_TIME, doNothing, sch, "switchTool");
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

LinkerReturn SoilLinker::link(GameCommand gcmd, GameObjectJudge& judge) {
	LinkerReturn result;
	result.eve = nullptr;
	result.judge = judgeNextObject;
	if (gcmd == useTool) {
		auto humancode = judge.getHumanCode();
		auto human = LIVE.api_getHuman(humancode);
		BaseCode tool;
		if (human != nullptr)
			tool = human->toolGet();
		else
			return result;
		auto objCode = judge.getObjectBaseCodeJudgedNow();
		auto objPtr = judge.getObjectPtrJudgedNow();
		if (tool == toolHoe && objCode == soilOriginCode) {
			CocoFunc sche = [objPtr](float dt) {
				LIVE.api_replaceObject(objPtr, BASE.getStuff(soilHoedCode));
			};
			LIVE.api_delayTime(sche, BASE.USE_TOOL_TIME, "soilHoe" + std::to_string((int)objCode), &*objPtr);
			result.judge = judgeEnd;
		}
		else if (tool == toolWaterCan && objCode == soilHoedCode) {
			CocoFunc sche = [objPtr](float dt) {
				LIVE.api_replaceObject(objPtr, BASE.getStuff(soilWateredCode));
			};
			LIVE.api_delayTime(sche, BASE.USE_TOOL_TIME, "soilWater" + std::to_string((int)objCode), &*objPtr);
			result.judge = judgeEnd;
		}
		else if (tool == toolPotatoSeed && (objCode == soilHoedCode || objCode == soilWateredCode)
			&& !LIVE.api_hasPlant(LIVE.api_getSceneCode(), objPtr->MP())) {
			CocoFunc sche = [objPtr, humancode](float dt) {
				LIVE.api_addObject(stuffPotatoSeed, objPtr->MP());
				LIVE.api_toolLose(humancode);
			};
			LIVE.api_delayTime(sche, BASE.USE_TOOL_TIME, "soilWater" + std::to_string((int)objCode), &*objPtr);
			result.judge = judgeEnd;
		}
	}
	//TODO 别的工具未完待续
	return result;
}

LinkerReturn BedLinker::link(GameCommand gcmd, GameObjectJudge& judge) {
	LinkerReturn result;
	result.eve = nullptr;
	result.judge = judgeNextObject;
	if (gcmd == GameCommand::pickNLook) {
		result.eve = BASE.getEvent(dayPassEventCode);
		result.judge = judgeEnd;
	}
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
	LIVE.api_allDimFrom();
	return true;
}

bool DayPassEvent::start(LiveObjPtr obj) {
	//TODO messageBox
	LIVE.api_dayPass();
	return true;
}