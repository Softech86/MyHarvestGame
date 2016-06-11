#include "GameUI.h"
#include "GameBase.h"
#include "GameLive.h"
#include "GamePrincipal.h"
#include "HelloScene.h"
#include "TalkNode.h"
#include "SelectNode.h"
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

void GameUI::stop(LiveCode node) {

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
			LIVE.api_kidMoveStep(BlockPos::cmdToDir(gcmd), true, true);
		else if (iswalk == BASE.RUN)
			LIVE.api_kidMoveStep(BlockPos::cmdToDir(gcmd), true, false);
		else
			return judgeEnd;
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
				return judgeNextObject;
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
			LIVE.api_autoActionLock(kid, BASE.SWITCH_TOOL_TIME, doNothing, sch, "switchTool");
			return judgeEnd;
		}
		else if (gcmd == pickNLook) {
			LiveObjPtr kid = LIVE.api_kidGet();
			if (kid == nullptr)
				return judgeNextObject;
			if (LIVE.api_humanDrop(kidHumanCode) == GameLive::done)
				return judgeEnd;
			else
				return judgeNextObject;
		}
		else
			return judgeNextObject;
	}
	return judgeNextObject;
}

JudgeReturn HandUI::action(LiveCode node, float* keyarray, GameObjectJudge& judge) {
	if (LIVE.api_hasScene()) {
		GameCommand gcmd = this->control()->translate(keyarray);
		if (gcmd == putIntoPack) {
			LiveObjPtr kid = LIVE.api_kidGet();
			if (kid == nullptr)
				return judgeNextObject;
			if (LIVE.api_humanPutIntoPack(kidHumanCode) != GameLive::ActionResult::cannotDo)
				return judgeEnd;
			else
				return judgeNextObject;
		}
	}
	return judgeNextObject;
}

JudgeReturn MenuUI::action(LiveCode node, float* keyarray, GameObjectJudge& judge) {
	return judgeNextObject;
	//TODO
}

// 像这种Cycle和Just都是有种算时间差的感觉的，就是你只要一组按钮不是同一个时间差的话，是不会同时判定的吧
GameCommand BasicMenuTranslator::translate(float* keyarray) {
	if (LIVE.keyCyclePushed(keyarray, GameKeyPress::buttonUp, KEY_CYCLE_SEC, true))
		return selectUp;
	else if (LIVE.keyCyclePushed(keyarray, GameKeyPress::buttonDown, KEY_CYCLE_SEC, true))
		return selectDown;
	else if (LIVE.keyCyclePushed(keyarray, GameKeyPress::buttonLeft, KEY_CYCLE_SEC, true))
		return selectLeft;
	else if (LIVE.keyCyclePushed(keyarray, GameKeyPress::buttonRight, KEY_CYCLE_SEC, true))
		return selectRight;
	else if (LIVE.keyJustPushed(keyarray, GameKeyPress::buttonA, true))
		return confirm;
	else if (LIVE.keyJustPushed(keyarray, GameKeyPress::buttonB, true))
		return cancel;
	else if (LIVE.keyJustPushed(keyarray, GameKeyPress::buttonSpace, true))
		return menu;
	else if (LIVE.keyJustPushed(keyarray, GameKeyPress::buttonStart, true))
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
	if(LIVE.keyJustPushed(keyarray, GameKeyPress::buttonA, false))
		return GameCommand::pickNLook;
	//TODO

	return GameCommand::emptyCmd;
}

GameCommand ToolTranslator::translate(float* keyarray) {
	if (LIVE.keyJustPushed(keyarray, GameKeyPress::buttonA, false))
		return GameCommand::pickNLook;
	if (LIVE.keyJustPushed(keyarray, GameKeyPress::buttonB, false))
		return GameCommand::useTool;
	if (LIVE.keyPushed(keyarray, GameKeyPress::buttonC, false))
		return GameCommand::switchTool;
	return GameCommand::emptyCmd;
}

GameCommand TalkTranslator::translate(float* keyarray) {
	if (LIVE.keyCyclePushed(keyarray, GameKeyPress::buttonA, 0.5f, true))
		return GameCommand::talkNext;
	if (LIVE.keyCyclePushed(keyarray, GameKeyPress::buttonDown, 0.5f, true))
		return GameCommand::talkNext;
	if (LIVE.keyCyclePushed(keyarray, GameKeyPress::buttonUp, 0.5f, true))
		return GameCommand::talkBacklog;
	return GameCommand::emptyCmd;
}


GameCommand HandTranslator::translate(float* keyarray) {
	if (LIVE.keyJustPushed(keyarray, GameKeyPress::buttonA, true))
		return GameCommand::drop;
	if (LIVE.keyJustPushed(keyarray, GameKeyPress::buttonB, true))
		return GameCommand::putIntoPack;
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
			LIVE.api_autoAnimeLock(judge.getHumanPtr(), BASE.USE_TOOL_TIME, doNothing, sche, "Hoing soil");;
			result.judge = judgeEnd;
		}
		else if (tool == toolWaterCan && objCode == soilHoedCode) {
			CocoFunc sche = [objPtr](float dt) {
				LIVE.api_replaceObject(objPtr, BASE.getStuff(soilWateredCode));
			};
			LIVE.api_autoAnimeLock(judge.getHumanPtr(), BASE.USE_TOOL_TIME, doNothing, sche, "Watering soil");
			result.judge = judgeEnd;
		}
		else if (tool == toolPotatoSeed && (objCode == soilHoedCode || objCode == soilWateredCode)
			&& !LIVE.api_hasPlant(LIVE.api_getSceneCode(), objPtr->MP())) {
			CocoFunc sche = [objPtr, humancode](float dt) {
				if(LIVE.api_addObject(stuffPotatoSeed, objPtr->MP()) != nullptr)
					LIVE.api_toolLose(humancode);
			};
			LIVE.api_autoAnimeLock(judge.getHumanPtr(), BASE.USE_TOOL_TIME, doNothing, sche, "Planting potato");
			result.judge = judgeEnd;
		}
	}
	//TODO 别的工具未完待续
	return result;
}

