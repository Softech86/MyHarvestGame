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
	PAINT.nodeRemoveAllChildren(paper);
	PxPos sceneSize = LIVE.api_getSceneSize();
	PxPos hudSize = PxPos(80, 80);
	if (sceneSize > hudSize) {
		LiveCode back = PAINT.objAddToObj(paper, "ToolUIBack.csb", PxPos::zero);
		if (tool > StuffCode::toolStart && tool < StuffCode::toolEnd) {
			switch (tool)
			{
			case toolHoe:
				PAINT.objAddToObj(back, "Hoe.csb", PxPos::zero);
				break;
			case toolWaterCan:
				PAINT.objAddToObj(back, "WaterCan.csb", PxPos::zero);
				break;
			default:
				break;
			}
			//TODO 枚举
		}
		paper->setPosition(sceneSize.x - 80, 0);
	}
}

JudgeReturn ToolUI::action(LiveCode node, float* keyarray) {
	if (LIVE.api_hasScene()) {
		GameCommand gcmd = this->control()->translate(keyarray);
		if (gcmd == useTool && tool > toolStart && tool < toolEnd) {
			// 动作锁的检测
			float now = PAINT.clock();
			LiveObjPtr kid = LIVE.api_kidGet();
			if (kid != nullptr) {
				if (LIVE.api_getActionLock(kid) > now)
					return JudgeReturn::judgeEnd;
				else
					LIVE.api_setActionLock(kid, now + BASE.USE_TOOL_TIME);

				// 枚举各种工具
				if (tool == toolHoe)
					LIVE.api_setCommandCache(useHoe);
				else if (tool = toolWaterCan)
					LIVE.api_setCommandCache(useWaterCan);
				// TODO 未完的枚举

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

				if (tool + 1 == toolEnd)
					tool = toolStart;
				else
					tool++;
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
	if (gcmd == useHoe && LIVE.api_getObjectBaseCodeJudgedNow() == soilOriginCode) {
		auto sche = [](float dt) {
			LIVE.api_replaceObject(LIVE.api_getObjectPtrJudgedNow(), BASE.getStuff(soilHoedCode));
		};
		LIVE.api_delayTime(sche, BASE.USE_TOOL_TIME, "soilHoe" + std::to_string((int)LIVE.api_getObjectCodeJudgedNow()));
		result.judge = judgeEnd;
	}
	else if (gcmd == useWaterCan && LIVE.api_getObjectBaseCodeJudgedNow() == soilHoedCode) {
		auto sche = [](float dt) {
			LIVE.api_replaceObject(LIVE.api_getObjectPtrJudgedNow(), BASE.getStuff(soilWateredCode));
		};
		LIVE.api_delayTime(sche, BASE.USE_TOOL_TIME, "soilWater" + std::to_string((int)LIVE.api_getObjectCodeJudgedNow()));
		cocos2d::log(std::to_string(LIVE.api_getObjectBaseCodeJudgedNow()).c_str());
		result.judge = judgeEnd;
	}
	//TODO weiwancheg
	return result;
}

bool StartGameEvent::start(LiveObjPtr obj) {
	LIVE.api_sceneICD(farmSceneCode, BlockPos(400, 400), BlockPos(PxPos(960, 640)));
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

	LIVE.api_UIStop(startPageCode);
	LIVE.api_UIStart(kidMoveUICode);
	LIVE.api_UIStart(toolUICode);
	return true;
}
