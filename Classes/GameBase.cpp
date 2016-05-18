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

    // Translator Create
    GameTranslator::create<BasicMenuTranslator>(basicMenuTranslator, &transData);
    
    // Linker Create
    
    // UI Create
    GameUI::create<StartPageUI>(startPageCode, "startPage", basicMenuTranslator,  &UIData);
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

JudgeReturn GameObject::link(GameCommand gcmd, EventPtr& out_event) {
    if (this->linker() != nullptr) {
        auto sec = this->linker()->link(gcmd);
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
LiveCode GameObject::customPaint(LiveCode father, const BlockPos& pos) {
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

ObjPtr GameBase::getStuff(CodeType code) {
    if (code >= 0 && code < (int) stuffData.size())
        return stuffData[code];
    else
        return nullptr;
}

ObjPtr GameBase::getScene(CodeType code) {
    if (code >= 0 && code < (int) sceneData.size())
        return sceneData[code];
    else
        return nullptr;
}

TransPtr GameBase::getTranslator(CodeType code) {
    if (code >= 0 && code < (int) transData.size())
        return transData[code];
    else
        return nullptr;
}

UIPtr GameBase::getUI(CodeType code) {
    if (code >= 0 && code < (int) UIData.size())
        return UIData[code];
    else
        return nullptr;
}

bool GameEvent::start(LiveObjPtr obj) {
    return true;
}

TransPtr GameTranslator::create(CodeType code, std::function<GameCommand(bool*)> &method, vector<TransPtr> *container) {
    TransPtr pt(new GameTranslator(method));
    T_push(container, pt, code);
    return pt;
}

LinkerPtr GameLinker::create(CodeType code, std::function<LinkerReturn(GameCommand)> &method, vector<LinkerPtr> *container) {
    LinkerPtr pt(new GameLinker(method));
    T_push(container, pt, code);
    return pt;
}

LinkerReturn GameLinker::link(GameCommand gcmd) {
    return convert(gcmd);
}

GameCommand GameTranslator::translate(bool* arrOfKeys) {
    return convert(arrOfKeys);
}

