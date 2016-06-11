#include "GamePaint.h"
#include "GamePrincipal.h"
#include "GameLive.h"
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"

using cocos2d::Node;
using cocos2d::Scene;

PxPos GamePaint::mix(const PxPos& input){
	return dx * input.x + dy * input.y;
}

float matrix(float a11, float a12, float a21, float a22) {
	return a11 * a22 - a12 * a21;
}

PxPos GamePaint::unmix(const PxPos& input) {
	/*
	float down = matrix(dx.x, dy.x, dx.y, dy.y);
	if (down == 0) {
		cocos2d::log("那个智障把两单位向量设为平行的？");
		return PxPos::zero;
	}
	else {
		return PxPos(matrix(input.x, dy.x, input.y, dy.y) / down, matrix(dx.x, input.x, dx.y, input.y) / down);
	}*/
	return input;
}

void GamePaint::init() {
	auto scene = cocos2d::Scene::create();
	this->mainsc = scene;
	cocos2d::Director::getInstance()->runWithScene(scene);
	//auto var = cocos2d::Director::getInstance()->getOpenGLView()->getVisibleSize();
	//windowSize.x = var.width;
	//windowSize.y = var.height;
	this->gameStartTime = std::chrono::system_clock::now();
}

// get the time elapsed from when the game started in seconds
float GamePaint::clock() {
	auto end = std::chrono::system_clock::now();
	auto dur = end - this->gameStartTime;
	float msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
	return msec / 1000;
}

LiveCode GamePaint::nodeNew() {;
	auto background = cocos2d::Node::create();
	if (!background)
		return nullptr;
	return background;
}

bool GamePaint::nodeDisplay(LiveCode needDisplayed) {
	//needDisplayed->setPosition(100, 100);
	GamePaint::mainsc->addChild(needDisplayed);
    return true;
}

void GamePaint::nodeRetain(LiveCode node) {
	node->retain();
}

void GamePaint::nodeRelease(LiveCode node) {
	node->autorelease();
}

void GamePaint::nodeRemove(LiveCode needRemoved) {
	GamePaint::mainsc->removeChild(needRemoved);
}

void GamePaint::nodeRemoveAllChildren(LiveCode node) {
    node->removeAllChildren();
}

LiveCode GamePaint::objAddToObj(LiveCode parent, const string& picture, const PxPos& pos, float scale, float alpha) {
	PxPos pos2 = mix(pos);
	cocos2d::Node* node;
	if (picture == "")
		node = cocos2d::Node::create();
	else {
		node = cocos2d::CSLoader::createNode(picture);
		if (!node) {
			cocos2d::log(("objAddToObj: node null error" + picture).c_str());
			return nullptr;
		}
		node->setPosition(pos2.x, pos2.y);
		node->setScale(scale);
		node->setOpacity(255 * alpha);
	}
	parent->addChild(node);
	return node;
}

LiveCode GamePaint::objMove(LiveCode object, const PxPos& newpos, MoveType type, float timeSec, float delaySec){
	PxPos newpos2 = mix(newpos); 
	if (delaySec < WUCHAF) {
		if (timeSec < WUCHAF)
			object->setPosition(newpos.toVec2());
		else {
			//cocos2d::log("PAINT.MOVE ->[%f, %f], %f", newpos.x, newpos.y, timeSec);
			object->runAction(cocos2d::MoveTo::create(timeSec, newpos.toVec2()));
		}
		return object;
	}
	else {
		CocoFunc sch = [timeSec, object, newpos](float dt) {
			if (timeSec < WUCHAF)
				object->setPosition(newpos.toVec2());
			else
				object->runAction(cocos2d::MoveTo::create(timeSec, newpos.toVec2()));
		};
		GameLive::api_delayTime(sch, delaySec, "move" + std::to_string((int)object), object);
		return object;
	}
}

LiveCode GamePaint::objRotate(LiveCode object, float olddegree, float newdegree, float timeSec){
    //TODO
    return nullptr;
}

LiveCode GamePaint::objAlpha(LiveCode object, float oldalpha, float newalpha, float timeSec, float delaySec) {
	// BUG SEEMS
	if (object && oldalpha != newalpha) {
		if (timeSec < WUCHAF && delaySec < WUCHAF)
			object->setOpacity(newalpha * 255);
		else {
			double *timePassed = new double;
			*timePassed = 0;
			double *timeUse = new double;
			*timeUse = timeSec;
			double *delta = new double;
			*delta = newalpha - oldalpha;
			int repeat = timeSec / LIVE.api_getLoopFreq();
			auto sch = [object, timePassed, timeUse, delta, oldalpha, newalpha](float dt) {
				*timePassed += dt;
				if (*timePassed + dt >= *timeUse) {
					object->setOpacity(newalpha * 255);
					delete timePassed;
					delete timeUse;
					delete delta;
				}
				else {
					double alpha = oldalpha + *delta / *timeUse * *timePassed;
					object->setOpacity(alpha * 255);
				}
			};
			cocos2d::Director::getInstance()->getScheduler()->schedule(sch, object, LIVE.api_getLoopFreq(), repeat - 1, delaySec, false, "AlphaFade" + std::to_string((int)object));
		}
	}
	else
		cocos2d::log("%xd alpha: %f -> %f + %f + %f", (int)object, oldalpha, newalpha, timeSec, delaySec);
    return object;
}

void GamePaint::objRemove(LiveCode object, LiveCode parent) {
	parent->removeChild(object);
}

void GamePaint::objZOrder(LiveCode object, int ZOrder) {
	if (object)
		object->setLocalZOrder(ZOrder);
}


void GamePaint::objZOrder(LiveCode object, int oldOrder, int newOrder, float timeSec, float delaySec) {
	if (object && oldOrder != newOrder)
		object->setLocalZOrder(newOrder);
	// BUG SEEMS
	//if (object && oldOrder != newOrder) {
	//	if (timeSec == 0 && delaySec == 0) {
	//		object->setLocalZOrder(newOrder);
	//	}
	//	else {
	//		float timePassed = 0;
	//		float delta = newOrder - oldOrder;
	//		int repeat = timeSec / LIVE.api_getLoopFreq();
	//		auto sch = [object, &timePassed, &timeSec, &delta, &oldOrder, &newOrder](float dt) {
	//			timePassed += dt;
	//			if (timePassed + dt >= timeSec)
	//				object->setLocalZOrder(newOrder);
	//			else {
	//				float zOrder = oldOrder + delta / timeSec * timePassed;
	//				object->setLocalZOrder(zOrder);
	//			}
	//		};
	//		cocos2d::Director::getInstance()->getScheduler()->schedule(sch, object, LIVE.api_getLoopFreq(), repeat, delaySec, false, "ZOrderFade" + std::to_string((int)object));
	//	}
	//}
}
