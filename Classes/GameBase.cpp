#include "GamePrincipal.h"
#include "GameBase.h"
#include "GameUI.h"

const PxPos PxPos::zero = PxPos(0, 0);
const BlockPos BlockPos::zero = BlockPos(0, 0);
GameObject GameObject::origin;

void GameBase::init() {
	// Object Create
	GameObject::create(GameObject::ground, farmPicCode, "farmBackground", "", BlockPos(PxPos(960, 640)), WalkType::allWalk, &stuffData, "FarmBackground.csb");
	ObjPtr soilComb = GameObject::create(GameObject::combStatue, soilCombCode, "soilComb", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData);
	GameObject::create(GameObject::ground, soilOriginCode, "soilOrigin", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "SoilOrigin.csb", soilComb);
	GameObject::create(GameObject::ground, soilHoedCode, "soilHoed", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "SoilHoed.csb", soilComb);
	GameObject::create(GameObject::ground, soilWateredCode, "soilWatered", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "SoilWatered.csb", soilComb);

	// Scene Create

	// Control Create
	

	// UI Create
	// GameUI::create(StartPageCode, "startPage", BlockPos::zero, &UIData, "StartPage.csb");
}

BlockPos BlockPos::dirToBlock(Direction dir) {
	if (dir == Direction::one)
		return BlockPos(-1, -1);
	else if (dir == Direction::two)
		return BlockPos(0, -1);
	else if (dir == Direction::three)
		return BlockPos(1, -1);
	else if (dir == Direction::four)
		return BlockPos(-1, 0);
	else if (dir == Direction::five)
		return BlockPos::zero;
	else if (dir == Direction::six)
		return BlockPos(1, 0);
	else if (dir == Direction::seven)
		return BlockPos(-1, 1);
	else if (dir == Direction::eight)
		return BlockPos(0, 1);
	else if (dir == Direction::nine)
		return BlockPos(1, 1);
	else
		return BlockPos::zero;
}

void BlockPos::move(Direction dir) {
	*this += dirToBlock(dir);
}

void BlockPos::moveBack(Direction dir) {
	*this -= dirToBlock(dir);
}

GameJudgeResult GameObject::link(GameBasicCommand gcmd, EventPtr& out_event) {
	if (this->linker() != nullptr) {
		auto sec = this->linker()->link(gcmd);
		out_event = sec.eve;
		return sec.judge;
	}
	else {
		out_event = nullptr;
		return this->_defaultJudge;
	}
}

ObjPtr GameBase::getStuff(CodeType code) {
	if (code >= 0 && code < (int)stuffData.size())
		return stuffData[code];
	else
		return nullptr;
}

ObjPtr GameBase::getScene(CodeType code) {
	if (code >= 0 && code < (int)sceneData.size())
		return sceneData[code];
	else
		return nullptr;
}

TransPtr GameBase::getControl(CodeType code) {
	if (code >= 0 && code < (int)controlData.size())
		return controlData[code];
	else
		return nullptr;
}

UIPtr GameBase::getUI(CodeType code) {
	if (code >= 0 && code < (int)UIData.size())
		return UIData[code];
	else
		return nullptr;
}

bool GameEvent::start(LiveObjPtr obj) {
	return true;
}

LinkerSecond GameLinker::link(GameBasicCommand gcmd) {
	return convert(gcmd);
}

GameBasicCommand GameTranslator::translate(bool* arrOfKeys) {
	return convert(arrOfKeys);
}

