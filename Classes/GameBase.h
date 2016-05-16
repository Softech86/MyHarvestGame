#pragma once
#include "ClassHeader.h"
#include "cocostudio/CocoStudio.h"

// 每个大格子占用的像素大小
const PxType BIG_BLOCK_PX = 50;
// 每个小格子占用的像素大小
const PxType SMALL_BLOCK_PX = 10;
const PxType BIG_TO_SMALL = BIG_BLOCK_PX / SMALL_BLOCK_PX;

// Well, 根据Cocos2dx的模式，这个x方向从左至右，y方向从下至上
struct PxPos {
public:
	PxType x = 0;
	PxType y = 0;
	PxPos() {}
	PxPos(PxType x, PxType y) : x(x), y(y) {}
	const static PxPos zero;

	PxPos operator + () const {
		return PxPos(*this);
	}
	PxPos operator - () const {
		return PxPos(-this->x, -this->y);
	}
	PxPos operator + (const PxPos& rhs) const {
		return PxPos(this->x + rhs.x, this->y + rhs.y);
	}
	PxPos operator - (const PxPos& rhs) const {
		return PxPos(this->x - rhs.x, this->y - rhs.y);
	}
	PxPos operator * (const int mul) const {
		return PxPos(this->x * mul, this->y * mul);
	}
	PxPos operator += (const PxPos& rhs) {
		this->x += rhs.x;
		this->y += rhs.y;
		return *this;
	}
	PxPos operator -= (const PxPos& rhs) {
		this->x -= rhs.x;
		this->y -= rhs.y;
		return *this;
	}
	bool operator == (const PxPos& rhs) const {
		if (this->x == rhs.x && this->y == rhs.y)
			return true;
		else
			return false;
	}
	bool operator != (const PxPos& rhs) const {
		if (this->x != rhs.x || this->y != rhs.y)
			return true;
		else
			return false;
	}
};

// 代表运算的基本单元，也就是小格子的类型
struct BlockPos {
public:
	BlockType x = 0;
	BlockType y = 0;
	const static BlockPos zero;
	BlockPos() {}
	BlockPos(BlockType x, BlockType y) {
		this->x = x;
		this->y = y;
	}
	BlockPos(const PxPos& px) {
		this->x = (BlockType)(px.x / SMALL_BLOCK_PX);
		this->y = (BlockType)(px.y / SMALL_BLOCK_PX);
	}

	enum Direction { empty, one, two, three, four, five, six, seven, eight, nine };
	void move(Direction dir);
	void moveBack(Direction dir);
	static BlockPos dirToBlock(Direction dir);

	operator PxPos() {
		return PxPos(this->x * SMALL_BLOCK_PX, this->y * SMALL_BLOCK_PX);
	}
	BlockPos operator +() const {
		return BlockPos(*this);
	}
	BlockPos operator -() const {
		return BlockPos(-this->x, -this->y);
	}
	BlockPos operator + (const BlockPos& rhs) const {
		return BlockPos(this->x + rhs.x, this->y + rhs.y);
	}
	BlockPos operator - (const BlockPos& rhs) const {
		return BlockPos(this->x - rhs.x, this->y - rhs.y);
	}
	BlockPos operator += (const BlockPos& rhs) {
		this->x += rhs.x;
		this->y += rhs.y;
		return *this;
	}
	BlockPos operator -= (const BlockPos& rhs) {
		this->x -= rhs.x;
		this->y -= rhs.y; 
		return *this;
	}
	bool operator == (const BlockPos& rhs) const {
		if (this->x == rhs.x && this->y == rhs.y)
			return true;
		else
			return false;
	}
	bool operator != (const BlockPos& rhs) const {
		if (this->x != rhs.x || this->y != rhs.y)
			return true;
		else
			return false;
	}
};

struct BigBlockPos{
public:
	BlockType x;
	BlockType y;
	BigBlockPos() {}
	BigBlockPos(BlockType x, BlockType y) : x(x), y(y) {}
	BigBlockPos(BlockPos pos) {
		this->x = pos.x / BIG_TO_SMALL;
		this->y = pos.y / BIG_TO_SMALL;
	}

	operator BlockPos() {
		return BlockPos(x * BIG_TO_SMALL, y * BIG_TO_SMALL);
	}
};

template<class PtrType>
void T_Push(vector<PtrType>* container, PtrType father, PtrType pt) {
	if (container != nullptr && pt->code() >= 0) {
		if ((int)container->size() <= pt->code()) {
			container->resize(pt->code() + 1);
		}
		(*container)[pt->code()] = pt;
	}
	if (father != nullptr)
		father->children().push_back(pt);
}

enum WalkType { allWalk, noneWalk, alphaWalk };
enum Walkable { nullWalk, canWalk, noWalk, jump, slide};

