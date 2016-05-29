#pragma once
#include "ClassHeader.h"

template<class PtrType>
void T_push(vector<PtrType>* container, PtrType pt, BaseCode code) {
    if (container != nullptr && code >= 0) {
        if ((int) container->size() <= code) {
            container->resize(code + 1);
        }
        (*container)[code] = pt;
    }
}

template<class childType, class parentPtr>
parentPtr T_create(BaseCode code, vector<parentPtr> *container) {
    parentPtr pt(new childType());
    T_push(container, pt, code);
    return pt;
}

class GameAlpha {
public:

    BlockPos size;
    Walkable* moveMap = nullptr;

    GameAlpha() {
    }

    GameAlpha(BlockPos size) : size(size) {
    }

    GameAlpha(BlockPos size, string BMPfile) {
        //TODO
    }

    Walkable getWalk(BlockPos pos) {
        auto pw = get(this->moveMap, this->size, pos);
        if (pw == nullptr)
            return nullWalk;
        else
            return *pw;
    }

    ~GameAlpha() {
        if (moveMap != nullptr)
            delete[] moveMap;
    }
};

class GameObject {
public:
	enum BigType {
		empty,
		background, ground, seed,
		building, furniture, stuff, plant, animal, npc, kid,
		weather, bubble,
		combStatue, combStuff,
	};

	struct JumpData {
		GameAlpha jump;
		BaseCode sceneCode = -1;
		BlockPos kidPos;
	};

	static const GameObject origin;

private:
	BigType _type = BigType::empty;
	BaseCode _code = -1;
	std::string _name;
	std::string _description;
	BlockPos _size;
	BlockPos _position;
	BlockPos _anchor;
	string _picture = "";
	vector<ObjWeak> _children;
	vector<BlockPos> _childrenPos;
	WalkType _walktype = WalkType::noneWalk;
	GameAlpha _alphaWalkableBMP;
	vector<JumpData> _jumpInfo;

	int _count = 1;
	int _quality = 0; //这个数字在物品上表示质量，在工具上表示用一次的损耗度

	bool _pickable = true;
	bool _dropable = true;

	JudgeReturn _defaultJudge = JudgeReturn::judgeNextObject;
	TransPtr _translator = nullptr;
	LinkerPtr _link = nullptr;

	bool _customPaint = false;
	bool _noFacingDifference = true;

	string _littlePicture;
	string _facingPicture[10];

public:

	GameObject() {
	}

