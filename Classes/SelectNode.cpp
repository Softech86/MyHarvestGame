#include "SelectNode.h"
#include "cocostudio/CocoStudio.h"

using namespace cocos2d;
using namespace cocostudio::timeline;

SelectNode* SelectNode::create(const std::string& personName, const std::string& content, const std::string* buttonText, int buttonCount)
{
	SelectNode *pRet = new(std::nothrow) SelectNode();
	if (pRet && pRet->init(personName, content, buttonText, buttonCount))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}

bool SelectNode::init(const std::string& personName, const std::string& content, const std::string* buttonText, int buttonCount)
{
	if (!Layer::init())
	{
		return false;
	}

	_size = buttonCount;

	if (_size > 5 || _size <= 0)
		return false;

	auto UI = CSLoader::createNode("SelectNode.csb");
	UI->setAnchorPoint(Point::ZERO);
	UI->setPosition(Point::ZERO);
	this->addChild(UI);

	auto bg = static_cast<ImageView*>(UI->getChildByName("bg"));
	bg->setPosition(Vec2(480, 345));
	bg->setContentSize(Size(270, 20 + 50 * _size));
	for (int i = 0; i < _size; ++i) {
		_btn.push_back(static_cast<Button*>(UI->getChildByName("btn" + Value(i).asString())));
		_btn[i]->setTitleText(buttonText[i]);
		_btn[i]->setPosition(Vec2(480, 320 + 25 * _size - 50 * i));

		_btn[i]->setHighlighted(false);
		_btn[i]->setTitleColor(Color3B(65, 65, 70));
		_btn[i]->setTitleFontSize(24);
		//TODO: bind action
	}
	for (int i = _size; i < 5; ++i) {
		static_cast<Button*>(UI->getChildByName("btn" + Value(i).asString()))->setVisible(false);
	}

	_text.clear();
	_text.push_back("");
	bool firstChinese = false;
	for (int i = 0; i < (int)content.length(); ++i) {
		_text.back().push_back(content[i]);
		if (content[i] < 0)
			firstChinese = !firstChinese;
		if (_text.back().length() >= 40 && !firstChinese)
			_text.push_back("");
	}

	auto nameBoard = static_cast<Button*>(UI->getChildByName("nameBoard"));
	nameBoard->setContentSize(Size(20 + 23 * personName.length() + 40, nameBoard->getContentSize().height));
	_name = static_cast<Text*>(UI->getChildByName("name"));
	_name->setString(personName);
	for (int i = 0; i < 3; ++i) {
		_line.push_back(static_cast<Text*>(UI->getChildByName("line" + Value(i).asString())));
		_line[i]->setString((int)_text.size() >= i + 1 ? _text[i] : "");
	}

	_displayTo = 2;

	return true;
}

bool SelectNode::nextLine() {
	if (_displayTo + 1 >= (int)_text.size())
		return false;
	for (int i = 0; i < 2; ++i) {
		_line[i]->setString(_line[i + 1]->getString());
	}
	_line[3]->setString(_text[++_displayTo]);
	return true;
}
void SelectNode::selectUp() {
	if (_selectNow == -1) {
		_selectNow = 0;
		_btn[_selectNow]->setHighlighted(true);
		_btn[_selectNow]->setTitleColor(Color3B::WHITE);
		_btn[_selectNow]->setTitleFontSize(30);
	}
	else if (_selectNow > 0) {
		_btn[_selectNow]->setHighlighted(false);
		_btn[_selectNow]->setTitleColor(Color3B(65, 65, 70));
		_btn[_selectNow]->setTitleFontSize(24);
		--_selectNow;
		_btn[_selectNow]->setHighlighted(true);
		_btn[_selectNow]->setTitleColor(Color3B::WHITE);
		_btn[_selectNow]->setTitleFontSize(30);
	}
}

void SelectNode::selectDown() {
	if (_selectNow == -1) {
		_selectNow = 0;
		_btn[_selectNow]->setHighlighted(true);
		_btn[_selectNow]->setTitleColor(Color3B::WHITE);
		_btn[_selectNow]->setTitleFontSize(30);
	}
	else if (_selectNow + 1 < _size) {
		_btn[_selectNow]->setHighlighted(false);
		_btn[_selectNow]->setTitleColor(Color3B(65, 65, 70));
		_btn[_selectNow]->setTitleFontSize(24);
		++_selectNow;
		_btn[_selectNow]->setHighlighted(true);
		_btn[_selectNow]->setTitleColor(Color3B::WHITE);
		_btn[_selectNow]->setTitleFontSize(30);
	}
}