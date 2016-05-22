#ifndef __HELLO_SCENE_H__
#define __HELLO_SCENE_H__

#include "cocos2d.h"
#include "editor-support\cocostudio\CCSGUIReader.h"
#include "ui/CocosGUI.h"

#include "ClassHeader.h"

class Hello : public cocos2d::Layer
{
private:
	cocos2d::ui::Button *enterBtn, *exitBtn;
	int selectedItem = 0;
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();

    // implement the "static create()" method manually
    CREATE_FUNC(Hello);

	void enterBtnOper(Ref*, cocos2d::ui::TouchEventType type);
	void exitBtnOper(Ref*, cocos2d::ui::TouchEventType type);

	void setHighlight(int selected);
	void unsetHighlight();
	void comeOn(LiveCode node, GameCommand cmd);
};

#endif // __HELLOWORLD_SCENE_H__
