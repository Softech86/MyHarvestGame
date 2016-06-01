#pragma once

#include <memory>
#include <vector>
#include "ClassHeader.h"
#include "GameBase.h"
#include "GameUI.h"
#include "GamePrincipal.h"

class GameLiveObject {
friend class GameLiveScene;
friend class GameObjectJudge;
friend class GameLive;
friend class GameLiveHuman;
public:
    enum StickTo {
        background, flat, cube, cloud
	};
private:
	static const int layerOrderMultiplier;
    ObjPtr _obj;
    LiveCode _paintCode = nullptr;
    BlockPos _margin;
    int _z = 0; // TODO z = (z, 0) move needs speed settings 
	float _order = 0;
    float _scale = 1;
    float _alpha = 1;
    StickTo _stickTo = cube;
    // it is from when the game started in seconds
	float _movingUntil = 0;
	float _alphaUntil = 0;
	float _lockUntil = 0;
	float _animeUntil = 0;
	BlockPos::Direction _face = BlockPos::Direction::five;
    vector<LiveObjWeak> _inBind;
	vector<LiveObjWeak> _outBind;
public:
    GameLiveObject() {
    }
private:
    GameLiveObject(ObjPtr pobj, bool copy = true);
    GameLiveObject(ObjPtr pobj, const BlockPos& margin, bool copy = true);

    GameLiveObject(BaseCode code, bool isScene)
    : GameLiveObject(isScene ? BASE.getScene(code) : BASE.getStuff(code), true) {
    }

public:
    ObjPtr getObj() {
        return this->_obj;
    }
private:
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
	BlockPos getCenter() { return this->getObj()->getCenter(); }
	float getActionLock() { return this->_lockUntil; }
	void setActionLock(float until) { this->_lockUntil = until; }

public:
	// 内外位置之和
    BlockPos MP() {
        return this->margin() + this->padding();
    }
	BlockPos MPC() {
		return this->MP() + this->getCenter();
	}
public:
	BlockPos::Direction getFace() { return this->_face; }
	void setFace(LiveObjPtr selfptr, BlockPos::Direction face) {
		if (selfptr != nullptr) {
			if (getFace() != face && this->getObj()->onFaceChange(selfptr, this->getFace(), face))
				this->_face = face;
		}
		else
			cocos2d::log("[LiveObject].setFace nullptr");
	}
    BlockPos paintPos();
	void decodePaintPos(BlockPos pos);
	int paintLayerOrder(int dotOrder);
	float getZorder() { return this->_order; }
	void setZOrder(float Zorder);
	void setZOrder(float Zorder, float timeSec, float delaySec);
	void autoZOrder(int dotOrder);
	void autoZOrder(int dotOrder, float timeSec, float delaySec);
	float getAlpha() { return this->_alpha; }
	void setAlpha(float alpha, float timeSec = 0, float delaySec = 0);

    LiveCode paint(LiveCode father, int dotOrder);
    void erase(LiveCode father);
    void move(const BlockPos& vec, MoveType move, float timeSec, float delaySec = 0);
    void cleanMove(const BlockPos& vec, MoveType move, float timeSec, float delaySec = 0);
	LiveCode repaint(LiveCode father, int dotOrder);
	LiveCode changePicture(const string& pic, LiveCode father, int dotOrder);
	void alpha(float newalpha, float timeSec, float delaySec = 0);

	static StickTo whereToStick(GameObject::BigType type);
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
friend class GameLive;
public:
    enum Layer { background, ground, building, furniture, stuff, animal, npc, kid, LAYER_MAX = kid };
    const static int LAYER_COUNT = LAYER_MAX + 1;
private:
    ObjPtr scene = nullptr;
    PxPos viewPoint;
    float viewMovingUntil = 0;
    BlockPos windowSize;
    // the padding of sceneObject represents the size of area we can see in the game
    // and this MazeSize represents the area to be stored and calculated
    // MazeSize must be larger than the padding size, or I don't know what will happen
    BlockPos mazeSize;
    LiveDot* blockMap = nullptr;
    LiveObjPtr focusOn = nullptr;
	LiveCode codeRoot = nullptr;
    LiveCode codeStuff = nullptr;
	LiveCode codeBackground = nullptr;
    LiveCode codeFlat = nullptr;
    LiveCode codeCube = nullptr;
	LiveCode codeCloud = nullptr;
	LiveCode codeEffect = nullptr;
	LiveCode codeLens = nullptr;
	LiveCode codeDim = nullptr;
	LiveObjPtr liveScene = nullptr;
    map<LiveCode, LiveObjPtr> dict;
    vector<LiveObjPtr> nodeCache;

