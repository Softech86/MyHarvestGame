#pragma once
#include <chrono>
#include "ClassHeader.h"

class GamePaint {
public:
    cocos2d::Scene* mainsc;
	decltype(std::chrono::system_clock::now()) gameStartTime;
	// 似乎斜坐标下屏幕位置的移动的计算还是很不正常，尤其涉及那人行进时间投射后的演算，还有地图是不是要维持方形，什么的挺麻烦的。
	// 所以暂时先维持直角吧
	PxPos dx = PxPos(1, 0), dy = PxPos(0, 1);
	PxPos windowSize;
private:
public:
    void init();
	// get the time elapsed from when the game started in seconds
	float clock();
    
	PxPos mix(const PxPos& input);
	PxPos unmix(const PxPos& input);

    LiveCode nodeNew();
    bool nodeDisplay(LiveCode scene);
    void nodeRemove(LiveCode scene);
    void nodeRemoveAllChildren(LiveCode node);
    
    LiveCode objAddToObj(LiveCode parent, const string& picture, const PxPos& pos, float scale = 1, float alpha = 1);
    LiveCode objMove(LiveCode object, const PxPos& newpos, MoveType type, float timeSec, float delaySec = 0);
    LiveCode objRotate(LiveCode object, float olddegree, float newdegree, float timeSec);
    LiveCode objAlpha(LiveCode object, float oldalpha, float newalpha, float timeSec);
    void objRemove(LiveCode object, LiveCode parent);
    
    void objZOrder(LiveCode object, float ZOrder);
	void objZOrder(LiveCode object, float oldOrder, float newOrder, float timeSec, float delaySec);

	long long getCurrentTime();
};
