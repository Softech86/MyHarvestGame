#ifndef _TALKNODE_H_
#define _TALKNODE_H_


#include "cocos2d.h"
#include "ui/CocosGUI.h"

using namespace cocos2d::ui;

class TalkNode : public cocos2d::Layer
{
public:
	static TalkNode* create(const std::string& personName, const std::string& personCsb, const std::string& text);

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init(const std::string& personName, const std::string& personCsb, const std::string& text);

	// return true means can scroll, false means it's the end.
	bool nextLine();

private:
	Text *_name, *_line1, *_line2, *_line3;
	std::vector<std::string> _text;
	int _displayTo;
};

#endif // _TALKNode_H_
