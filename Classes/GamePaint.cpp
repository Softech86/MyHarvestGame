#include "GamePaint.h"
#include "GamePrincipal.h"
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
	auto background = cocos2d::Node::create();
	if (!background)
		return nullptr;
	return background;
}

bool GamePaint::nodeDisplay(LiveCode needDisplayed) {
#ifdef DEBUG2
	auto pai = GamePrincipal::getPaint();

	auto root = pai.nodeNew();
	auto surrounding = pai.objAddToObj(root, "", PxPos::zero);
	auto kid = pai.objAddToObj(root, "", PxPos::zero);
	auto box = cocos2d::CSLoader::createNode("soilHoed.csb");
	surrounding->addChild(box);
	mainsc->addChild(root);
#else
	// needDisplayed->setPosition(100, 100);
	GamePaint::mainsc->addChild(needDisplayed);
	// Set the node position here
	// DEBUG
#endif
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
	// bug seems to be here
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
    //TODO

	if (timeSec == 0) {
		object->setPosition(newpos.toVec2());
	}
	else
		object->runAction(cocos2d::MoveTo::create(timeSec, newpos.toVec2()));

    return object;
}

LiveCode GamePaint::objRotate(LiveCode object, float olddegree, float newdegree, float timeSec){
    //TODO
	if (timeSec == 0) {
		object->setRotation(newdegree);
	}
	else
		object->runAction(cocos2d::RotateTo::create(timeSec, newdegree));

	return object;
}

LiveCode GamePaint::objAlpha(LiveCode object, float oldalpha, float newalpha, float timeSec) {
    //TODO
	
	if (newalpha <= 1)
		newalpha *= 255;
	
	if (timeSec == 0) {
		object->setOpacity(newalpha);
	}
	else
		object->runAction(cocos2d::FadeTo::create(timeSec, newalpha));

	return object;
}

void GamePaint::objRemove(LiveCode object) {
    //注意：对象仍然存在，只是不可描述了而已
	object->setVisible(false);
}