	float _animeUntil = 0;

    // 6 big blocks per sec
	float walkSpeed = GameBase::KID_MOVE_SPEED_IN_BIG_BLOCKS * BIG_TO_SMALL;
    // 1 small block per step
	BlockType step = GameBase::KID_STEP;
    // run velocity is twice of walk velocity
	float runComparedToWalk = GameBase::KID_RUN_COMPARED_TO_WALK;

	//To deprecate
    //TransPtr defaultTranslator = nullptr;
	//GameCommand commandCache = GameBase::DEFAULT_COMMAND;

	int detectSplit = GameBase::DETECT_SPLIT;

	static int insertPositionCompare(LiveObjPtr lhs, LiveObjPtr rhs);
	static LiveDot::iterator findInsertPosition(LiveDot& ld, LiveObjPtr obj);
	static int liveDotInsert(LiveDot &ld, LiveObjPtr obj);
	static int liveDotOrderFind(LiveDot &ld, LiveObjPtr obj);

	BlockPos validize(const BlockPos& input);
	void rangeGetObjects(BlockPos start, BlockPos size, map<LiveObjPtr, int>& out_objects, LiveObjPtr itself);
	static void sortObjects(const BlockPos& kidPos, BlockPos::Direction kidDir, map<LiveObjPtr, int>& objects, vector<LiveObjPtr> &outResult);
	void kidRangeObjects(vector<LiveObjPtr>& out_result);

    // these four functions only moves one object in the blockMap
    int blockAdd(LiveObjPtr ptr);
    void blockRemove(LiveObjPtr ptr);
    int blockMove(LiveObjPtr oldptr, const BlockPos& vec);
    int blockReplace(LiveObjPtr oldptr, ObjPtr newobj);
	int blockIndexQuery(LiveObjPtr ptr);

    // remove a layer in a dot, if you want to remove all of a layer, use mapRemove
    static void dotRemoveLayer(LiveDot& ld, LiveObjPtr ptr);

    static void addBind(LiveObjPtr outptr, LiveObjPtr inptr);
    static void removeBind(LiveObjPtr outptr, LiveObjPtr inptr);
    static void removeAllOutBind(LiveObjPtr ptr);
    static void removeAllInBind(LiveObjPtr ptr);

    int mapAdd(LiveObjPtr ptr, bool recursive = false);
    void mapRemove(LiveObjPtr ptr, bool recursive = false);
    void mapRemoveOutBind(LiveObjPtr ptr, bool recursive = false);
    int mapMove(LiveObjPtr ptr, const BlockPos& vec, bool recursive = false);
    int mapReplace(LiveObjPtr oldptr, ObjPtr newobj);

	int getDotOrder(LiveObjPtr ptr);
	void dotZOrderRefresh(const BlockPos& pos);

    void dictAdd(LiveCode code, LiveObjPtr obj);
    void dictRemove(LiveCode code);

    void cacheAdd(LiveObjPtr ptr);
    void cacheRemove(LiveObjPtr ptr);

	LiveObjPtr make_(ObjPtr obj, const BlockPos& margin, const BlockPos& mappadding, BlockPos parentAdd, int z, float scale, float alpha, bool copy);
    LiveCode getParent(LiveObjPtr obj);

    BlockPos nextVectorToApproachALine(const BlockPos& lineTarget, const BlockPos& now);
    
    
    enum LineReturn {
        NEVER, XDIR, YDIR
    };
    LineReturn DistanceToCentralLine(const PxPos& windowRelative, const PxPos& direction, PxPos& outResult);
    LineReturn DistanceToTheOtherLine(const PxPos& windowRelative, const PxPos& direction, PxPos& outResult);
    static PxPos distProcess(const PxPos& dist, LineReturn line, const PxPos& moveAll);
	// 返回的是双轴移动的距离
    static PxPos moveBreak(const PxPos& moveAll, const PxPos& move2, const PxPos& direction);
public:
    GameLiveScene() {
    }

public:
	LiveCode rootCode() {
		return this->codeRoot;
	}