	GameObject(
		BigType type,
		BaseCode code,
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
		_walktype(walkable) {
	}

	static ObjPtr create(
		BigType type,
		BaseCode code,
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
		T_push(container, pt, code);
		if (father != nullptr)
			father->children().push_back(pt);
		return pt;
	}

	template<class ObjectType>
	static ObjPtr create(BaseCode code, vector<ObjPtr> *container = nullptr) {
		return T_create<ObjectType>(code, container);
	}

	BigType& type() { return this->_type; }
	BaseCode& code() { return this->_code; }
	string& name() { return this->_name; }
	string& description() { return this->_description; }
	BlockPos& size() { return this->_size; }
	BlockPos& padding() { return this->_position; }
	string& picture() { return this->_picture; }
	vector<ObjWeak>& children() {
		return this->_children;
	}
	vector<BlockPos>& childrenPos() { return this->_childrenPos; }
	WalkType& walktype() { return this->_walktype; }
	GameAlpha& walkBMP() { return this->_alphaWalkableBMP; }
	vector<JumpData>& jumpInfo() { return this->_jumpInfo; }
	LinkerPtr& linker() { return this->_link; }

	BlockPos getCenter() const { return this->_anchor; }
	GameObject* const setCenter(const BlockPos& cent) { this->_anchor = cent; return this; }
	GameObject* const setChildrenLinker(LinkerPtr lin) {
		for (auto &obj : this->children()) {
			obj.lock()->linker() = lin;
		}
		return this;
	}

	TransPtr& translator() { return this->_translator; }
	string& littlePicture() { return this->_littlePicture; }
	string* getFacingPicture() { 
		for (int i = 0; i < 10; i++)
			cocos2d::log(_facingPicture[i].c_str());
		return this->_facingPicture;
	}
	GameObject* const setFacingPicture(string* newArray) {
		this->_noFacingDifference = false;
		for (int i = 0; i < 10; i++)
			this->_facingPicture[i] = newArray[i];
		return this;
	}

	GameObject* const setCustomPaint(bool custom) {
		this->_customPaint = custom;
		return this;
	}

	bool isCustomPaint() { return this->_customPaint; }
	bool isCustomTranslate() { return (bool)(this->_translator); }
	bool isPickable() { return this->_pickable; }
	bool isDropable() { return this->_dropable; }
	int getQuality() { return this->_quality; }
	GameObject* const setPickable(bool pick) { this->_pickable = pick; return this; }
	GameObject* const setDropable(bool drop) { this->_dropable = drop; return this; }
	GameObject* const setQuality(int quality) { this->_quality = quality; return this; }

	int getCount() { return this->_count; }

	GameCommand translate(float* arrOfKeys);

	SHCP_BASE(GameObject);
	virtual JudgeReturn link(GameCommand gcmd, EventPtr& out_event, GameObjectJudge& judge);
	virtual LiveCode customPaint(LiveCode father, const BlockPos& pos, int dotOrder);
	// DO NOT call paint/repaint/replace/... on the same object in this method
	virtual void afterPaint(LiveCode obj);
	// return true will change [GameLiveObject]._face later automatically (which means succeed) TODO
	virtual bool onFaceChange(LiveObjPtr obj, BlockPos::Direction oldface, BlockPos::Direction newface);

	virtual ~GameObject() {
	}
};

class GameTranslator {
private:
    std::function<GameCommand(float*) > convert = [](float* key) {
        return GameCommand::emptyCmd;
    }; // use anonymous function?
public:

    GameTranslator() {
    }
    GameTranslator(std::function<GameCommand(float*)> &method)
    : convert(method) {        
    }
    
    static TransPtr create(BaseCode code, std::function<GameCommand(float*)> &method, vector<TransPtr> *container = nullptr);
    template<class TranslatorType>
    static TransPtr create(BaseCode code, vector<TransPtr> *container = nullptr) {
        return T_create<TranslatorType>(code, container);
    }

	SHCP_BASE(GameTranslator);
    virtual GameCommand translate(float* arrOfKeys);

    virtual ~GameTranslator() {
    }
};

class GameLinker {
private:
    std::function<LinkerReturn(GameCommand) > convert = [](GameCommand gcmd) {
        return LinkerReturn();
    };
    
public:
    GameLinker() {
    }
    GameLinker(std::function<LinkerReturn(GameCommand)> &method)
    : convert(method) {
    }
    
    static LinkerPtr create(BaseCode code, std::function<LinkerReturn(GameCommand)> &method, vector<LinkerPtr> *container = nullptr);
    template<class LinkerType>
    static LinkerPtr create(BaseCode code, vector<LinkerPtr> *container = nullptr) {
        return T_create<LinkerType, LinkerPtr>(code, container);
    }
    
	SHCP_BASE(GameLinker);
	virtual LinkerReturn link(GameCommand gcmd, GameObjectJudge& judge);

    virtual ~GameLinker() {
    }
};

class GameEvent {
private:
public:
    GameEvent() {
    }
	
	template<class EventType>
	static EventPtr create(BaseCode code, vector<EventPtr> *container = nullptr) {
		return T_create<EventType, EventPtr>(code, container);
	}

	SHCP_BASE(GameEvent);
    virtual bool start(LiveObjPtr obj);

