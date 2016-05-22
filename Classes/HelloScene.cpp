#include "HelloScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "GameLive.h"
#include "GamePaint.h"
#include "GamePrincipal.h"

USING_NS_CC;

using namespace cocostudio::timeline;

Scene* Hello::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = Hello::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool Hello::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
	auto UI = cocostudio::GUIReader::getInstance()->widgetFromJsonFile("Hello/BlockUi_1.ExportJson");
	UI->setPosition(Point(0, 0));
	this->addChild(UI);

	auto title = (Text*)Helper::seekWidgetByName(UI, "title");
	title->setFontName("fonts/ARLRDBD.ttf");

	enterBtn = (Button*)Helper::seekWidgetByName(UI, "enterBtn");
	enterBtn->setTitleFontName("fonts/Marker Felt.ttf");
	enterBtn->addTouchEventListener(this, toucheventselector(Hello::enterBtnOper));

	exitBtn = (Button*)Helper::seekWidgetByName(UI, "exitBtn");
	exitBtn->setTitleFontName("fonts/Marker Felt.ttf");
	exitBtn->addTouchEventListener(this, toucheventselector(Hello::exitBtnOper));

    return true;
}

void Hello::enterBtnOper(Ref*, TouchEventType type) {
	switch (type) {
	case TouchEventType::TOUCH_EVENT_BEGAN:
		enterBtn->setTitleColor(Color3B(255, 255, 255));
		break;
	case TouchEventType::TOUCH_EVENT_MOVED:
		if (!enterBtn->isHighlighted())
			enterBtn->setTitleColor(Color3B(0, 0, 0));
		else
			enterBtn->setTitleColor(Color3B(255, 255, 255));
		break;
	case TouchEventType::TOUCH_EVENT_ENDED:
		GamePrincipal::getLive().api_eventStart(startGameEventCode, nullptr);
		break;
	case TouchEventType::TOUCH_EVENT_CANCELED:
		enterBtn->setTitleColor(Color3B(0, 0, 0));
		break;
	}
}

void Hello::exitBtnOper(Ref*, TouchEventType type) {
	switch (type) {
	case TouchEventType::TOUCH_EVENT_BEGAN:
		exitBtn->setTitleColor(Color3B(255, 255, 255));
		break;
	case TouchEventType::TOUCH_EVENT_MOVED:
		if (!exitBtn->isHighlighted())
			exitBtn->setTitleColor(Color3B(0, 0, 0));
		else
			exitBtn->setTitleColor(Color3B(255, 255, 255));
		break;
	case TouchEventType::TOUCH_EVENT_ENDED:
		Director::getInstance()->end();
		break;
	case TouchEventType::TOUCH_EVENT_CANCELED:
		exitBtn->setTitleColor(Color3B(0, 0, 0));
		break;
	}
}