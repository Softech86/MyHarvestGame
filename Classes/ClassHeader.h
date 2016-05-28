#pragma once
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <memory>
#include <functional>
#include "cocos2d.h"


#define SHCP shallowCopy
#define SHCP_BASE(type) virtual inline type* SHCP() { return new type(*this); }
// 所有的继承类给我把这句话加上
#define SHCP_OVERRIDE(base, child) virtual inline base* SHCP() override { return new child(*this); }

#define PRINCIPAL GamePrincipal
#define PAINT GamePrincipal::getPaint()
#define BASE GamePrincipal::getBase()
#define LIVE GamePrincipal::getLive()

const float KEY_CYCLE_SEC = 0.5;

using std::vector;
using std::string;
using std::map;
using std::set;

enum MoveType { linear, };

enum SeasonType { haru, natsu, aki, fuyu };

enum WalkType {
    allWalk, noneWalk, alphaWalk
};

enum Walkable {
    nullWalk, canWalk, noWalk, jump, slide
};

enum JudgeReturn {
    judgeEnd, judgeNextObject, judgeNextLayer, judgePreviousObject, judgeObjectLayer, judgeResetLayer, judgeResetAll
};

enum GameKeyPress {
	buttonEmpty,
	buttonUp,
	buttonDown,
	buttonLeft,
	buttonRight,
	buttonA,
	buttonB,
	buttonC,
	buttonU,
	buttonI,
	buttonO,
	buttonP,
	buttonN,
	buttonM,
	buttonStart,
	buttonSpace,
	buttonEnd,
};

enum GameCommand {
    emptyCmd,
	walkStart,
    walkOne,
    walkTwo,
    walkThree,
    walkFour,
    walkSix,
    walkSeven,
    walkEight,
    walkNine,
	walkEnd,
	runStart,
    runOne,
    runTwo,
    runThree,
    runFour,
    runSix,
    runSeven,
    runEight,
    runNine,
	runEnd,

    pickNLook,
    drop,
    useTool,
	switchTool,
    eat,

    talkNext,
    talkBacklog,
    selectUp,
    selectDown,
    selectLeft,
    selectRight,
    confirm,
    cancel,
	menu,
	detail,

	useHoe,
	useWaterCan,
	usePotatoSeed,

};

enum StuffCode {
    farmPicCode,
    kidNormalCode,
	
	toolStart,
	toolHoe,
	toolWaterCan,
	toolPotatoSeed,
	toolEnd,
	// 如果你要增加工具的话记得增加command枚举和BASE.toolToCmd函数

    soilCombCode,
    soilOriginCode,
    soilHoedCode,
    soilWateredCode,

	plantStuffStart,
	stuffPotatoWithered,
	stuffPotatoSeed, 
	stuffPotatoLittle,
	stuffPotatoBig, 
	stuffPotatoHarvest,
	plantStuffEnd,
};

enum PlantCode {
	plantStart,
	plantPotato,
	plantEnd,
	// 如果要增加植物的话记得增加BASE.stuffToPlant函数
};

enum AnimalCode {

};

enum HumanCode {
	humanStart,
	kidHumanCode,
	seedSellerCode,
	humanEnd,
};

enum SceneCode {
	sceneStart,
    farmSceneCode,
	sceneEnd,
};

enum TransCode {
    basicMenuTranslator,
	basicMoveTranslator,
	basicObjectTranslator,
};

enum LinkerCode {
	soilLinkerCode,
};

enum UICode {
    startPageCode,
	kidMoveUICode,
	toolUICode,
};

enum EventCode {
	startGameEventCode,
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
const PxType BIG_BLOCK_PX = 40;
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

    PxPos operator*(const float mul) const {
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

	PxPos XCut() {
		return PxPos(this->x, 0);
	}

	PxPos YCut() {
		return PxPos(this->y, 0);
	}
	
	PxPos flip() {
		return PxPos(this->y, this->x);
	}

    bool operator==(const PxPos& rhs) const {
        if (this->x == rhs.x && this->y == rhs.y)
            return true;
        else
            return false;
    }

	bool operator>(const PxPos& rhs) const {
		if (this->x > rhs.x && this->y > rhs.y)
			return true;
		else
			return false;
	}

	bool operator<(const PxPos& rhs) const {
		if (this->x < rhs.x && this->y < rhs.y)
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
	static float distance(const PxPos& left, const PxPos& right) {
		float xt = left.x - right.x;
		float yt = left.y - right.y;
		return std::sqrt((xt * xt) + (yt * yt));
	}
	static float time(const PxPos& left, const PxPos& right, float speedInPxPerSecond) {
		float xt = left.x - right.x;
		float yt = left.y - right.y;
		return std::sqrt((xt * xt) + (yt * yt)) / speedInPxPerSecond;
	}

	static float time(const PxPos& distance, float speedInPxPerSecond) {
		float xt = distance.x;
		float yt = distance.y;
		return std::sqrt((xt * xt) + (yt * yt)) / speedInPxPerSecond;
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
	static Direction cmdToDir(GameCommand cmd);
	static Direction degreeToDir(float degree);

    operator PxPos() const {
        return PxPos(this->x * SMALL_BLOCK_PX, this->y * SMALL_BLOCK_PX);
    }

	operator Direction() const;

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

	bool operator>(const BlockPos& rhs) const {
		if (this->x > rhs.x && this->y > rhs.y)
			return true;
		else
			return false;
	}

	bool operator<(const BlockPos& rhs) const {
		if (this->x < rhs.x && this->y < rhs.y)
			return true;
		else
			return false;
	}

	BlockPos operator * (const BlockPos& rhs) {
		return BlockPos(this->x * rhs.x, this->y * rhs.y);
	}

	BlockPos XCut() {
		return BlockPos(this->x, 0);
	}

	BlockPos YCut() {
		return BlockPos(0, this->y);
	}

	BlockPos flip() {
		return BlockPos(this->y, this->x);
	}

	static void directionAreaSplit(const BlockPos& start, const BlockPos& size, const Direction dir, const int split, BlockPos& out_start, BlockPos& out_size);

	float distance() const {
		float xt = this->x, yt = this->y;
		return std::sqrt((xt * xt) + (yt * yt));
	}

	static float distance(const BlockPos& left, const BlockPos& right) {
		float xt = left.x - right.x;
		float yt = left.y - right.y;
		return std::sqrt((xt * xt) + (yt * yt));
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
class GamePlant;
class GameAnimal;
class GameHuman;

class GameLiveObject;
class GameLiveScene;
class GameLiveUI;

typedef std::shared_ptr<GameObject> ObjPtr;
typedef std::weak_ptr<GameObject> ObjWeak;
typedef std::shared_ptr<GameTranslator> TransPtr;
typedef std::shared_ptr<GameUI> UIPtr;
typedef std::shared_ptr<GameEvent> EventPtr;
typedef std::shared_ptr<GameLinker> LinkerPtr;
typedef std::shared_ptr<GamePlant> PlantPtr;
typedef std::shared_ptr<GameHuman> HumanPtr;

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
