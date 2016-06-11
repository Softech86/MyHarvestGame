#ifndef _SELECTNODE_H_
#define _SELECTNODE_H_


#include "cocos2d.h"
#include "ClassHeader.h"

#include "ui/CocosGUI.h"

using namespace cocos2d::ui;

class SelectNode : public cocos2d::Layer
{
public:
	static SelectNode* create(const std::string& personName, const std::string& content, const std::string* buttonText, int buttonCount);
	virtual bool init(const std::string& personName, const std::string& content, const std::string* buttonText, int buttonCount);
	bool nextLine();
	void selectUp();
	void selectDown();
	int getSelect() { return this->_selectNow; }

private:
	int _selectNow = -1;
	Text* _name;
	std::vector<Text*> _line;
	std::vector<Button*> _btn;
	std::vector<std::string> _text;
	int _displayTo, _size;
};

#endif // !_SELECTNODE_H_