    virtual ~GameEvent() {
    }
};

class GamePlant {
private:
	const int WITHERED_INDEX = 0;
	const int DEFALUT_STATUE_COUNT = 5;
	BaseCode code = -1; // PlantCode
	string name;
	int statueCount;
	// 用combStatue来储存不同的状态所对应的可绘制物品，其中把枯萎的状态放在第一个
	ObjWeak statueObj;
	// 状态转移判定
	vector<int> waterNeeded;
	vector<SeasonType> season;
	virtual inline bool isWithered(int water, int sun, SeasonType currentSeason) {
		for (auto sea : this->season) {
			if (sea == currentSeason)
				return false;
		}
		return true;
	};
	virtual inline int levelUpRule(int water, int sun, SeasonType currentSeason) {
		for (int index = 1; index < (int)waterNeeded.size(); index++) {
			if (waterNeeded[index] > water)
				return index - 1 < 0 ? 0 : index - 1;
		}
		return waterNeeded.size() - 1;
	};
public:
	GamePlant() {};
	GamePlant(BaseCode code, const string& name, int statueCount, ObjWeak statueObj, const vector<int>& waterNeeded, const vector<SeasonType>& season)
	: code(code), name(name), statueCount(statueCount), statueObj(statueObj), waterNeeded(waterNeeded), season(season) {}
	static PlantPtr create(BaseCode code, const string& name, int statueCount, ObjWeak statueObj, const vector<int>& waterNeeded, const vector<SeasonType>& season, vector<PlantPtr>* container = nullptr) {
		PlantPtr pt(new GamePlant(code, name, statueCount, statueObj, waterNeeded, season));
		T_push(container, pt, code);
		return pt;
	}

	virtual inline ObjPtr getDefaultStatue() {
		ObjPtr comb = this->statueObj.lock();
		if (comb != nullptr && comb->children().size() > 1)
			return comb->children()[WITHERED_INDEX + 1].lock();
		else
			return nullptr;
	}
	virtual inline int levelUp(int water, int sun, SeasonType currentSeason) {
		if (isWithered(water, sun, currentSeason))
			return WITHERED_INDEX;
		return levelUpRule(water, sun, currentSeason);
	}

	// 这样子的话就算要追加品质系统也就是一个数组的事情了
};

class GameHuman {
public:
	BaseCode code = -1; // HumanCode
	string name;
	int fullEnergy = 100;
	// 喜好，好感，对话，人物的一些特定事件比如购物菜单等等
public:
	GameHuman(){}
	GameHuman(BaseCode code, const string& name, int fullEnergy) : code(code), name(name), fullEnergy(fullEnergy) {}
	static HumanPtr create(BaseCode code, const string& name, int fullEnergy, vector<HumanPtr>* container = nullptr) {
		HumanPtr pt(new GameHuman(code, name, fullEnergy));
		T_push(container, pt, code);
		return pt;
	}
};

class GameBase {
private:
    std::vector<ObjPtr> stuffData;
    std::vector<ObjPtr> sceneData;
    std::vector<TransPtr> transData;
    std::vector<UIPtr> UIData;
	std::vector<LinkerPtr> linkerData;
	std::vector<EventPtr> eventData;
	std::vector<PlantPtr> plantData;
	std::vector<HumanPtr> humanData;
public:
	static const float KID_MOVE_SPEED_IN_BIG_BLOCKS;
	static const BlockType KID_STEP;
	static const float KID_RUN_COMPARED_TO_WALK;
	static const int DETECT_SPLIT;
	static const GameCommand DEFAULT_COMMAND;
	static const float USE_TOOL_TIME;

    void init();
	static const int WALK = 1, RUN = 2, OTHERCMD = 0;
	// return 1:walk, 2:run, 0:others
	static int cmdWalkOrRun(GameCommand cmd);
	//static GameCommand toolToCmd(BaseCode tool);
	static PlantCode stuffToPlant(BaseCode plantStuff);

    ObjPtr getStuff(BaseCode code);
    ObjPtr getScene(BaseCode code);
    TransPtr getTranslator(BaseCode code);
    UIPtr getUI(BaseCode code);
	EventPtr getEvent(BaseCode code);
	LinkerPtr getLinker(BaseCode code);
	PlantPtr getPlant(BaseCode code);
	HumanPtr getHuman(BaseCode code);

	string getStuffCSB(BaseCode code);
};
