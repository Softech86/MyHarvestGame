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
	// auto background = cocos2d::Sprite::create("tollgateBG.jpg");
	auto background = cocos2d::Node::create();
	if (!background)
		return nullptr;
	return background;
}

bool GamePaint::nodeDisplay(LiveCode needDisplayed) {
	needDisplayed->setPosition(100, 100);
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

LiveCode GamePaint::objMove(LiveCode object, const PxPos& oldpos, const PxPos& newpos, MoveType type, float timeSec) {
	if (timeSec == 0) {
		object->setPosition(newpos.toVec2());
	}
	else
		object->runAction(cocos2d::MoveTo::create(timeSec, newpos.toVec2()));

	return object;
}

LiveCode GamePaint::objRotate(LiveCode object, float olddegree, float newdegree, float timeSec) {
	auto setArchorToCenter = [](LiveCode object) {
		for (auto obj : object->getChildren()) 
			for (auto o : obj->getChildren())
				o->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	};
	auto setArchorToZero = [](LiveCode object) {
		cocos2d::log(("object_begin" + cocos2d::Value(object->getPosition().x).asString() + cocos2d::Value(object->getPosition().y).asString()).c_str());
		for (auto obj : object->getChildren()) {
			cocos2d::log(("obj_begin" + cocos2d::Value(obj->getPosition().x).asString() + cocos2d::Value(obj->getPosition().y).asString()).c_str());
			for (auto o : obj->getChildren()) {
				cocos2d::log(("o_begin" + cocos2d::Value(o->getPosition().x).asString() + cocos2d::Value(o->getPosition().y).asString()).c_str());
				o->setAnchorPoint(cocos2d::Vec2::ZERO);
				cocos2d::log(("o_end" + cocos2d::Value(o->getPosition().x).asString() + cocos2d::Value(o->getPosition().y).asString()).c_str());
			}
			cocos2d::log(("obj_end" + cocos2d::Value(obj->getPosition().x).asString() + cocos2d::Value(obj->getPosition().y).asString()).c_str());
		}
		cocos2d::log(("object_end" + cocos2d::Value(object->getPosition().x).asString() + cocos2d::Value(object->getPosition().y).asString()).c_str());

	};

	if (timeSec == 0) {
		object->setRotation(newdegree);
	}
	else
		object->runAction(
			cocos2d::Sequence::create(
				cocos2d::CallFunc::create(std::bind(setArchorToCenter, object)),
				cocos2d::RotateTo::create(timeSec, newdegree),
				cocos2d::CallFunc::create(std::bind(setArchorToZero, object)),
				NULL
			)
		);
	return object;
}

LiveCode GamePaint::objAlpha(LiveCode object, float oldalpha, float newalpha, float timeSec) {
	if (newalpha <= 1)
		newalpha *= 255;

	object->setOpacity(0);

	if (timeSec == 0) {
		object->setOpacity((int)newalpha);
	}
	else
		object->runAction(cocos2d::FadeTo::create(timeSec, (int)newalpha));

	return object;
}

void GamePaint::objRemove(LiveCode object) {
	//注意：对象仍然存在，只是不可描述了而已
	object->setVisible(false);
}