	LiveCode backgroundCode() {
		return this->codeBackground;
	}

    LiveCode stuffCode() {
        return this->codeStuff;
    };

    LiveCode flatCode() {
        return this->codeFlat;
    }

    LiveCode cubeCode() {
        return this->codeCube;
    }

	LiveCode cloudCode() {
		return this->codeCloud;
	}

	BaseCode getCode() {
		return this->scene->code();
	}

	int stuffFind(BaseCode stuff, const BlockPos& position);
	bool stuffExistsOn(BaseCode stuffcode, const BlockPos& position);

    // we get the code once it is painted, then we should add it into the dictionary
    LiveObjPtr queryCode(LiveCode code);
    // mix some numbers into a LiveObj
    LiveObjPtr make(BaseCode ptr, bool scene, const BlockPos& margin, int z = 0, float scale = 1, float alpha = 1);
    LiveObjPtr make(ObjPtr ptr, const BlockPos& margin, bool copy, int z = 0, float scale = 1, float alpha = 1);

    BlockPos getStepDist(BlockPos::Direction dir) const {
        BlockPos move = BlockPos(step, step);
        BlockPos tmp = BlockPos::dirToBlock(dir);
        return tmp * move;
    }

    float getWalkTime(const BlockPos& dist) const {
        return BlockPos::time(dist, walkSpeed);
    }

    float getRunTime(const BlockPos& dist) const {
        return getWalkTime(dist) / runComparedToWalk;
    }

    // there are some root nodes in every Live Scene, this method should create it
    void init(const BlockPos& Mazesize);
    void setScene(BaseCode scenecode);

    // add a stuff to the scene
    LiveObjPtr add(ObjPtr ptr, const BlockPos& margin, bool copy = true, int z = 0);
    // add a stuff to the scene, this LiveObject should not be painted at the moment
    void add(LiveObjPtr j);
    void remove(LiveObjPtr ptr, bool recursive = true);
    void movemove(LiveObjPtr ptr, const BlockPos& vec, MoveType move, float timeSec, bool recursive = true);
    void replace(LiveObjPtr oldptr, ObjPtr newptr);
	void changePicture(LiveObjPtr ptr, const string& picture);

    // TODO breakpause
    void setFocus(const LiveObjPtr ptr, bool breakpause = false);

    LiveObjPtr getFocus() const {
        return this->focusOn;
    }

	bool isFocusOnHuman(BaseCode humanCode);

	// 对于一个绝对坐标给出focus视点的绝对坐标
    PxPos focus(const PxPos& newpos);
	// rollback是一个对于dalyaSec为负数的确认提示
	void focusMoveViewPoint(LiveObjPtr obj, const PxPos& oldpos, const PxPos& newpos, bool flash, float speed);
    void setViewPoint(const PxPos& point);
    void moveViewPoint(const PxPos& point, float speedInBlocksPerSecond, float delaySec = 0, bool rollback = false);


	LiveObjPtr humanSet(BaseCode humancode, ObjPtr child, const BlockPos& margin, bool copy = true);
	void humanMove(BaseCode humancode, const BlockPos& vec, MoveType type, float time, bool recursive = true);
	void humanWalk(BaseCode humancode, const BlockPos& vec);
	void humanRun(BaseCode humancode, const BlockPos& vec);
	void humanRemove(BaseCode humancode, bool recursive = true);
	void humanReplace(BaseCode humancode, ObjPtr newhuman);

 //   LiveObjPtr kidSet(ObjPtr child, const BlockPos& margin, bool copy = true);
 //   void kidMove(const BlockPos& vec, MoveType type, float time, bool recursive = true);
 //   void kidWalk(const BlockPos& vec);
 //	  void kidRun(const BlockPos& vec);
 //   void kidRemove(bool recursive = true);
 //   void kidReplace(ObjPtr newkid);

	LiveObjPtr humanAddObject(BaseCode humancode, ObjPtr obj, const BlockPos& marginRelative, bool copy = true, int z = 4);

	void switchFromSurroundingsToHuman(BaseCode humancode, LiveObjPtr obj, const BlockPos& margin, bool recursive = false, int z = 4);
	void switchFromHumanToSurroundings(LiveObjPtr obj, const BlockPos& margin, bool recursive = false);

