#include "GamePaint.h"
#include "GamePrincipal.h"
#include "GameLive.h"
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"

using cocos2d::Node;
using cocos2d::Scene;

// 似乎斜坐标下屏幕位置的移动的计算还是很不正常，尤其涉及那人行进时间投射后的演算，还有地图是不是要维持方形，什么的挺麻烦的。
// 所以暂时先维持直角吧
PxPos GamePaint::dx = PxPos(1, 0);
PxPos GamePaint::dy = PxPos(0, 1);

PxPos GamePaint::mix(const PxPos& input){
	return dx * input.x + dy * input.y;
}

float matrix(float a11, float a12, float a21, float a22) {
	return a11 * a22 - a12 * a21;
}

PxPos GamePaint::unmix(const PxPos& input) {
	float down = matrix(dx.x, dy.x, dx.y, dy.y);
	if (down == 0) {
		cocos2d::log("那个智障把两单位向量设为平行的？");
		return PxPos::zero;
	}
	else {
		return PxPos(matrix(input.x, dy.x, input.y, dy.y) / down, matrix(dx.x, input.x, dx.y, input.y) / down);
	}
}

void GamePaint::init() {
	auto scene = cocos2d::Scene::create();
	this->mainsc = scene;
	cocos2d::Director::getInstance()->runWithScene(scene);
	this->gameStartTime = std::chrono::system_clock::now();
}

// get the time elapsed from when the game started in seconds
float GamePaint::clock() {
	auto end = std::chrono::system_clock::now();
	auto dur = end - this->gameStartTime;
	float msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
	return msec / 1000;
}

LiveCode GamePaint::nodeNew() {
    //TODO
	// auto background = cocos2d::Sprite::create("tollgateBG.jpg");
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
		if (!node)
			return nullptr;
		node->setPosition(pos2.x, pos2.y);
		node->setScale(scale);
		node->setOpacity(255 * alpha);
	}
	parent->addChild(node);
	return node;
}

LiveCode GamePaint::objMove(LiveCode object, const PxPos& newpos, MoveType type, float timeSec, float delaySec){
	PxPos newpos2 = mix(newpos);
	object->runAction(
		cocos2d::Sequence::create(
		cocos2d::DelayTime::create(delaySec),
		cocos2d::MoveTo::create(timeSec, newpos2.toVec2()),
		NULL
		)
	);
	//cocos2d::log("object move:");
	//string str = std::to_string(newpos2.x) + ", " + std::to_string(newpos2.y) + " : " + std::to_string(timeSec) + " - " +std::to_string(delaySec);
	//cocos2d::log(str.c_str());
	return object;
}

LiveCode GamePaint::objRotate(LiveCode object, float olddegree, float newdegree, float timeSec){
    //TODO
    return nullptr;
}

LiveCode GamePaint::objAlpha(LiveCode object, float oldalpha, float newalpha, float timeSec) {
    //TODO
    return nullptr;
}

void GamePaint::objRemove(LiveCode object, LiveCode parent) {
	parent->removeChild(object);
}

void GamePaint::objZOrder(LiveCode object, float ZOrder) {
	if (object)
		object->setGlobalZOrder(ZOrder);
}


void GamePaint::objZOrder(LiveCode object, float oldOrder, float newOrder, float timeSec, float delaySec) {
	if (object && oldOrder != newOrder) {
		float timePassed = 0;
		float delta = newOrder - oldOrder;
		int repeat = delta / LIVE.api_getLoopFreq();
		auto sch = [object, &timePassed, &timeSec, &delta, &oldOrder, &newOrder](float dt) {
			timePassed += dt;
			if (timePassed + dt >= timeSec)
				object->setGlobalZOrder(newOrder);
			else {
				float zOrder = oldOrder + delta / timeSec * timePassed;
				object->setGlobalZOrder(zOrder);
			}
		};
		cocos2d::Director::getInstance()->getScheduler()->schedule(sch, object, LIVE.api_getLoopFreq(), repeat, delaySec, false, "ZOrderFade");
	}
}
