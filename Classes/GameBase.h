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
        empty, kid, npc, animal, plant, ground, stuff, combStuff, combStatue
    };

    struct JumpData {
        GameAlpha jump;
        BaseCode sceneCode = -1;
        BlockPos kidPos;
    };

    static GameObject origin;

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

    JudgeReturn _defaultJudge = JudgeReturn::judgeEnd;
    TransPtr _translator = nullptr;
    LinkerPtr _link = nullptr;
    ValuePtr _value = nullptr;

    bool _customPaint = false;
    
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

    TransPtr& translator() {
        return this->_translator;
    }

    ValuePtr& value() {
        return this->_value;
    }
    
    void setCustomPaint(bool custom) {
        this->_customPaint = custom;
    }
    
    bool isCustomPaint() {
        return this->_customPaint;
    }

    virtual JudgeReturn link(GameCommand gcmd, EventPtr& out_event);
    virtual LiveCode customPaint(LiveCode father, const BlockPos& pos);
    virtual void afterPaint(LiveCode obj);
    
    virtual ~GameObject() {
    }
};

class GameTranslator {
private:
    std::function<GameCommand(bool*) > convert = [](bool* key) {
        return GameCommand::emptyCmd;
    }; // use anonymous function?
public:

    GameTranslator() {
    }
    GameTranslator(std::function<GameCommand(bool*)> &method)
    : convert(method) {        
    }
    
    static TransPtr create(BaseCode code, std::function<GameCommand(bool*)> &method, vector<TransPtr> *container = nullptr);
    template<class TranslatorType>
    static TransPtr create(BaseCode code, vector<TransPtr> *container = nullptr) {
        return T_create<TranslatorType>(code, container);
    }
    
    virtual GameCommand translate(bool* arrOfKeys);

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
    
    virtual LinkerReturn link(GameCommand);

    virtual ~GameLinker() {
    }
};

class GameEvent {
private:
    // TODO
public:

    GameEvent() {
    }

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
public:
	// 2 big blocks per sec
	float kidMoveSpeed = 2 * BIG_TO_SMALL;
    void init();
    ObjPtr getStuff(BaseCode code);
    ObjPtr getScene(BaseCode code);
    TransPtr getTranslator(BaseCode code);
    UIPtr getUI(BaseCode code);
};