    void allDim(bool black = true, float timeSec = 1);
	void allDimFrom(bool black = true, float timeSec = 1);
	void removeDim(float delaySec = 0);
    void allClear();

	void setWindowSize(const BlockPos& size) {
		this->windowSize = size;
		if (this->codeRoot != nullptr) {
			PxPos temp = size;
			this->codeRoot->setContentSize(cocos2d::Size(temp.x, temp.y));
		}
	}

    BlockPos getWindowRelativePosition(const BlockPos& pos);
    static BlockPos getObjectRelativePosition(const BlockPos& pos, LiveObjPtr ptr);
    Walkable detect(LiveObjPtr ptr, const LiveDot& ld, const BlockPos& current, LiveObjPtr& out_jumpObj);
    Walkable detect(LiveObjPtr ptr, const BlockPos::Direction& dir, LiveObjPtr& out_jumpObj);

    enum detectMoveReturn {
        canMove, cannotMove, breakedMove
    };
	// TODO 关于检测移动这里还有一个坑要填啊
    detectMoveReturn detectMoveOneObject(LiveObjPtr obj, const BlockPos& vec, MoveType move, float timeSec);
    BlockPos detectDistanceCouldMove(LiveObjPtr obj, const BlockPos& vec, Walkable& out_walk, LiveObjPtr& out_jumpObj);

    ~GameLiveScene() {
        if (blockMap != nullptr)
            delete[] blockMap;
    }
};

class GameTime {
public:
	int year = 1;
	int season = SeasonType::haru;
	int date = 1;
	int hour = 6;
	int minute = 0;

	GameTime() {}

	void timeAdd(int minutes = 10);
	void hourAdd(int hours = 1);
	void dayAdd(int days = 1);
	void seasonAdd(int season = 1);
};

class GameLivePlant {
	BaseCode _plant = -1;
	BaseCode _plantScene = -1;
	BlockPos _plantPos = BlockPos::zero;
	GameTime _plantTime;
	bool _refresh = true;
	bool _added = false;
	int _stage = 1;
	int _water = 0;
	int _sun = 0;
	int _energy = 0;

public:
	GameLivePlant() {}
	GameLivePlant(BaseCode plant, BaseCode plantScene, const BlockPos& plantPos);
	void levelRefresh();
	void add(int water, int sun) {
		this->_water += water;
		this->_sun += sun;
		if (_refresh)
			levelRefresh();
	}
	int getStage() { return this->_stage; }
	void setStage(int stage) { this->_stage = stage; _refresh = false; }
	void setRefresh(bool refresh) { this->_refresh = refresh; }
	BlockPos getMargin() const { return this->_plantPos; }
	const string& getCSB() const;
	ObjPtr getObject() const;

	void onSceneClose(GameLiveScene* oldscene);
	void onSceneCreate(GameLiveScene* newscene);
	void onDayPass();
};

class GameLiveHuman {
	BaseCode _human = -1;
	LiveObjPtr _liveObj = nullptr; // 不显示的时候这项恢复到默认值
	ObjPtr _handObject = nullptr;
	vector<ObjPtr> _pack;
	LiveObjPtr _handObjLive = nullptr;
	BaseCode _tool = toolStart;
	int _toolCount = 1;
	int _packMaxSize = 12;

