#include "GameUI.h"
#include "GameBase.h"

GameUI GameUI::origin;

LiveCode GameUI::start() {
    return nullptr;
}

#if defined(LINUX) || defined(__APPLE__) || defined(APPLE)
JudgeReturn GameUI::action(__attribute__((unused)) bool* keyarray) {
#endif
#ifdef WIN32
JudgeReturn GameUI::action(bool* keyarray) {
	UNREFERENCED_PARAMETER(keyarray);
#endif
    return JudgeReturn::judgeEnd;
}

string GameUI::save() {
    return "test";
}

void GameUI::stop() {
    
}

LiveCode StartPageUI::start() {
    LiveCode result = nullptr; 
    // TODO
    // add some code here
    // and assign result to the root node pointer of your paintings
    return result;
}

JudgeReturn StartPageUI::action(bool* keyarray) {
    GameCommand gcmd = this->control()->translate(keyarray);
    // TODO some code
    return JudgeReturn::judgeEnd;
}

GameCommand BasicMenuTranslator::translate(bool* keyarray) {
    if(keyarray[GameKeyPress::buttonStart]) {
        //TODO
    }
    return GameCommand::emptyCmd;
}
