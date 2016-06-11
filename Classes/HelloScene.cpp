#include "HelloScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "GameLive.h"
#include "GamePaint.h"
#include "GamePrincipal.h"

USING_NS_CC;

using namespace cocostudio::timeline;
using namespace cocostudio;
using namespace cocos2d::ui;

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
		setHighlight(0);
		break;
	case TouchEventType::TOUCH_EVENT_MOVED:
		if (!enterBtn->isHighlighted())
			unsetHighlight();
		else
			setHighlight(0);
		break;
	case TouchEventType::TOUCH_EVENT_ENDED:
		LIVE.api_eventStart(startGameEventCode, nullptr);
		break;
	case TouchEventType::TOUCH_EVENT_CANCELED:
		unsetHighlight();
		break;
	}
}

void Hello::exitBtnOper(Ref*, TouchEventType type) {
	switch (type) {
	case TouchEventType::TOUCH_EVENT_BEGAN:
		setHighlight(1);
		break;
	case TouchEventType::TOUCH_EVENT_MOVED:
		if (!exitBtn->isHighlighted())
			unsetHighlight();
		else
			setHighlight(1);
		break;
	case TouchEventType::TOUCH_EVENT_ENDED:
		Director::getInstance()->end();
		break;
	case TouchEventType::TOUCH_EVENT_CANCELED:
		unsetHighlight();
		break;
	}
}

void Hello::setHighlight(int selected) {
	if (this == nullptr)
		return;
	unsetHighlight();
	if (selected == 0)
		enterBtn->setTitleColor(Color3B(255, 255, 255));
	else if (selected == 1)
		exitBtn->setTitleColor(Color3B(255, 255, 255));
}

void Hello::unsetHighlight() {
	if (this == nullptr)
		return;
	enterBtn->setTitleColor(Color3B(0, 0, 0));
	exitBtn->setTitleColor(Color3B(0, 0, 0));
}

void Hello::comeOn(LiveCode node, GameCommand cmd) {
	if (node == nullptr)
		return;
	if (cmd == emptyCmd)
		return;
	if (cmd == confirm) {
		switch (selectedItem)
		{
		case 0:
			LIVE.api_eventStart(startGameEventCode, nullptr);
			break;
		case 1:
			Director::getInstance()->end();
			break;
		default:
			break;
		}
	}

	if (cmd == selectUp && selectedItem > 0)
		this->selectedItem--;
	else if (cmd == selectDown && selectedItem < 1)
		this->selectedItem++;

	switch (selectedItem)
	{
	case 0:
		setHighlight(0);
		break;
	case 1:
		setHighlight(1);
		break;
	default:
		break;
	}
}