#include "TalkNode.h"
#include "cocostudio/CocoStudio.h"

using namespace cocos2d;
using namespace cocostudio::timeline;

TalkNode* TalkNode::create(const std::string& personName, const std::string& personCsb, const std::string& talkText)
{
	TalkNode *pRet = new(std::nothrow) TalkNode();
	if (pRet && pRet->init(personName, personCsb, talkText))
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

// on "init" you need to initialize your instance
bool TalkNode::init(const std::string& personName, const std::string& personCsb, const std::string& talkText)
{
	if (!Layer::init())
	{
		return false;
	}

	auto UI = CSLoader::createNode("TalkNode.csb");
	UI->setAnchorPoint(Point::ZERO);
	UI->setPosition(Point::ZERO);
	this->addChild(UI);

	if (personCsb != "") {
		auto person = CSLoader::createNode(personCsb);
		person->setAnchorPoint(Point(1, 0));
		person->setPosition(Point(960, 200)); //TODO: change the magic number to var.
		UI->addChild(person);
	}

	auto nameBoard = static_cast<Button*>(UI->getChildByName("nameBoard"));
	nameBoard->setContentSize(Size(20 + 23 * personName.length() + 40, nameBoard->getContentSize().height));
	_name = static_cast<Text*>(UI->getChildByName("name"));
	_line1 = static_cast<Text*>(UI->getChildByName("line1"));
	_line2 = static_cast<Text*>(UI->getChildByName("line2"));
	_line3 = static_cast<Text*>(UI->getChildByName("line3"));

	_text = {""};
	bool firstChinese = false;
	for (int i = 0; i < talkText.length(); ++i) {
		_text.back().push_back(talkText[i]);
		if (talkText[i] < 0)
			firstChinese = !firstChinese;
		if (_text.back().length() >= 40 && !firstChinese)
			_text.push_back("");
	}

	_name->setText(personName);
	_line1->setString(_text.size() >= 1 ? _text[0] : "");
	
	_line2->setText(_text.size() >= 2 ? _text[1] : "");
	_line3->setText(_text.size() >= 3 ? _text[2] : "");
	_displayTo = 2;

	return true;
}

bool TalkNode::nextLine() {
	if (_displayTo + 1 >= _text.size())
		return false;
	_line1->setText(_line2->getString());
	_line2->setText(_line3->getString());
	_line3->setText(_text[++_displayTo]);
	return true;
}
