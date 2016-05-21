#include "GameUI.h"
#include "GameBase.h"
#include "GameLive.h"
#include "GamePrincipal.h"
#include "HelloScene.h"

GameUI GameUI::origin;

LiveCode GameUI::start() {
    return nullptr;
}

#if defined(LINUX) || defined(__APPLE__) || defined(APPLE)
JudgeReturn GameUI::action(__attribute__((unused)) float* keyarray) {
#endif
#ifdef WIN32
JudgeReturn GameUI::action(float* keyarray) {
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
	result = Hello::create();
    // TODO
    // add some code here
    // and assign result to the root node pointer of your paintings
    return result;
}

JudgeReturn StartPageUI::action(float* keyarray) {
    GameCommand gcmd = this->control()->translate(keyarray);

    // TODO some code
    return JudgeReturn::judgeEnd;
}

GameCommand BasicMenuTranslator::translate(float* keyarray) {
    if(keyarray[GameKeyPress::buttonStart]) {
        //TODO
    }
    return GameCommand::emptyCmd;
}

GameCommand BasicMoveTranslator::translate(float* keyarray) {
	return GameCommand::emptyCmd;
}

bool StartGameEvent::start(LiveObjPtr obj) {
	return true;
}