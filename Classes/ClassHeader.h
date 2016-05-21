#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include "cocos2d.h"

using std::vector;
using std::string;
using std::map;

enum MoveType { linear, };

enum WalkType {
    allWalk, noneWalk, alphaWalk
};

enum Walkable {
    nullWalk, canWalk, noWalk, jump, slide
};

enum JudgeReturn {
    judgeEnd, judgeNextObject, judgeNextLayer, judgePreviousObject, judgeResetLayer, judgeResetAll
};

enum KeyName {
    holdable, dropable
};

enum GameKeyPress {
    buttonUp,
    buttonDown,
    buttonLeft,
    buttonRight,
    buttonA,
    buttonB,
    buttonStart,
    buttonSpace,
};

enum GameCommand {
    emptyCmd,
    walkOne,
    walkTwo,
    walkThree,
    walkFour,
    walkSix,
    walkSeven,
    walkEight,
    walkNine,
    runOne,
    runTwo,
    runThree,
    runFour,
    runSix,
    runSeven,
    runEight,
    runNine,

    pick,
    drop,
    useTool,
    eat,

    talkNext,
    talkBacklog,
    selectUp,
    selectDown,
    selectLeft,
    selectRight,
    confirm,
    cancel,
};

enum StuffCode {
    farmPicCode,
    KidCode,
    soilCombCode,
    soilOriginCode,
    soilHoedCode,
    soilWateredCode,
};

enum SceneCode {
    farmSceneCode
};

enum TransCode {
    basicMenuTranslator,
    basicSceneTranslator,
};

enum UICode {
    startPageCode,
};


typedef int BlockType;
typedef float PxType;
typedef float NumType;
typedef int BaseCode;
typedef cocos2d::Node* LiveCode;

class GameLive;
class GameBase;
class GamePrincipal;
class GameCalculate;
class GamePaint;
class GameUI;

// 每个大格子占用的像素大小
const PxType BIG_BLOCK_PX = 50;
// 每个小格子占用的像素大小
const PxType SMALL_BLOCK_PX = 10;
const BlockType BIG_TO_SMALL = BIG_BLOCK_PX / SMALL_BLOCK_PX;

// Well, 根据Cocos2dx的模式，这个x方向从左至右，y方向从下至上

struct PxPos {
public:
    PxType x = 0;
    PxType y = 0;

    PxPos() {
    }

    PxPos(PxType x, PxType y) : x(x), y(y) {
    }
    const static PxPos zero;

    PxPos operator+() const {
        return PxPos(*this);
    }

    PxPos operator-() const {
        return PxPos(-this->x, -this->y);
    }

    PxPos operator+(const PxPos& rhs) const {
        return PxPos(this->x + rhs.x, this->y + rhs.y);
    }

    PxPos operator-(const PxPos& rhs) const {
        return PxPos(this->x - rhs.x, this->y - rhs.y);
    }

    PxPos operator*(const int mul) const {
        return PxPos(this->x * mul, this->y * mul);
    }

    PxPos operator+=(const PxPos& rhs) {
        this->x += rhs.x;
        this->y += rhs.y;
        return *this;
    }

    PxPos operator-=(const PxPos& rhs) {
        this->x -= rhs.x;
        this->y -= rhs.y;
        return *this;
    }

    bool operator==(const PxPos& rhs) const {
        if (this->x == rhs.x && this->y == rhs.y)
            return true;
        else
            return false;
    }

    bool operator!=(const PxPos& rhs) const {
        if (this->x != rhs.x || this->y != rhs.y)
            return true;
        else
            return false;
	}

	cocos2d::Vec2 toVec2() const {
		return cocos2d::Vec2(x, y);
	}
};

// 代表运算的基本单元，也就是小格子的类型

struct BlockPos {
public:

    enum Direction {
        empty, one, two, three, four, five, six, seven, eight, nine
    };
    BlockType x = 0;
    BlockType y = 0;
    const static BlockPos zero;

    BlockPos() {
    }

