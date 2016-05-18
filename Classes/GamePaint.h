#pragma once
#include "ClassHeader.h"

class GamePaint {
public:
    
    LiveCode nodeNew();
    bool nodeDisplay(LiveCode scene);
    void nodeClear(LiveCode scene);
    
    LiveCode objAddToObj(LiveCode parent, const string& picture, const PxPos& pos, float scale = 1, float alpha = 1);
    LiveCode objMove(LiveCode object, const PxPos& oldpos, const PxPos& newpos, MoveType type, float timeSec);
    LiveCode objRotate(LiveCode object, float olddegree, float newdegree, float timeSec);
    LiveCode objAlpha(LiveCode object, float oldalpha, float newalpha, float timeSec);
    void objErase(LiveCode object);
    
    
private:

};
