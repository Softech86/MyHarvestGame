#ifndef __HELLO_SCENE_H__
#define __HELLO_SCENE_H__

#include "cocos2d.h"

#include "editor-support\cocostudio\CCSGUIReader.h"
#include "ui/CocosGUI.h"

using namespace cocos2d::ui;
using namespace cocostudio;

class Hello : public cocos2d::Layer
{
private:
	Button *enterBtn, *exitBtn;
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();

    // implement the "static create()" method manually
    CREATE_FUNC(Hello);

	void enterBtnOper(Ref*, TouchEventType type);
	void exitBtnOper(Ref*, TouchEventType type);
};

#endif // __HELLOWORLD_SCENE_H__
