#pragma once
#include <chrono>
#include "ClassHeader.h"

class GamePaint {
public:
    cocos2d::Scene* mainsc;
	decltype(std::chrono::system_clock::now()) gameStartTime;
public:
    void init();
	// get the time elapsed from when the game started in milliseconds
	long long clock();
    
    LiveCode nodeNew();
    bool nodeDisplay(LiveCode scene);
    void nodeRemove(LiveCode scene);
    void nodeRemoveAllChildren(LiveCode node);
    
    LiveCode objAddToObj(LiveCode parent, const string& picture, const PxPos& pos, float scale = 1, float alpha = 1);
    LiveCode objMove(LiveCode object, const PxPos& oldpos, const PxPos& newpos, MoveType type, float timeSec);
    LiveCode objRotate(LiveCode object, float olddegree, float newdegree, float timeSec);
    LiveCode objAlpha(LiveCode object, float oldalpha, float newalpha, float timeSec);
    void objRemove(LiveCode object);
    
	long long getCurrentTime();
};