	string _name;
	int _energy = 100;
	int _stage = 0;
	map<BaseCode, int> _haoGanDu;

public:
	enum RangeType { objectRelative, zeroRelative };
	// judge range is rectangle now
	RangeType rangeType = RangeType::objectRelative;
	// if objectRelative:
	//	the center of the range area is rangeCenter + this->MP() + this->center()
	// if zeroRelative:
	//	the center of the range area is rangeCenter + BlockPos::zero
	BlockPos rangeCenter = BlockPos::zero;
	// should be positive, or it will be backward
	// if objectRelative:
	//	rangeArea.x is the arm length in front
	//	rangeArea.y is the arm length in left or right
	// if zeroRelative:
	//	rangeArea.x is the length in x from center
	//	rangeArea.y is the length in y from center
	//	the range area size in total will be (2x, 2y)
	BlockPos rangeArea = BlockPos(4, 1);

public:
	GameLiveHuman(){}
	GameLiveHuman(BaseCode humanCode);
	void copyFrom(HumanPtr human);
	LiveObjPtr getLive() { return this->_liveObj; }
	void setLive(LiveObjPtr liveObj) { this->_liveObj = liveObj; }
	void removeLive(){ this->_liveObj = nullptr; }
	vector<ObjPtr>& getPack() { return this->_pack; }
	ObjPtr getHand() { return this->_handObject; }
	bool setHand(const ObjPtr& obj) {
		if (this->_handObject == nullptr) {
			this->_handObject = obj;
			return true;
		}
		else
			return false;
	}
	ObjPtr dropHand() {
		if (this->_handObject != nullptr && this->_handObject->isDropable()) {
			int cnt = 0;
			if ((cnt = this->_handObject->getCount()) > 0) {
				this->_handObject->setCount(cnt - 1);
			}
			else {
				this->_handObject = nullptr;
				return nullptr;
			}
			ObjPtr result;
			if (cnt == 0) {
				result = _handObject;
				result->setCount(1);
				this->_handObject = nullptr;
			}
			else {
				result = ObjPtr(this->_handObject->SHCP());;
			}
			result->setCount(1);
			return result;
		}
		else
			return nullptr;
	}
	void setHandLive(LiveObjPtr obj) { this->_handObjLive = obj; }
	LiveObjPtr getHandLive() { return this->_handObjLive; }
	bool handIsEmpty() { 
		if (_handObject == nullptr)
			return true;
		else if (_handObject->getCount() == 0) {
			_handObject = nullptr;
			return true;
		}
		else
			return false;
	}

	ObjPtr* hasStuffInPack(ObjPtr obj);
	bool putIntoPack();

	int getToolCount() { return this->_toolCount; }

	void getRange(BlockPos& out_start, BlockPos& out_size);
	GameCommand toolUse() { 
		if (_tool > toolStart && _tool < toolEnd && _toolCount > 0)
			return useTool;
		//TODO 这里之后还可以做减体力之类的事情
		else
			return emptyCmd;
	}
	BaseCode toolGet() { return this->_tool; }
	void toolSet(BaseCode toolStuffCode, int count = 1) {
		if (toolStuffCode >= toolStart && toolStuffCode < toolEnd) {
			this->_toolCount = count;
			this->_tool = toolStuffCode;
		}
		else
			toolStuffCode = toolStart;
	}
	void toolLoseStep() {
		if (_tool > toolStart && _tool < toolEnd) {
			this->_toolCount -= BASE.getStuff(_tool)->getQuality();
			if (_toolCount <= 0) {
				this->_tool = toolStart;
				this->_toolCount = 0;
			}
		}
	}
};

class GameObjectJudge {
	BaseCode _humancode = humanStart;
	LiveObjPtr _objJudged = nullptr;
	GameCommand _cmdcache = emptyCmd;
	TransPtr _translator = BASE.getTranslator(basicObjectTranslator);
	GameLive* _live = nullptr;

private:
	void setObject(LiveObjPtr obj) { this->_objJudged = obj; }
public:
	GameObjectJudge() {}
	GameObjectJudge(GameLive* live, BaseCode humancode, GameCommand cmdcache) : _live(live), _humancode(humancode), _cmdcache(cmdcache) {}

	LiveObjPtr getObjectPtrJudgedNow() const { return this->_objJudged; }
	LiveCode getObjectNodeJudgedNow() {
		if (getObjectPtrJudgedNow())
			return getObjectPtrJudgedNow()->paintCode();
		else
			return nullptr;
	}
	BaseCode getObjectBaseCodeJudgedNow() {
		if (getObjectPtrJudgedNow() && getObjectPtrJudgedNow()->getObj())
			return getObjectPtrJudgedNow()->getObj()->code();
		else
			return -1;
	}
	GameCommand getCmdCache() { return this->_cmdcache; }
	void setCmdCache(GameCommand cmdcache) { this->_cmdcache = cmdcache; }
	BaseCode getHumanCode() { return this->_humancode; }
	LiveObjPtr getHumanPtr();
	TransPtr setTranslator(TransPtr tr) { this->_translator = tr; }

	void translate(float* keyarray) {
		if (_translator != nullptr)
			setCmdCache(_translator->translate(keyarray));
	}
	void autoTranslate(float* keyarray) {
		if (this->_cmdcache == emptyCmd)
			translate(keyarray);
	}
	JudgeReturn judge(float* keyarray, GameLiveScene& scene, vector<LiveObjPtr>& objects);
};

