#include "GamePrincipal.h"
#include "GameBase.h"
#include "GameLive.h"
#include "GameUI.h"

const PxPos			PxPos::zero = PxPos(0, 0);
const BlockPos		BlockPos::zero = BlockPos(0, 0);
const GameObject	GameObject::origin;

const float			GameBase::KID_MOVE_SPEED_IN_BIG_BLOCKS = 6.0f;
const BlockType		GameBase::KID_STEP = 1;
const float			GameBase::KID_RUN_COMPARED_TO_WALK = 2.5f;

const int			GameBase::DETECT_SPLIT = 2;
const GameCommand	GameBase::DEFAULT_COMMAND = GameCommand::emptyCmd;

void GameBase::init() {
    // Translator Create
    GameTranslator::create<BasicMenuTranslator>(basicMenuTranslator, &transData);
	GameTranslator::create<BasicMoveTranslator>(basicMoveTranslator, &transData);
	GameTranslator::create<BasicObjectTranslator>(basicObjectTranslator, &transData);

    // Linker Create
	GameLinker::create<SoilLinker>(soilLinkerCode, &linkerData);

    // Object Create
    GameObject::create(GameObject::ground, farmPicCode, "farmBackground", "", BlockPos(PxPos(960, 640)), WalkType::allWalk, &stuffData, "FarmBackground.csb");
    ObjPtr soilComb = GameObject::create(GameObject::combStatue, soilCombCode, "soilComb", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData);
    GameObject::create(GameObject::ground, soilOriginCode, "soilOrigin", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "SoilOrigin.csb", soilComb);
    GameObject::create(GameObject::ground, soilHoedCode, "soilHoed", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "SoilHoed.csb", soilComb);
    GameObject::create(GameObject::ground, soilWateredCode, "soilWatered", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "SoilWatered.csb", soilComb);
	soilComb->setChildrenLinker(BASE.getLinker(soilLinkerCode));
	ObjPtr kidp = GameObject::create(GameObject::BigType::kid, KidCode, "kid", "", BigBlockPos(1, 1), WalkType::noneWalk, &stuffData, "Kid.csb");
	kidp->center() = BlockPos(2, 2);

    // Scene Create
	ObjPtr farmsc = GameObject::create(GameObject::BigType::background, farmSceneCode, "farmScene", "", BlockPos(200, 200), WalkType::allWalk, &sceneData, "Grass.csb", nullptr, BlockPos::zero, BlockPos::zero);
	for (int i = 2; i < 17; i++)
		for (int j = 2; j < 17; j++) {
			farmsc->children().push_back(getStuff(soilOriginCode));
			farmsc->childrenPos().push_back(BigBlockPos(i, j));
		}


    // UI Create
    GameUI::create<StartPageUI>(startPageCode, "startPage", basicMenuTranslator,  &UIData);
	GameUI::create<KidMoveUI>(kidMoveUICode, "kidMoveUI", basicMoveTranslator, &UIData);
	GameUI::create<ToolUI>(toolUICode, "toolUI", basicObjectTranslator, &UIData);

	// Event Create
	GameEvent::create<StartGameEvent>(startGameEventCode, &eventData);
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

BlockPos::Direction BlockPos::cmdToDir(GameCommand cmd) {
	switch (cmd)
	{
	case walkOne:
	case runOne:
		return one;
		break;
	case walkTwo:
	case runTwo:
	case selectDown:
		return two;
		break;
	case walkThree:
	case runThree:
		return three;
		break;
	case walkFour:
	case runFour:
	case selectLeft:
		return four;
		break;
	case walkSix:
	case runSix:
	case selectRight:
		return six;
		break;
	case walkSeven:
	case runSeven:
		return seven;
		break;
	case walkEight:
	case runEight:
	case selectUp:
		return eight;
		break;
	case walkNine:
	case runNine:
		return nine;
		break;
	default:
		return empty;
		break;
	}
}

void BlockPos::move(Direction dir) {
    *this += dirToBlock(dir);
}

void BlockPos::moveBack(Direction dir) {
    *this -= dirToBlock(dir);
}

BlockPos::Direction BlockPos::degreeToDir(float degree) {
	while (degree < 0)
		degree += 360;
	while (degree > 360)
		degree -= 360;
	if (degree > 337.5 || degree <= 22.5)
		return Direction::six;
	if (degree > 22.5 && degree <= 67.5)
		return Direction::nine;
	if (degree > 67.5 && degree <= 112.5)
		return Direction::eight;
	if (degree > 112.5 && degree <= 157.5)
		return Direction::seven;
	if (degree > 157.5 && degree <= 202.5)
		return Direction::four;
	if (degree > 202.5 && degree <= 247.5)
		return Direction::one;
	if (degree > 247.5 && degree <= 292.5)
		return Direction::two;
	if (degree > 292.5 && degree <= 337.5)
		return Direction::three;
	else
		return Direction::six;
}

BlockPos::operator BlockPos::Direction() const {
	if (*this == BlockPos::zero)
		return Direction::five;
	float angle = std::atan2(this->y, this->x) * 180 / std::_Pi;
	return degreeToDir(angle);
}

void BlockPos::directionAreaSplit(const BlockPos& start, const BlockPos& size, const Direction dir, const int split, BlockPos& out_start, BlockPos& out_size) {
	BlockPos area = size;
	BlockPos startRelative = BlockPos::zero;
	if (dir == BlockPos::Direction::empty) {
	}
	else if (dir == BlockPos::Direction::one) {
		area = BlockPos(area.x / split, area.y / split);
	}
	else if (dir == BlockPos::Direction::two) {
		area = BlockPos(area.x, area.y / split);
	}
	else if (dir == BlockPos::Direction::three) {
		area = BlockPos(area.x / split, area.y / split);
		startRelative = BlockPos(size.x - area.x, 0);
	}
	else if (dir == BlockPos::Direction::four) {
		area = BlockPos(area.x / split, area.y);
	}
	else if (dir == BlockPos::Direction::five) {
	}
	else if (dir == BlockPos::Direction::six) {
		area = BlockPos(area.x / split, area.y);
		startRelative = BlockPos(size.x - area.x, 0);
	}
	else if (dir == BlockPos::Direction::seven) {
		area = BlockPos(area.x / split, area.y / split);
		startRelative = BlockPos(0, size.y - area.y);
	}
	else if (dir == BlockPos::Direction::eight) {
		area = BlockPos(area.x, area.y / split);
		startRelative = BlockPos(0, size.y - area.y);
	}
	else if (dir == BlockPos::Direction::nine) {
		area = BlockPos(area.x / split, area.y / split);
		startRelative = BlockPos(size.x - area.x, size.y - area.y);
	}
	else {
	}
	out_start = start + startRelative;
	out_size = area;
}

GameCommand GameObject::translate(float* arrOfKeys) {
	if (isCustomTranslate()) {
		return this->_translator->translate(arrOfKeys);
	}
	else
		return GameCommand::emptyCmd;
}

JudgeReturn GameObject::link(GameCommand gcmd, EventPtr& out_event) {
    if (this->linker() != nullptr) {
        LinkerReturn sec = this->linker()->link(gcmd);
        out_event = sec.eve;
        return sec.judge;
    } else {
        out_event = nullptr;
        return this->_defaultJudge;
    }
}

#if defined(LINUX) || defined(__APPLE__) || defined(APPLE)
LiveCode GameObject::customPaint(__attribute__((unused)) LiveCode father, __attribute__((unused)) const BlockPos& pos) {
#endif
#ifdef WIN32
LiveCode GameObject::customPaint(LiveCode father, const BlockPos& pos, int dotOrder) {
	UNREFERENCED_PARAMETER(father);
	UNREFERENCED_PARAMETER(pos);
#endif
    return nullptr;
}

#if defined(LINUX) || defined(__APPLE__) || defined(APPLE)
void GameObject::afterPaint(__attribute__((unused)) LiveCode obj) {
#endif
#ifdef WIN32
void GameObject::afterPaint(LiveCode obj) {
	UNREFERENCED_PARAMETER(obj);
#endif
}

bool GameObject::onFaceChange(BlockPos::Direction oldface, BlockPos::Direction newface) {
	if (this->_noFacingDifference)
		return true;
	else {
		auto obj = LIVE.api_getObjectPtrJudgedNow();
		if (obj == nullptr)
			return true;
		LIVE.api_objectChangePicture(obj, this->facingPicture()[(int)newface]);
		return true;
	}
}

ObjPtr GameBase::getStuff(BaseCode code) {
    if (code >= 0 && code < (int) stuffData.size())
        return stuffData[code];
    else
        return nullptr;
}

string GameBase::getStuffCSB(BaseCode code) {
	ObjPtr ptr = getStuff(code);
	if (ptr)
		return ptr->picture();
	else
		return "";
}

ObjPtr GameBase::getScene(BaseCode code) {
    if (code >= 0 && code < (int) sceneData.size())
        return sceneData[code];
    else
        return nullptr;
}

TransPtr GameBase::getTranslator(BaseCode code) {
    if (code >= 0 && code < (int) transData.size())
        return transData[code];
    else
        return nullptr;
}

UIPtr GameBase::getUI(BaseCode code) {
    if (code >= 0 && code < (int) UIData.size())
        return UIData[code];
    else
        return nullptr;
}

EventPtr GameBase::getEvent(BaseCode code) {
	if (code >= 0 && code < (int)eventData.size())
		return eventData[code];
	else
		return nullptr;
}
 
LinkerPtr GameBase::getLinker(BaseCode code) {
	if (code >= 0 && code < (int)linkerData.size())
		return linkerData[code];
	else
		return nullptr;
}

bool GameEvent::start(LiveObjPtr obj) {
    return true;
}

TransPtr GameTranslator::create(BaseCode code, std::function<GameCommand(float*)> &method, vector<TransPtr> *container) {
    TransPtr pt(new GameTranslator(method));
    T_push(container, pt, code);
    return pt;
}

LinkerPtr GameLinker::create(BaseCode code, std::function<LinkerReturn(GameCommand)> &method, vector<LinkerPtr> *container) {
    LinkerPtr pt(new GameLinker(method));
    T_push(container, pt, code);
    return pt;
}

LinkerReturn GameLinker::link(GameCommand gcmd) {
    return convert(gcmd);
}

GameCommand GameTranslator::translate(float* arrOfKeys) {
    return convert(arrOfKeys);
}

int GameBase::cmdWalkOrRun(GameCommand cmd) {
	if (cmd > walkStart && cmd < walkEnd)
		return WALK;
	else if (cmd > runStart && cmd < runEnd)
		return RUN;
	else
		return OTHERCMD;
}