    BlockPos(BlockType x, BlockType y) {
        this->x = x;
        this->y = y;
    }

    BlockPos(const PxPos& px) {
        this->x = (BlockType) (px.x / SMALL_BLOCK_PX);
        this->y = (BlockType) (px.y / SMALL_BLOCK_PX);
    }

    void move(Direction dir);
    void moveBack(Direction dir);
    static BlockPos dirToBlock(Direction dir);

    operator PxPos() {
        return PxPos(this->x * SMALL_BLOCK_PX, this->y * SMALL_BLOCK_PX);
    }

    BlockPos operator+() const {
        return BlockPos(*this);
    }

    BlockPos operator-() const {
        return BlockPos(-this->x, -this->y);
    }

    BlockPos operator+(const BlockPos& rhs) const {
        return BlockPos(this->x + rhs.x, this->y + rhs.y);
    }

    BlockPos operator-(const BlockPos& rhs) const {
        return BlockPos(this->x - rhs.x, this->y - rhs.y);
    }

    BlockPos operator+=(const BlockPos& rhs) {
        this->x += rhs.x;
        this->y += rhs.y;
        return *this;
    }

    BlockPos operator-=(const BlockPos& rhs) {
        this->x -= rhs.x;
        this->y -= rhs.y;
        return *this;
    }

    bool operator==(const BlockPos& rhs) const {
        if (this->x == rhs.x && this->y == rhs.y)
            return true;
        else
            return false;
    }

    bool operator!=(const BlockPos& rhs) const {
        if (this->x != rhs.x || this->y != rhs.y)
            return true;
        else
            return false;
    }
	
	static float time(const BlockPos& left, const BlockPos& right, float speedInBlocksPerSecond) {
		float xt = left.x - right.x;
		float yt = left.y - right.y;
		return std::sqrt((xt * xt) + (yt * yt)) / speedInBlocksPerSecond;
	}
	
	static float time(const BlockPos& distance, float speedInBlocksPerSecond) {
		float xt = distance.x;
		float yt = distance.y;
		return std::sqrt((xt * xt) + (yt * yt)) / speedInBlocksPerSecond;
	}
};

struct BigBlockPos {
public:
    BlockType x = 0;
    BlockType y = 0;

    BigBlockPos() {
    }

    BigBlockPos(BlockType x, BlockType y) : x(x), y(y) {
    }

    BigBlockPos(BlockPos pos) {
        this->x = pos.x / BIG_TO_SMALL;
        this->y = pos.y / BIG_TO_SMALL;
    }

    operator BlockPos() {
        return BlockPos(x * BIG_TO_SMALL, y * BIG_TO_SMALL);
    }
};

class GameAlpha;
class GameObject;
class GameLinker;
class GameEvent;
class GameTranslator;

class GameLiveObject;
class GameLiveScene;
class GameLiveUI;

typedef std::shared_ptr<GameObject> ObjPtr;
typedef std::weak_ptr<GameObject> ObjWeak;
typedef std::shared_ptr<GameTranslator> TransPtr;
typedef std::shared_ptr<GameUI> UIPtr;
typedef std::shared_ptr<GameEvent> EventPtr;
typedef std::shared_ptr<GameLinker> LinkerPtr;
typedef std::shared_ptr<map<KeyName, NumType>> ValuePtr;

typedef std::shared_ptr<GameLiveObject> LiveObjPtr;
typedef std::weak_ptr<GameLiveObject> LiveObjWeak;
typedef std::shared_ptr<GameLiveUI> LiveUIPtr;

typedef std::vector<LiveObjPtr> LiveDot;

struct LinkerReturn {
    EventPtr eve = nullptr;
    JudgeReturn judge = judgeEnd;
};

template<class Item>
inline Item* get(Item* array, BlockPos size, BlockPos pos) {
    if (pos.x >= 0 && pos.y >= 0 && pos.x < size.x && pos.y < size.y)
        return array + pos.x * size.y + pos.y;
    else
        return nullptr;
}
