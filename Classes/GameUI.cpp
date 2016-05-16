#include "GameUI.h"
#include "GameBase.h"

GameUI GameUI::origin;

UIPtr GameUI::create(CodeType code, const string& name, TransPtr control, const BlockPos& pos, vector<UIPtr>* container, const string& pic) {
	UIPtr pt(new GameUI(code, name, control, pos, pic));
	if (container != nullptr){
		if ((int)container->size() <= code)
			container->resize(code + 1);
		(*container)[code] = pt;
	}
	return pt;
}

LiveCodeType GameUI::start() {
	return nullptr;
}

GameJudgeResult GameUI::action(bool* keyarray) {
	return GameJudgeResult::judgeEnd;
}