class GameLiveCreature {
	vector<GameLivePlant> plantData[sceneEnd]; // Scene and its Plants
	GameLiveHuman humanData[humanEnd]; // 人物请严格把对应的humanCode的东西放在对应的index上面
	
public:
	GameLiveCreature() {
	}

	void init() {
		for (int i = humanStart + 1; i < humanEnd; i++)
			humanData[i].copyFrom(BASE.getHuman(i));
	}

	GameLiveHuman* getHuman(BaseCode humanCode) {
		if (humanCode < humanEnd && humanCode > humanStart)
			return humanData + humanCode;
		else
			return nullptr;
	}
	vector<GameLivePlant>* getPlants(BaseCode sceneCode) {
		if (sceneCode < sceneEnd && sceneCode > sceneStart)
			return plantData + sceneCode;
		else
			return nullptr;
	}

	//Human related
	LiveObjPtr getHumanPtr(BaseCode human) {
		auto temp = getHuman(human);
		if (temp)
			return temp->getLive();
		else
			return nullptr;
	}
	bool setHumanPtr(BaseCode human, LiveObjPtr obj) {
		auto temp = getHuman(human);
		if (temp) {
			temp->setLive(obj);
			return true;
		}
		else {
			return false;
		}
	}
	LiveObjPtr getKidPtr() { return getHumanPtr(kidHumanCode); }
	bool setKidPtr(LiveObjPtr obj) { return setHumanPtr(kidHumanCode, obj); }

	// Plant Related
	bool addPlant(BaseCode plant, BaseCode sceneCode, const BlockPos& margin) {
		vector<GameLivePlant>* temp;
		if ((temp = getPlants(sceneCode)) != nullptr) {
			if (hasPlant(sceneCode, margin))
				return false;
			temp->emplace_back(plant, sceneCode, margin);
			return true;
		}
		return false;
	}
private:
	static vector<GameLivePlant>::iterator _findPlant(vector<GameLivePlant>& container, const BlockPos& margin);
	static bool _removePlant(vector<GameLivePlant>& container, const BlockPos& margin);
public:
	bool hasPlant(BaseCode sceneCode, const BlockPos& margin);
	bool removePlant(BaseCode sceneCode, const BlockPos& margin);

	void removeAllHumanLive() {
		for (int i = humanStart + 1; i < humanEnd; i++) {
			this->humanData[i].removeLive();
		}
	}

	void onSceneClose(GameLiveScene* oldscene);
	void GameLiveCreature::onSceneCreate(GameLiveScene* newscene);
	void GameLiveCreature::onDayPass();
};

class GameLive {
public:
	enum ActionResult { timeBlocked, done, cannotDo };
	static const int DELEGATE_ROLLBACK = -1;

    static const int LOOP_FREQ_MS = 20;
    static const int LOOP_DEVATION_MS = 5;
    static const int KEY_COUNT = 20;
    static const string KEY_LOOP_NAME;
private:
    float _loopfreq = (float) LOOP_FREQ_MS / 1000;
    float _loopdevation = (float) LOOP_DEVATION_MS / 1000;
    vector<LiveUIPtr> _UIUp;
    vector<LiveUIPtr> _UIDown;
    GameLiveScene* _scene = nullptr;
	GameTime _time;
	GameLiveCreature* _creature = nullptr;
    bool _close = false;
    bool* _press = nullptr;
    float* _keys = nullptr;
	LiveUIPtr _UIJudgeNow = nullptr;
	LiveCode _codeDim = nullptr;
	float _animeUntil = 0;

	vector<GameLiveScene*> _trash;

public:
	bool noPressStillJudge = false;

	GameLive();

    float getLoopFreq() {
        return this->_loopfreq;
    }

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
    int keyAddTime();

private:
    bool _keyCyclePushed(float time, float cycleSec);
    vector<LiveUIPtr>::iterator _UIPtrQuery(LiveCode id, GameUI::UIType& out_type);
	vector<LiveUIPtr>::iterator _UIPtrQuery(BaseCode code, GameUI::UIType& out_type);
public:
    bool keyPushedOnly(float* keyarray, GameKeyPress gkp);
    bool keyPushedOnly(float* keyarray, vector<GameKeyPress> vgkp);
    bool keyJustPushed(float* keyarray, GameKeyPress gkp, bool only);
	bool keyJustPushed(float* keyarray, vector<GameKeyPress> vgkp, bool only);
	bool keyCyclePushed(float* keyarray, GameKeyPress gkp, float cycleSec, bool only);
	bool keyCyclePushed(float* keyarray, vector<GameKeyPress> vgkp, float cycleSec, bool only);
	bool keyPushed(float* keyarray, GameKeyPress gkp, bool only);
	bool keyPushed(float* keyarray, vector<GameKeyPress> vgkp, bool only);

