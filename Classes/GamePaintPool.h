#pragma once

#include <vector>
#include "GameBase.h"

class PaintCmd {
public:
	enum CmdType { empty,
		sceneNew, sceneDraw, sceneClear, //意思分别是新建场景，切换场景，释放场景，是这样的。
		objAddToScene, objAddToObj, objMove, objRotate, objRemove, objListenMouse, objListenKey };
private:
	PaintCmd::CmdType type = CmdType::empty;
	LiveCodeType* code = nullptr;
	std::vector<std::string> comments;
	std::vector<NumType> nums;
	std::vector<LiveCodeType*> pointers;
public:
	PaintCmd(PaintCmd::CmdType type) {
		this->type = type;
	}
	PaintCmd(PaintCmd::CmdType type, LiveCodeType* code) {
		this->type = type;
		this->code = code;
	}
	PaintCmd(PaintCmd::CmdType type, LiveCodeType* code, LiveCodeType* father, string picture, PxPos pos) {
		this->type = type;
		this->code = code;
		this->pointers.push_back(father);
		this->comments.push_back(picture);
		this->nums.push_back(pos.x);
		this->nums.push_back(pos.y);
	}
	PaintCmd::CmdType getType() { return this->type; }
	LiveCodeType* getCode() { return this->code; }
	LiveCodeType* getFather() { 
		if (this->pointers.size() > 0)
			return this->pointers[0];
		else
			return nullptr;
	}
	string* getPicture() {
		if (this->comments.size() > 0)
			return &(this->comments[0]);
		else
			return nullptr;
	}
	PxType* getPosX() {
		if (this->nums.size() >= 2)
			return &(nums[0]);
		else
			return nullptr;
	}
	PxType* getPosY() {
		if (this->nums.size() >= 2)
			return &(nums[1]);
		else
			return nullptr;
	}
	std::vector<std::string>& getComments() { return this->comments; }
	std::vector<NumType>& getNums() { return this->nums; }
	std::vector<LiveCodeType*>& getPointers() { return this->pointers; }
};

class GamePaintPool {
private:
	typedef PaintCmd* ContentPtr;
	std::vector<ContentPtr> data;
public:
	void pushBack(ContentPtr pcmd) { this->data.push_back(pcmd); }
	void pushFront(ContentPtr pcmd) { this->data.insert(this->data.begin(), pcmd); }
	ContentPtr getBack() {
		if (data.size() == 0) return nullptr;
		else return *(this->data.end() - 1);
	}
	ContentPtr getFront() {
		if (data.size() == 0) return nullptr;
		else return *(this->data.begin());
	}

	// 队列里的每一个命令必须要是new出来的。
	void popBack() {
		auto res = this->getBack();
		if (res != nullptr)
			delete res;
		this->data.pop_back();
	}

	void popFront() {
		auto res = this->getFront();
		if (res != nullptr)
			delete res;
		this->data.erase(this->data.begin());
	}
};
