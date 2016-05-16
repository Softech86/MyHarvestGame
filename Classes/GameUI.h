#pragma once

#include "ClassHeader.h"
#include "GameBase.h"

// 还是需要有待设计和提升
class GameUI {
public:
	enum UIType { empty, up, down };
	static GameUI origin;

private:
	UIType _type = up;
	CodeType _code = -1;
	string _name;
	TransPtr _control = nullptr;

public:
	GameUI() {}
	GameUI(CodeType code,
		const string& name,
		TransPtr control,
		const BlockPos& pos = BlockPos::zero,
		const string& pic = "")	: 
		_code(code),
		_name(name),
		_control(control)
		{}

	static UIPtr create(CodeType code, const string& name, TransPtr control, const BlockPos& pos = BlockPos::zero, vector<UIPtr>* container = nullptr, const string& pic = "");

	virtual LiveCodeType start();
	virtual GameJudgeResult action(bool* keyarray);
	
	UIType& type() { return this->_type; }
	CodeType& code() { return this->_code; }
	string& name() { return this->_name; }
	TransPtr& control() { return this->_control; }
};
