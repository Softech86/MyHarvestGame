#include "GameNPC.h"

LinkerReturn SeedSellerLinker::link(GameCommand gcmd, GameObjectJudge& judge) {
	LinkerReturn result;
	if (gcmd == GameCommand::pickNLook) {
		TimeFunc sch = []() {
			LIVE.api_startStory(seedSellerStoryCode);
			return 0;
		};
		int out_return;
		LIVE.api_delegateActionTimeCompare(judge.getHumanPtr(), 0.3f, sch, out_return, "Talk to seed seller");
		result.judge = judgeEnd;
	}
	return result;
}