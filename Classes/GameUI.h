#pragma once

#include "ClassHeader.h"
#include "GameBase.h"
#include "GamePrincipal.h"

// it still needs development
class GameUI {
public:
    enum UIType {
        empty, up, down
    };
    static GameUI origin;

private:
    UIType _type = up;
    BaseCode _code = -1;
    string _name;
    TransPtr _translator = nullptr;

public:

    GameUI() {
    }

    GameUI(BaseCode code,
            const string& name,
            TransPtr translator) :
    _code(code),
    _name(name),
    _translator(translator) {
    }
    
    template<class UIClass> 
    static UIPtr create(BaseCode code, const string& name, TransPtr translator, vector<UIPtr> *container = nullptr) {
        UIPtr pt(new UIClass());
        pt->_code = code;
        pt->_name = name;
        pt->_translator = translator;
        T_push(container, pt, code);
        return pt;
    }
    template<class UIClass> 
    static UIPtr create(BaseCode code, const string& name, BaseCode translator, vector<UIPtr> *container = nullptr) {
        return create<UIClass>(code, name, BASE.getTranslator(translator), container);
    }

	
	SHCP_BASE(GameUI);
    virtual LiveCode start();
    virtual string save();
	virtual JudgeReturn action(LiveCode node, float* keyarray, GameObjectJudge& judge);
    virtual void stop();

    UIType& type() {
        return this->_type;
    }

    BaseCode& code() {
        return this->_code;
    }

    string& name() {
        return this->_name;
    }

    TransPtr& control() {
        return this->_translator;
    }

    virtual ~GameUI() {
    }
};

class StartPageUI : public GameUI {
public:
	SHCP_OVERRIDE(GameUI, StartPageUI);
    virtual LiveCode start() override;
	virtual JudgeReturn action(LiveCode node, float* keyarray, GameObjectJudge& judge) override;
};

class ToolUI : public GameUI {
public:
	void toolPaint(LiveCode paper);
	SHCP_OVERRIDE(GameUI, ToolUI);
	virtual LiveCode start() override;
	virtual JudgeReturn action(LiveCode node, float* keyarray, GameObjectJudge& judge) override;
};

class KidMoveUI : public GameUI {
public:
	SHCP_OVERRIDE(GameUI, KidMoveUI);
	virtual LiveCode start() override;
	virtual JudgeReturn action(LiveCode node, float* keyarray, GameObjectJudge& judge) override;
};

class TalkUI : public GameUI {
public:
	SHCP_OVERRIDE(GameUI, TalkUI);
	virtual LiveCode start() override;
	virtual JudgeReturn action(LiveCode node, float* keyarray, GameObjectJudge& judge) override;
};

class BasicMenuTranslator : public GameTranslator {
public:
	SHCP_OVERRIDE(GameTranslator, BasicMenuTranslator);
	virtual GameCommand translate(float* keyarray) override;
};

class BasicMoveTranslator : public GameTranslator {
public:
	SHCP_OVERRIDE(GameTranslator, BasicMoveTranslator);
	virtual GameCommand translate(float* keyarray) override;
};

class BasicObjectTranslator : public GameTranslator {
public:
	SHCP_OVERRIDE(GameTranslator, BasicObjectTranslator);
	virtual GameCommand translate(float* keyarray) override;
};

class ToolTranslator : public GameTranslator {
public:
	SHCP_OVERRIDE(GameTranslator, ToolTranslator);
	virtual GameCommand translate(float* keyarray) override;
};
//
//class HandTranslator : public GameTranslator {
//public:
//	SHCP_OVERRIDE(GameTranslator, HandTranslator);
//	virtual GameCommand translate(float* keyarray) override;
//};

class SoilLinker : public GameLinker{
public:
	SHCP_OVERRIDE(GameLinker, SoilLinker);
	virtual LinkerReturn link(GameCommand, GameObjectJudge&) override;
};

class BedLinker : public GameLinker {
public:
	SHCP_OVERRIDE(GameLinker, BedLinker);
	virtual LinkerReturn link(GameCommand, GameObjectJudge&) override;
};

class PotatoLinker : public GameLinker {
public:
	SHCP_OVERRIDE(GameLinker, PotatoLinker);
	virtual LinkerReturn link(GameCommand, GameObjectJudge&) override;
};

class DefaultLinker : public GameLinker {
public:
	SHCP_OVERRIDE(GameLinker, DefaultLinker);
	virtual LinkerReturn link(GameCommand, GameObjectJudge&) override;
};

class StartGameEvent : public GameEvent {
public:
	SHCP_OVERRIDE(GameEvent, StartGameEvent);
	virtual bool start(LiveObjPtr obj) override;
};

class DayPassEvent : public GameEvent {
public:
	SHCP_OVERRIDE(GameEvent, DayPassEvent);
	virtual bool start(LiveObjPtr obj) override;
};