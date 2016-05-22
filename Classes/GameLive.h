#pragma once

#include <memory>
#include <vector>
#include "ClassHeader.h"
#include "GameBase.h"
#include "GameUI.h"
#include "GamePrincipal.h"

class GameLiveObject {
public:
    enum StickTo{ surroundings, kid };
private:
    ObjPtr _obj;
    LiveCode _paintCode = nullptr;
    BlockPos _margin;
    int _z = 0; // TODO z = (z, 0) move needs speed settings 
    float _scale = 1;
    float _alpha = 1;
    StickTo _stickTo = surroundings;
	// it is from when the game started in seconds
	float _movingUntil = 0;
    vector<LiveObjWeak> _inBind;
    vector<LiveObjWeak> _outBind;
public:
    GameLiveObject() {
    }
    GameLiveObject(ObjPtr pobj);
    GameLiveObject(ObjPtr pobj, const BlockPos& margin);

    GameLiveObject(BaseCode code, bool isScene)
    : GameLiveObject(isScene ? BASE.getScene(code) : BASE.getStuff(code)) {
    }

    ObjPtr getObj() {
        return this->_obj;
    }

	void setObj(ObjPtr obj) {
		this->_obj = ObjPtr(obj->SHCP());
	}

    LiveCode& paintCode() {
        return this->_paintCode;
    }

    string& picture() {
        return this->_obj->picture();
    }

    BlockPos& size() {
        return this->_obj->size();
    }

    BlockPos& margin() {
        return this->_margin;
    }

    BlockPos& padding() {
        return this->_obj->padding();
    }

    WalkType& walktype() {
        return this->_obj->walktype();
    }

    GameAlpha& walkBMP() {
        return this->_obj->walkBMP();
    }

    int& zValue() {
        return this->_z;
    }

    vector<LiveObjWeak>& inBind() {
        return this->_inBind;
    }

    vector<LiveObjWeak>& outBind() {
        return this->_outBind;
    }

    float& scale() {
        return this->_scale;
    }

    float& alpha() {
        return this->_alpha;
    }
    
    StickTo getStick() {
        return this->_stickTo;
    }
    
    void setStick(StickTo to) {
        this->_stickTo = to;
    }
    
	// 内外位置之和
	BlockPos MP() { return this->margin() + this->padding(); }
    BlockPos paintPos();

	LiveCode paint(LiveCode father);
	void erase(LiveCode father);
	void move(const BlockPos& vec, MoveType move, float timeSec, float delaySec = 0);
	void cleanMove(const BlockPos& vec, MoveType move, float timeSec, float delaySec = 0);
	LiveCode repaint(LiveCode father);
};

class GameLiveUI {
private:
    UIPtr _ui;
    LiveCode _id;
public:

    GameLiveUI() {
    }
    GameLiveUI(UIPtr ori);

    UIPtr UI() {
        return this->_ui;
    }

    LiveCode& id() {
        return this->_id;
    }
};

class GameLiveScene {
private:
    ObjPtr scene = nullptr;
    PxPos viewPoint;
	float viewMovingUntil = 0;
    BlockPos windowSize;
    // the padding of sceneObject represents the size of area we can see in the game
    // and this MazeSize represents the area to be stored and calculated
    // MazeSize must be larger than the padding size, or I don't know what will happen
    BlockPos mazeSize;
	LiveObjPtr focusOn = nullptr;
    LiveCode codeRoot = nullptr;
    LiveCode codeSurrounding = nullptr;
    LiveCode codeKid = nullptr;
	LiveObjPtr liveKid = nullptr;
    map<LiveCode, LiveObjPtr> dict;
	vector<LiveObjPtr> nodeCache;
	LiveObjPtr liveHand = nullptr;
	int NumHand = 0;

	// 3 big blocks per sec
	float kidMoveSpeed = 3 * BIG_TO_SMALL;
	// 1 small block per step
	BlockType kidStep = 1;
	// run velocity is twice of walk velocity
	float kidRunComparedToMove = 2.0f;

    LiveDot* blockMap = nullptr;
    TransPtr defaultTranslator = nullptr;

    static const int detectSplit = 2;

    // these four functions only moves one object in the blockMap
    void blockAdd(LiveObjPtr ptr);
    void blockRemove(LiveObjPtr ptr);
    void blockMove(LiveObjPtr oldptr, const BlockPos& vec);
    void blockReplace(LiveObjPtr oldptr, ObjPtr newobj);

    // remove a layer in a dot, if you want to remove all of a layer, use mapRemove
    static void dotRemoveLayer(LiveDot& ld, LiveObjPtr ptr);

    // all object's inBind vector doesn't contains the map, which is one of the root nodes
    static void addBind(LiveObjPtr outptr, LiveObjPtr inptr);
    static void removeBind(LiveObjPtr outptr, LiveObjPtr inptr);
    static void removeAllOutBind(LiveObjPtr ptr);
    static void removeAllInBind(LiveObjPtr ptr);
   
