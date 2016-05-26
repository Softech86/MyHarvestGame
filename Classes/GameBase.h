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
friend class GameBase;
public:

    enum BigType {
		empty,
		background, ground,
		building, furniture, stuff, plant, animal, npc, kid,
		weather, bubble,
		combStatue,
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

    JudgeReturn _defaultJudge = JudgeReturn::judgeNextObject;
    TransPtr _translator = nullptr;
    LinkerPtr _link = nullptr;
    ValuePtr _value = nullptr;

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

    
    
    BigType& type() {
        return this->_type;
    }

    BaseCode& code() {
        return this->_code;
    }

    string& name() {
        return this->_name;
    }

    string& description() {
        return this->_description;
    }

    BlockPos& size() {
        return this->_size;
    }

    BlockPos& padding() {
        return this->_position;
    }

    BlockPos& center() {
        return this->_anchor;
    }

    string& picture() {
        return this->_picture;
    }

    vector<ObjWeak>& children() {
        return this->_children;
    }

    vector<BlockPos>& childrenPos() {
        return this->_childrenPos;
    }
    
    WalkType& walktype() {
        return this->_walktype;
    }

    GameAlpha& walkBMP() {
        return this->_alphaWalkableBMP;
    }

    vector<JumpData>& jumpInfo() {
        return this->_jumpInfo;
    }

    LinkerPtr& linker() {
        return this->_link;
    }

	void setChildrenLinker(LinkerPtr lin) {
		for (auto &obj : this->children()) {
			obj.lock()->linker() = lin;
		}
	}

    TransPtr& translator() {
        return this->_translator;
    }

    ValuePtr& value() {
        return this->_value;
    }
    
	string& littlePicture() {
		return this->_littlePicture;
	}

	string* facingPicture() {
		return this->_facingPicture;
	}

    void setCustomPaint(bool custom) {
        this->_customPaint = custom;
    }
    
    bool isCustomPaint() {
        return this->_customPaint;
    }

	bool isCustomTranslate() {
		return (bool)(this->_translator);
	}

	GameCommand translate(float* arrOfKeys);

	SHCP_BASE(GameObject);
    virtual JudgeReturn link(GameCommand gcmd, EventPtr& out_event);
	virtual LiveCode customPaint(LiveCode father, const BlockPos& pos, int dotOrder);
	// DO NOT call paint/repaint/replace/... on the same object in this method
    virtual void afterPaint(LiveCode obj);
	// return true will change [GameLiveObject]._face later automatically (which means succeed) TODO
	virtual bool onFaceChange(BlockPos::Direction oldface, BlockPos::Direction newface);

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
    virtual LinkerReturn link(GameCommand);

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

class GameBase {
private:
    std::vector<ObjPtr> stuffData;
    std::vector<ObjPtr> sceneData;
    std::vector<TransPtr> transData;
    std::vector<UIPtr> UIData;
	std::vector<LinkerPtr> linkerData;
	std::vector<EventPtr> eventData;
public:
	static const float KID_MOVE_SPEED_IN_BIG_BLOCKS;
	static const BlockType KID_STEP;
	static const float KID_RUN_COMPARED_TO_WALK;
	static const int DETECT_SPLIT;
	static const GameCommand DEFAULT_COMMAND;

    void init();
	static const int WALK = 1, RUN = 2, OTHERCMD = 0;
	// return 1:walk, 2:run, 0:others
	static int cmdWalkOrRun(GameCommand cmd);
    ObjPtr getStuff(BaseCode code);
    ObjPtr getScene(BaseCode code);
    TransPtr getTranslator(BaseCode code);
    UIPtr getUI(BaseCode code);
	EventPtr getEvent(BaseCode code);
	LinkerPtr getLinker(BaseCode code);

	string getStuffCSB(BaseCode code);
};