	float api_getLoopFreq() { return this->_loopfreq; }
	
	BlockPos api_getSceneSize() { if (this->_scene) return this->_scene->windowSize; else return BlockPos::zero; }
    bool api_setSceneSize(const BlockPos& size);
    void api_UIStart(BaseCode uicode);
    void api_UIStart(UIPtr uiptr);
	void api_UIStop(LiveCode id);
	void api_UIStop(BaseCode code);
	LiveUIPtr api_getUI(BaseCode code);
    void api_eventStart(BaseCode eveCode, LiveObjPtr obj);
    void api_eventStart(EventPtr eve, LiveObjPtr obj);

    void api_close();

    bool api_hasScene() {
        return this->_scene != nullptr;
    }

    GameLiveScene* api_getScenePtr() {
        return this->_scene;
    }
    LiveCode api_getStuffCode() {
        return this->_scene->stuffCode();
    }
	BaseCode api_getSceneCode() {
		if (this->_scene && this->_scene->scene)
			return this->_scene->scene->code();
		else
			return sceneStart;
	}
    LiveCode api_getCubeCode() {
        return this->_scene->cubeCode();
    }
    LiveCode api_getFlatCode() {
        return this->_scene->flatCode();
    }
	LiveCode api_getCloudCode() {
		return this->_scene->cloudCode();
	}
    LiveUIPtr api_getUIPtrJudgedNow() {
        return this->_UIJudgeNow;
    };
    LiveCode api_getUICodeJudgedNow() {
        if (this->_UIJudgeNow != nullptr)
            return this->_UIJudgeNow->id();
        else
            return nullptr;
    }
	void api_objectChangePicture(LiveObjPtr ptr, const string& picture) {
		this->_scene->changePicture(ptr, picture);
	}
	void api_stopUIJudgedNow() {
		// 这里不关心null问题
        LiveCode tmp = api_getUICodeJudgedNow();
        api_UIStop(tmp);
    }

	void api_removeDim(float delaySec = 0);
	void api_allDim(bool black = true, float timeSec = 1);
	void api_allDimFrom(bool black = true, float timeSec = 1);

    void api_sceneInit(BaseCode sceneCode, BlockPos mazeSize);
    void api_sceneDisplay();
    // calculate the object on scene according to the time or something more is processed here
    void api_sceneCalculate();
    void api_sceneICD(BaseCode sceneCodev, const BlockPos& mazeSize, const BlockPos& windowSize);
	void api_sceneClose();
	void api_trashClearWillTakeALongTime() { for (auto sc : _trash) delete sc; _trash.clear(); }

	void api_dayPass();

	LiveObjPtr api_humanGet(BaseCode humancode) {
		return this->_creature->getHumanPtr(humancode);
	}
	LiveObjPtr api_kidGet() {
		return api_humanGet(kidHumanCode);
	}
	GameLiveHuman* api_getHuman(BaseCode humanCode) {
		return this->_creature->getHuman(humanCode);
	}
	GameLiveHuman* api_getKidHuman() {
		return api_getHuman(kidHumanCode);
	}

	void api_humanSet(BaseCode humancode, BaseCode stuffCode, const BlockPos& pos, bool focus);
	void api_humanSet(BaseCode humancode, ObjPtr ptr, const BlockPos& pos, bool focus, bool copy = true);
	void api_humanWalk(BaseCode humancode, const BlockPos& vec);
	void api_humanRun(BaseCode humancode, const BlockPos& vec);
	void api_humanWalkStep(BaseCode humancode, BlockPos::Direction dir, LockType lock = doNothing);
	void api_humanRunStep(BaseCode humancode, BlockPos::Direction dir, LockType lock = doNothing);