    void mapAdd(LiveObjPtr ptr, bool recursive = false);
    void mapRemove(LiveObjPtr ptr, bool recursive = false);
    void mapRemoveOutBind(LiveObjPtr ptr, bool recursive = false);
    void mapMove(LiveObjPtr ptr, const BlockPos& vec, bool recursive = false);
    void mapReplace(LiveObjPtr oldptr, ObjPtr newobj);

    void dictAdd(LiveCode code, LiveObjPtr obj);
    void dictRemove(LiveCode code);

	void cacheAdd(LiveObjPtr ptr);
	void cacheRemove(LiveObjPtr ptr);
	
    LiveObjPtr make_(ObjPtr obj, GameLiveObject::StickTo stick, const BlockPos& margin, const BlockPos& mappadding, BlockPos parentAdd, int z, float scale, float alpha);
	LiveCode getParent(LiveObjPtr obj);

	BlockPos nextVectorToApproachALine(const BlockPos& lineTarget, const BlockPos& now);

	enum LineReturn { NEVER, XDIR, YDIR };
	LineReturn DistanceToCentralLine(const PxPos& windowRelative, const PxPos& direction, PxPos& outResult);
	LineReturn DistanceToTheOtherLine(const PxPos& windowRelative, const PxPos& direction, PxPos& outResult);
	static PxPos distProcess(const PxPos& dist, LineReturn line, const PxPos& moveAll);
	// 返回的是双轴移动的距离
	static PxPos moveBreak(const PxPos& moveAll, const PxPos& move2, const PxPos& direction);
public:
    GameLiveScene() { }

public:
    LiveCode rootCode() { return this->codeRoot; };
	LiveCode surroundingCode() { return this->codeSurrounding; }
	LiveCode kidCode() { return this->codeKid; }
	LiveObjPtr kidPtr() { return this->liveKid; }
    
    // we get the code once it is painted, then we should add it into the dictionary
    LiveObjPtr queryCode(LiveCode code); 
    // mix some numbers into a LiveObj
    LiveObjPtr make(BaseCode ptr, bool scene, GameLiveObject::StickTo stick, const BlockPos& margin, int z = 0, float scale = 1, float alpha = 1);
    LiveObjPtr make(ObjPtr ptr, GameLiveObject::StickTo stick, const BlockPos& margin, int z = 0, float scale = 1, float alpha = 1);
	
	
	BlockPos getStepDist(BlockPos::Direction dir) const {
		BlockPos move = BlockPos(kidStep, kidStep);
		BlockPos tmp = BlockPos::dirToBlock(dir);
		return tmp * move;
	}
	float getKidWalkTime(const BlockPos& dist) const {
		return BlockPos::time(dist, kidMoveSpeed);
	}
	float getKidRunTime(const BlockPos& dist) const {
		return getKidWalkTime(dist) / kidRunComparedToMove;
	}

    // there are three root nodes in every Live Scene, this method should create it
    void init(const BlockPos& Mazesize);
	void setScene(BaseCode scenecode);

    // add a stuff to the scene
    LiveObjPtr add(ObjPtr ptr, GameLiveObject::StickTo stick, const BlockPos& margin);
    // add a stuff to the scene, this LiveObject should not be painted at the moment
	void add(LiveObjPtr j);
    void remove(LiveObjPtr ptr, bool recursive = true);
    void movemove(LiveObjPtr ptr, const BlockPos& vec, MoveType move, float timeSec, bool recursive = true);
    void replace(LiveObjPtr oldptr, ObjPtr newptr);
    
	// TODO
	void setFocus(const LiveObjPtr ptr, bool breakpause = false);
	LiveObjPtr getFocus() const { return this->focusOn; }
	bool focusOnKid() { if (this->kidPtr() == nullptr) return false; else if (this->getFocus() == this->kidPtr()) return true; }

	// 对于一个绝对坐标给出focus视点的绝对坐标
	PxPos focus(const PxPos& newpos);
	// rollback是一个对于dalyaSec为负数的确认提示
	void focusMoveViewPoint(LiveObjPtr obj, const PxPos& oldpos, const PxPos& newpos, bool flash);
	void setViewPoint(const PxPos& point);
	void moveViewPoint(const PxPos& point, float speedInBlocksPerSecond, float delaySec = 0, bool rollback = false);
	
    void kidSet(ObjPtr child, const BlockPos& margin);
    void kidMove(const BlockPos& vec, MoveType type, float time, bool recursive = true);
	void kidWalk(const BlockPos& vec);
    void kidRemove(bool recursive = true);
	void kidReplace(ObjPtr newkid);

	//TODO
	void delayTime(std::function<void()> func, float delaySec);

	void switchFromSurroundingsToKid(LiveObjPtr obj, const BlockPos& margin, bool recursive = false);
	void switchFromKidToSurroundings(LiveObjPtr obj, const BlockPos& margin, bool recursive = false);
    
    void allDim(bool black = true);
    void allClear();

	void setWindowSize(const BlockPos& size) { this->windowSize = size; }

