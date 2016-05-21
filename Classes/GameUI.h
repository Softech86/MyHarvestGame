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
        return create<UIClass>(code, name, GamePrincipal::getBase().getTranslator(translator), container);
    }

    virtual LiveCode start();
    virtual string save();
    virtual JudgeReturn action(float* keyarray);
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
    virtual LiveCode start() override;
	virtual JudgeReturn action(float* keyarray) override;
};

class BasicMenuTranslator : public GameTranslator {
public:
	virtual GameCommand translate(float* keyarray) override;
};

class BasicMoveTranslator : public GameTranslator {
public:
	virtual GameCommand translate(float* keyarray) override;
};

class StartGameEvent : public GameEvent {
public:
	virtual bool start(LiveObjPtr obj) override;
};