LiveCode TalkUI::start(){
	if (talkContent != nullptr && parentLoader != nullptr)
		return TalkNode::create(talkContent->personName, talkContent->personCsb, talkContent->talkText);
	else
		return PAINT.nodeNew();
}

JudgeReturn TalkUI::action(LiveCode node, float* keyarray, GameObjectJudge& judge) {
	auto gcmd = this->control()->translate(keyarray);
	if (gcmd == talkNext) {
		TimeFunc sch = [node, this]() {
			auto* talk = dynamic_cast<TalkNode*>(node);
			bool cando = talk->nextLine();
			if (cando == false)
				this->parentLoader->next();
			return 0;
		};
		int temp;
		LIVE.api_delegateActionTimeCompare(judge.getHumanPtr(), 0.3f, sch, temp, "Talk Window Close");
	}
	return judgeEnd;
}

LiveCode ChooseUI::start() {
	if (content != nullptr && parentLoader != nullptr)
		return SelectNode::create(content->personName, content->description, content->choice, content->choiceCount); //TODO
	else
		return PAINT.nodeNew();
}

JudgeReturn ChooseUI::action(LiveCode node, float* keyarray, GameObjectJudge& judge) {
	auto gcmd = this->control()->translate(keyarray);
	auto* talk = dynamic_cast<SelectNode*>(node);
	int temp;
	if (gcmd == selectDown) {
		TimeFunc sch = [talk, this]() {
			talk->selectDown();
			return 0;
		};
		LIVE.api_delegateActionTimeCompare(judge.getHumanPtr(), 0.1f, sch, temp, "Select Down");
	}
	else if (gcmd == selectUp) {
		TimeFunc sch = [talk, this]() {
			talk->selectUp();
			return 0;
		};
		LIVE.api_delegateActionTimeCompare(judge.getHumanPtr(), 0.1f, sch, temp, "Select Up");
	}
	else if (gcmd == confirm) {
		TimeFunc sch = [talk, this]() {
			int selected = talk->getSelect();
			if (selected == -1)
				return 0;
			auto psto = this->content->getEffect(selected);
			if (psto == nullptr)
				this->parentLoader->next();
			else {
				LIVE.api_startStory(*psto);
				this->parentLoader->end();
			}
			return 0;
		};
		LIVE.api_delegateActionTimeCompare(judge.getHumanPtr(), 0.3f, sch, temp, "Select Confirm");
	}

	// TODO
	return judgeEnd;
}

LinkerReturn BedLinker::link(GameCommand gcmd, GameObjectJudge& judge) {
	LinkerReturn result;
	result.eve = nullptr;
	result.judge = judgeNextObject;
	auto humanptr = judge.getHumanPtr();
	if (gcmd == GameCommand::pickNLook && humanptr != nullptr) {
		BlockPos dist = judge.getObjectPtrJudgedNow()->MPC() - humanptr->MP();
		CocoFunc sch1 = [dist](float) {
			LIVE.api_humanWalk(kidHumanCode, dist);
		};
		if (LIVE.api_autoActionLock(humanptr, 0.3f, doNothing, sch1, "goBed1")) {
			CocoFunc sch2 = [humanptr](float) {
				humanptr->setFace(humanptr, BlockPos::two);
			};
			LIVE.api_autoActionLock(humanptr, 0, doAfter, sch2, "goBed2", 0);
			result.eve = BASE.getEvent(dayPassEventCode);
			result.judge = judgeEnd;
			result.delayTime = 0.5f;
		}
	}
	return result;
}

LinkerReturn PotatoLinker::link(GameCommand gcmd, GameObjectJudge& judge) {
	LinkerReturn result;
	if (gcmd == GameCommand::pickNLook) {
		auto live = judge.getObjectPtrJudgedNow();
		auto pota = LIVE.api_addObject(potatoCode, live->getMargin());
		auto actres = LIVE.api_kidPick(pota);
		if (actres == GameLive::ActionResult::done)
			LIVE.api_removeObject(live);
		else {
			LIVE.api_removeObject(pota);
		}
		result.judge = judgeEnd;
	}
	return result;
}

LinkerReturn DefaultLinker::link(GameCommand gcmd, GameObjectJudge& judge) {
	LinkerReturn result;
	result.eve = nullptr;
	result.judge = judgeNextObject;
	if (gcmd == pickNLook) {
		if (judge.getObjectPtrJudgedNow()->getObj()->isPickable())
			LIVE.api_kidPick(judge.getObjectPtrJudgedNow());
	}
	//TODO
	return result;
}

bool StartGameEvent::start(LiveObjPtr obj) {
	LIVE.api_UIStop(startPageCode);
	LIVE.api_sceneICD(farmSceneCode, BASE.SCENE_DISPLAY_SIZE);
	LIVE.api_kidSet(kidNormalStuffCode, BlockPos(PxPos(2290, 2342)), true);
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

	LIVE.api_UIStart(kidMoveUICode);
	LIVE.api_UIStart(toolUICode);
	LIVE.api_UIStart(handUICode);
	LIVE.api_allDimFrom();
	return true;
}

bool DayPassEvent::start(LiveObjPtr obj) {
	//TODO messageBox
	LIVE.api_dayPass();
	return true;
}