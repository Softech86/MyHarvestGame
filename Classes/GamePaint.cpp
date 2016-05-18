#include "GamePaint.h"
#include "cocostudio/CocoStudio.h"

using cocos2d::Node;
using cocos2d::Scene;

Scene* GamePaint::mainsc = nullptr;

void GamePaint::init() {
	auto scene = cocos2d::Scene::create();
	GamePaint::mainsc = scene;
	cocos2d::Director::getInstance()->runWithScene(scene);
}

LiveCode GamePaint::nodeNew() {
    //TODO
	//auto background = cocos2d::Sprite::create("tollgateBG.jpg");
	auto background = cocos2d::Node::create();
	//background->setAnchorPoint(cocos2d::Vec2(0, 0));
	if (!background)
		return nullptr;
	return background;
}

bool GamePaint::nodeDisplay(LiveCode needDisplayed) {
	//展示参数中的节点，直接挂到主场景下面
	GamePaint::mainsc->addChild(needDisplayed);
	// Set the node position here
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
		node->setOpacity(alpha);
	}
	parent->addChild(node);
	return node;
}

LiveCode GamePaint::objMove(LiveCode object, const PxPos& oldpos, const PxPos& newpos, MoveType type, float timeSec){
    //TODO
    return nullptr;
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