    BlockPos getWindowRelativePosition(const BlockPos& pos);
    static BlockPos getObjectRelativePosition(const BlockPos& pos, LiveObjPtr ptr);
    Walkable detect(LiveObjPtr ptr, const LiveDot& ld, const BlockPos& current, LiveObjPtr& out_jumpObj);
    Walkable detect(LiveObjPtr ptr, const BlockPos::Direction& dir, LiveObjPtr& out_jumpObj);

	enum detectMoveReturn { canMove, cannotMove, breakedMove };
	// TODO 关于检测移动这里还有一个坑要填啊
	detectMoveReturn detectMoveOneObject(LiveObjPtr obj, const BlockPos& vec, MoveType move, float timeSec);
	BlockPos detectDistanceCouldMove(LiveObjPtr obj, const BlockPos& vec, Walkable& out_walk, LiveObjPtr& out_jumpObj);

    ~GameLiveScene() {
        if (blockMap != nullptr)
            delete[] blockMap;
    }
};

class GameLive {
public:
	static const int LOOP_FREQ_MS = 20;
	static const int LOOP_DEVATION_MS = 5;
	static const int KEY_COUNT = 20;
    static const string KEY_LOOP_NAME;
private:
    vector<LiveUIPtr> _UIUp;
    vector<LiveUIPtr> _UIDown;
	GameLiveScene* _scene = nullptr;
	float _loopfreq = (float)LOOP_FREQ_MS / 1000;
	float _loopdevation = (float)LOOP_DEVATION_MS / 1000;
    bool _close = false;
	bool* _press = nullptr;
    float* _keys = nullptr;
	float _timeInGame = 0;

	LiveObjPtr _objectJudgeNow = nullptr; // TODO
	LiveUIPtr _UIJudgeNow = nullptr;

public:

    GameLive() {
    }
	float getLoopFreq() { return this->_loopfreq; }

    void keySet();
    void enter();
    void init();
    void save();
    void keyLoop();
    void judge();

	bool* press() {
		return this->_press;
	}
    float* keys() {
        return this->_keys;
    }
	void keyAddTime();

private:
	bool _keyCyclePushed(float time, float cycleSec);
	vector<LiveUIPtr>::iterator _UIPtrQuery(LiveCode id, GameUI::UIType& out_type);
public:
	bool keyPushedOnly(float* keyarray, GameKeyPress gkp);
	bool keyPushedOnly(float* keyarray, vector<GameKeyPress> vgkp);
	bool keyJustPushedOnly(float* keyarray, GameKeyPress gkp);
	bool keyJustPushedOnly(float* keyarray, vector<GameKeyPress> vgkp);
	bool keyCyclePushedOnly(float* keyarray, GameKeyPress gkp, float cycleSec);
	bool keyCyclePushedOnly(float* keyarray, vector<GameKeyPress> vgkp, float cycleSec);

	bool api_setWindowSize(const BlockPos& size);
    void api_UIStart(BaseCode uicode);
    void api_UIStart(UIPtr uiptr);
    void api_UIStop(LiveCode id);
    void api_eventStart(BaseCode eveCode, LiveObjPtr obj);
    void api_eventStart(EventPtr eve, LiveObjPtr obj);
    void api_close();

	bool api_hasScene() { return this->_scene != nullptr; }
	GameLiveScene* api_getScenePtr() { return this->_scene; }
	LiveCode api_getRootCode() { return this->_scene->rootCode(); }
	LiveCode api_getKidCode() { return this->_scene->kidCode(); }
	LiveCode api_getSurroundingCode() { return this->_scene->surroundingCode(); }
	LiveUIPtr api_getUIPtrJudgedNow() { return this->_UIJudgeNow; };
	LiveCode api_getUICodeJudgedNow() {
		if (this->_UIJudgeNow != nullptr)
			return this->_UIJudgeNow->id();
		else
			return nullptr; 
	}

	void api_stopUIJudgedNow() {
		// 这里不关心null问题
		LiveCode tmp = api_getUICodeJudgedNow();
		api_UIStop(tmp);
	}

    void api_sceneInit(BaseCode sceneCode, BlockPos mazeSize);
    void api_sceneDisplay();
    // calculate the object on scene according to the time or something more is processed here
    void api_sceneCalculate();
	void api_sceneICD(BaseCode sceneCodev, const BlockPos& mazeSize, const BlockPos& windowSize);
	void api_kidSet(BaseCode kidCode, const BlockPos& pos, bool focus);
    void api_kidSet(ObjPtr ptr, const BlockPos& pos, bool focus);
    void api_kidWalk(const BlockPos& vec);
	void api_kidWalkStep(BlockPos::Direction dir);
    void api_kidPick(LiveObjPtr stuff);
    void api_kidJump(BaseCode sceneCode, BlockPos blocksize, BlockPos kidpos);

	void api_npcWalk(const BlockPos& vec);
	

    ~GameLive() {
        if (_scene != nullptr)
            delete _scene;
        if (_keys != nullptr)
            delete _keys;
		if (_press != nullptr)
			delete _press;
    }
};