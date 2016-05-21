#include "GamePaint.h"
#include "GamePrincipal.h"
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"

using cocos2d::Node;
using cocos2d::Scene;

void GamePaint::init() {
	auto scene = cocos2d::Scene::create();
	this->mainsc = scene;
	cocos2d::Director::getInstance()->runWithScene(scene);
	this->gameStartTime = std::chrono::system_clock::now();
}

// get the time elapsed from when the game started in milliseconds
long long GamePaint::clock() {
	auto end = std::chrono::system_clock::now();
	auto dur = end - this->gameStartTime;
	auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
	return msec;
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
	cocos2d::Node* node;
	if (picture == "")
		node = cocos2d::Node::create();
	else {
		node = cocos2d::CSLoader::createNode(picture);
		if (!node)
			return nullptr;
		node->setPosition(pos.x, pos.y);
		node->setScale(scale);
		node->setOpacity(255 * alpha);
	}
	parent->addChild(node);
	return node;
}

LiveCode GamePaint::objMove(LiveCode object, const PxPos& oldpos, const PxPos& newpos, MoveType type, float timeSec){
	// TODO
	if (timeSec == 0) {
		object->setPosition(newpos.toVec2());
	}
	else
		object->runAction(cocos2d::MoveTo::create(timeSec, newpos.toVec2()));

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

void GamePaint::objRemove(LiveCode object) {
    //TODO
}
