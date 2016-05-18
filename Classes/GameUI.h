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
    CodeType _code = -1;
    string _name;
    TransPtr _translator = nullptr;

public:

    GameUI() {
    }

    GameUI(CodeType code,
            const string& name,
            TransPtr translator) :
    _code(code),
    _name(name),
    _translator(translator) {
    }
    
    template<class UIClass> 
    static UIPtr create(CodeType code, const string& name, TransPtr translator, vector<UIPtr> *container = nullptr) {
        UIPtr pt(new UIClass());
        pt->_code = code;
        pt->_name = name;
        pt->_translator = translator;
        T_push(container, pt, code);
        return pt;
    }
    template<class UIClass> 
    static UIPtr create(CodeType code, const string& name, CodeType translator, vector<UIPtr> *container = nullptr) {
        return create<UIClass>(code, name, GamePrincipal::getBase().getTranslator(translator), container);
    }

    virtual LiveCode start();
    virtual string save();
    virtual JudgeReturn action(bool* keyarray);
    virtual void stop();

    UIType& type() {
        return this->_type;
    }

    CodeType& code() {
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
    LiveCode start() override;
    JudgeReturn action(bool* keyarray) override;
};

class BasicMenuTranslator : public GameTranslator {
public:
    GameCommand translate(bool* keyarray) override;
};

class BasicMoveTranslator : public GameTranslator {
public:
    GameCommand translate(bool* keyarray) override;
};