class AlphaBMP {
public:

	BlockPos size;
	Walkable* moveMap = nullptr;
	AlphaBMP() {}
	AlphaBMP(BlockPos size) : size(size) {}
	AlphaBMP(BlockPos size, string BMPfile) {
		//TODO
	}

	Walkable getWalk(BlockPos pos) {
		auto pw =  get(this->moveMap, this->size, pos);
		if (pw == nullptr)
			return nullWalk;
		else
			return *pw;
	}
};

enum GameJudgeResult { judgeEnd, judgeNextObject, judgeNextLayer, judgePreviousObject, judgeResetLayer, judgeResetAll };
enum KeyName{ holdable, dropable };
struct LinkerSecond {
	EventPtr eve = nullptr;
	GameJudgeResult judge = judgeEnd;
};

class GameObject {
public:
	enum BigType { empty, npc, animal, plant, ground, stuff, combStuff, combStatue };
	struct JumpData {
		AlphaBMP jump;
		CodeType sceneCode = -1;
		BlockPos kidPos;
	};

	static GameObject origin;

private:
	BigType _type = BigType::empty;
	CodeType _code = -1;
	std::string _name;
	std::string _description;
	BlockPos _size;
	BlockPos _position;
	BlockPos _anchor;
	string _picture = "";
	vector<ObjWeak> _children;
	WalkType _walktype = WalkType::noneWalk;
	AlphaBMP _alphaWalkableBMP;
	vector<JumpData> _jumpInfo;

	GameJudgeResult _defaultJudge = GameJudgeResult::judgeEnd;
	TransPtr _translator = nullptr;
	LinkerPtr _link = nullptr;
	ValuePtr _value = nullptr;

public:
	GameObject() {}
	GameObject(
		BigType type,
		CodeType code,
		const string& name,
		const string& description,
		const BlockPos& size,
		WalkType walkable,
		const string& pic = "",
		const BlockPos& anchor = BlockPos::zero,
		const BlockPos& position = BlockPos::zero) :
		_type(type),
		_code(code),
		_name(name),
		_description(description),
		_size(size),
		_position(position),
		_picture(pic),
		_anchor(anchor),
		_walktype(walkable){}

	static ObjPtr create(
		BigType type,
		CodeType code,
		const string& name,
		const string& description,
		const BlockPos& size,
		WalkType walkable,
		vector<ObjPtr>* container = nullptr,
		const string& pic = "",
		ObjPtr father = nullptr,
		const BlockPos& anchor = BlockPos::zero,
		const BlockPos& position = BlockPos::zero) {
		ObjPtr pt(new GameObject(type, code, name, description, size, walkable, pic, anchor, position));
		T_Push(container, father, pt);
		return pt;
	}

	BigType& type() { return this->_type; }
	CodeType& code() { return this->_code; }
	string& name() { return this->_name; }
	string& description() { return this->_description; }
	BlockPos& size() { return this->_size; }
	BlockPos& padding() { return this->_position; }
	BlockPos& center() { return this->_anchor; }
	string& picture() { return this->_picture; }
	vector<ObjWeak>& children() { return this->_children; }
	WalkType& walktype() { return this->_walktype; }
	AlphaBMP& walkBMP() { return this->_alphaWalkableBMP; }
	vector<JumpData>& jumpInfo() { return this->_jumpInfo; }
	LinkerPtr& linker() { return this->_link; }
	TransPtr& translator() { return this->_translator; }
	ValuePtr& value() { return this->_value; }


	virtual GameJudgeResult link(GameBasicCommand gcmd, EventPtr& out_event);
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

enum GameBasicCommand {
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

class GameTranslator {
private:
	std::function<GameBasicCommand(bool*)> convert = [](bool* key) { return GameBasicCommand::emptyCmd; }; // 利用匿名函数吗
public:
	GameTranslator() { }
	virtual GameBasicCommand translate(bool* arrOfKeys);
};

class GameLinker {
private:
	std::function<LinkerSecond(GameBasicCommand)> convert = [](GameBasicCommand gcmd) { return LinkerSecond(); };
public:
	GameLinker() { }
	virtual LinkerSecond link(GameBasicCommand);
};

class GameEvent {
private:
	// TODO
public:
	GameEvent() {}

	virtual bool start(LiveObjPtr obj);
};

class GameBase {
private:
	std::vector<ObjPtr> stuffData;
	std::vector<ObjPtr> sceneData;
	std::vector<TransPtr> controlData;
	std::vector<UIPtr> UIData;
public:
	void init();
	ObjPtr getStuff(CodeType code);
	ObjPtr getScene(CodeType code);
	TransPtr getControl(CodeType code);
	UIPtr getUI(CodeType code);
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

};

enum UICode {
	StartPageCode,
};