	void api_kidSet(BaseCode stuffCode, const BlockPos& pos, bool focus);
    void api_kidSet(ObjPtr ptr, const BlockPos& pos, bool focus, bool copy = true);
    void api_kidWalk(const BlockPos& vec);
	void api_kidRun(const BlockPos& vec);
    void api_kidWalkStep(BlockPos::Direction dir);
	void api_kidRunStep(BlockPos::Direction dir);

private:
	int pickZValue(ObjPtr human);
	BlockPos pickPosition(LiveObjPtr human);
	BlockPos dropPosition(LiveObjPtr human);
	void pickAnimation(BaseCode humancode, LiveObjPtr stuff);
	void dropAnimation(BaseCode humancode, LiveObjPtr stuff, const BlockPos& dropPosition);

public:
	ActionResult api_humanPick(BaseCode humancode, LiveObjPtr stuff);
	ActionResult api_humanDrop(BaseCode humancode);
	ActionResult api_kidPick(LiveObjPtr stuff);
    // void api_kidJump(BaseCode sceneCode, BlockPos blocksize, BlockPos kidpos);
	LiveObjPtr api_humanAddObject(BaseCode humancode, ObjPtr obj, const BlockPos& marginRelative, bool copy = true, int z = 0);

	LiveObjPtr api_addObject(BaseCode stuffCode, const BlockPos& margin, bool autoAdd = true, int z = 0);
	LiveObjPtr api_addObject(ObjPtr object, const BlockPos& margin, bool autoAdd = true, bool copy = true, int z = 0);
	void api_moveObject(LiveObjPtr obj, const BlockPos& vec, MoveType type, float timeSec, bool recursive = true);
	void api_replaceObject(LiveObjPtr obj, ObjPtr ptr);
	void api_changePicture(LiveObjPtr obj, const string& newcsb);
	void api_removeObject(LiveObjPtr obj, bool recursive = true, bool autoRemove = true);

	bool api_hasPlant(BaseCode sceneCode, const BlockPos& position);
	void api_toolLose(BaseCode humanCode);

	GameTime api_getGameTime() { return this->_time; }
	int api_getSeason() { return this->_time.season; }

	// refer上面写着个函数的主要关心的物体，取消的时候就会调用它，如果不知道就写&LIVE
	static void api_delayTime(const CocoFunc& func, float delaySec, const string& key, void* refer, int repeat = 0, int interval = 0);
	static void api_undelay(void* refer, const string &key);
	static void api_undelayAll(void* refer);

	static float api_getActionLock(LiveObjPtr obj) { return obj->getActionLock(); }
	static void api_setActionLock(LiveObjPtr obj, float lock) { obj->setActionLock(lock); }
	static void api_addActionLock(LiveObjPtr obj, float lockAdd);

	// 返回有没有执行操作
	// type: 
	// doAfter动作追加
	// doBreak从现在开始经过delaySec后清空任务立刻执行
	// doClearBreak现在清空开始计时
	// doOverwrite直接复写
	// doNothing现在lock时不操作
	static bool api_autoLock(float& lockvalue, float timeSec, LockType type, const CocoFunc& func, const string& key, float delaySec = 0, int repeat = 0, int interval = 0);

	// 返回有没有执行操作
	// type: 
	// doAfter动作追加
	// doBreak从现在开始经过delaySec后清空任务立刻执行
	// doClearBreak现在清空开始计时
	// doOverwrite直接复写
	// doNothing现在lock时不操作
	static bool api_autoActionLock(LiveObjPtr obj, float timeSec, LockType type, const CocoFunc& func, const string& key, float delaySec = 0, int repeat = 0, int interval = 0);
	static bool api_autoAnimeLock(LiveObjPtr obj, float timeSec, LockType type, const CocoFunc& func, const string& key, float delaySec = 0, int repeat = 0, int interval = 0);

	static bool api_delegateTimeCompare(float& lockvalue, const float& timeSec, const TimeFunc& func, int& out_return, const string& logname);
	static bool api_delegateActionTimeCompare(LiveObjPtr& obj, const float& timeSec, const TimeFunc& func, int& out_return, const string& logname);

    ~GameLive() {
		for (auto sce : _trash)
			delete sce;
        if (_scene != nullptr)
            delete _scene;
        if (_keys != nullptr)
            delete _keys;
        if (_press != nullptr)
            delete _press;
		if (_creature != nullptr)
			delete _creature;
    }
};