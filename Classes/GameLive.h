#pragma once

#include <memory>
#include <vector>
#include "ClassHeader.h"
#include "GameBase.h"
#include "GameUI.h"
#include "GamePrincipal.h"

class GameLiveObject{
private:
	GameObject _obj;
	LiveCodeType _paintCode = nullptr;
	BlockPos _margin;
	int _z = 0;  // TODO 绘制时用它再加一下高度，1z等于一大格，move应该要为了不同的运动方式而增加参数，否则变速就没有了
	float _scale = 1;
	float _alpha = 1;
	vector<LiveObjWeak> _inBind;
	vector<LiveObjWeak> _outBind;
public:
	GameLiveObject() {}
	GameLiveObject(ObjPtr pobj);
	GameLiveObject(ObjPtr pobj, const BlockPos& margin);
	GameLiveObject(CodeType code, bool isScene) 
		: GameLiveObject(isScene ? GamePrincipal::getBase().getScene(code) : GamePrincipal::getBase().getStuff(code)) {}

	GameObject& obj() { return this->_obj; }
	LiveCodeType paintCode() { return this->_paintCode; }
	string& picture() { return this->_obj.picture(); }
	BlockPos& size() { return this->_obj.size(); }
	BlockPos& margin() { return this->_margin; }
	BlockPos& padding() { return this->_obj.padding(); }
	WalkType& walktype() { return this->_obj.walktype(); }
	AlphaBMP& walkBMP() { return this->_obj.walkBMP(); }
	int& zValue() { return this->_z; }
	vector<LiveObjWeak>& inBind() { return this->_inBind; }
	vector<LiveObjWeak>& outBind() { return this->_outBind; }
	float& scale() { return this->_scale; }
	float& alpha() { return this->_alpha; }

	//TODO
	LiveCodeType paintRoot();
	//TODO z alpha scale
	LiveCodeType paint(LiveCodeType father);
};

class GameLiveUI {
private:
	GameUI _ui;
	LiveCodeType _id;
public:
	GameLiveUI() {}
	GameLiveUI(UIPtr ori);

	GameUI& ui() { return this->_ui; }
	LiveCodeType& id() { return this->_id; }
};

class GameLiveScene {
private:
	ObjPtr scene = nullptr;
	BlockPos viewPoint;
	BlockPos windowSize;
	BlockPos mazeSize; // scene内部的size用来控制可视区域的大小，这个则控制整个格子计算的区域的大小啊哈哈
	bool focusOnKid = true;
	LiveObjPtr root = nullptr;
	map<LiveCodeType, LiveObjPtr> dict;
	LiveDot* blockMap = nullptr;
	TransPtr defaultTranslator = nullptr;

	static const int detectSplit = 5;

	// 这里的函数都是只操作一个object的，
	void blockAdd(LiveObjPtr ptr);
	void blockRemove(LiveObjPtr ptr);
	void blockMove(LiveObjPtr oldptr, const BlockPos& vec);
	void blockReplace(LiveObjPtr oldptr, ObjPtr newobj);

	// 在地图的一个点上删掉一个图层的对应信息，想完整删除图层的所有点即一整个图层请用mapRemove
	static void dotRemoveLayer(LiveDot& ld, LiveObjPtr ptr);

	// 注：所有的inBind均不包含map
	static void addBind(LiveObjPtr outptr, LiveObjPtr inptr);
	static void removeBind(LiveObjPtr outptr, LiveObjPtr inptr);
	static void removeAllOutBind(LiveObjPtr ptr);
	static void removeAllInBind(LiveObjPtr ptr);

	// 这里的几个函数还是只负责修改格子地图而不涉及绘画的啦
	// 但是它们要负责维护物品之间的动态关系的啊（dict由paint来维护，paint由scenexxx来调用）（根节点的特殊规则什么的由scenexxx来维护）
	// outBind有递归操作时的爆炸可能，就和自我嵌套的node一样吧

	void mapAdd(LiveObjPtr ptr, bool recursive = false);
	// 注意两种情况都要需要修改关系
	void mapRemove(LiveObjPtr ptr, bool recursive = false);
	// 删除所有的outBind，同时修改关系
	void mapRemoveOutBind(LiveObjPtr ptr, bool recursive = false);
	// 别忘了绑定移动
	void mapMove(LiveObjPtr ptr, const BlockPos& vec, bool recursive = false);
	void mapReplace(LiveObjPtr oldptr, ObjPtr newobj);

public:
	GameLiveScene() {}
private:
	GameLiveScene(CodeType sceneCode, BlockPos mazeSize, BlockPos viewPoint, BlockPos windowSize);

public:
	LiveObjPtr queryCode(LiveCodeType code);

	// dict由obj paint/erase画完之后维护，paint/erase由scenexxx来调用
	// 根节点的特殊规则什么的由scenexxx来维护
	// 显示上是画一个空的Scene和底层Node, 然后操作上是要初始化地图的
	void sceneInit(const BlockPos& size);
	void sceneAdd(ObjPtr ptr, const BlockPos& margin);
	void sceneRemove(LiveObjPtr ptr);
	void sceneMove(LiveObjPtr ptr, BlockPos vec);
	void sceneReplace(LiveObjPtr oldptr, ObjPtr newptr, const BlockPos& margin);
	void sceneSetKid(ObjPtr child, const BlockPos& margin);
	void sceneMoveKid(const BlockPos& vec);
	void sceneRemoveKid();

	void sceneDim(bool black = true);
	void sceneClear();

	// 计算地图坐标相对于目前视点/窗口的位置，通过pos减去viewpoint得到
	BlockPos getWindowRelativePosition(const BlockPos& pos);
	
	// 计算地图坐标相对于某物件的位置，通过pos减去ptr的margin得到
	static BlockPos getObjectRelativePosition(const BlockPos& pos, LiveObjPtr ptr);
	
	// 这个函数的作用是检测一个点上的各个图层的可否行动的情况， out_jumpObj填入如果有跳转或者滑行情况的话触发跳转的那个图层啊
	// 从规则上面说是除了自己之外的图层只要有一个说不能走就不能走，触发跳转或者滑行的是最后检查到的那个可跳转滑行的物件
	// 滑行的优先级高于跳转
	Walkable detect(LiveObjPtr ptr, const LiveDot& ld, const BlockPos& current, LiveObjPtr& out_jumpObj);
	
	// 这个函数的作用是检测一个物体向某方向可否运动的情况，校验的区域就是物件所覆盖的区域所以如果移动的话请填入移动之后的位置
	// 规则是除了自己之外遇到任何一个点任何一个物品说不能走那就不能走了（可能会有门口物体卡位的bug吗？）
	// 其他规则同上面检测一个点的函数
	Walkable detect(LiveObjPtr ptr, const BlockPos::Direction& dir, LiveObjPtr& out_jumpObj);

	~GameLiveScene() {
		if (blockMap != nullptr)
			delete[] blockMap;
	}
};

class GameLive {
public:
	static const int LOOP_FREQ_MS = 20;
	static const int KEY_COUNT = 20;
	static const string KEY_LOOP_NAME;
private:
	vector<LiveUIPtr> _UIUp;
	vector<LiveUIPtr> _UIDown;
	GameLiveScene* _scene;
	float _loopfreq = LOOP_FREQ_MS / 1000;
	bool _close = false;
	bool* _keys;

public:
	GameLive() {}

	void keySet();
	void enter();
	void start();
	void save();
	void keyLoop();
	void judge();
	bool* keys() { return this->_keys; }

	void api_UIStart(CodeType uicode);
	void api_UIStart(UIPtr uiptr);
	void api_UIStop(LiveCodeType id);
	void api_eventStart(CodeType eveCode, LiveObjPtr obj);
	void api_eventStart(EventPtr eve, LiveObjPtr obj);
	void api_close();

	void api_sceneInit(CodeType sceneCode, BlockPos blocksize);
	void api_sceneCalculate();
	void api_kidMove(BlockPos::Direction dir);
	void api_kidPick(LiveObjPtr stuff);
	void api_kidPos(BlockPos pos);
	// 这里面还有用数据计算scene里面东西变化的部分啊
	void api_kidJump(CodeType sceneCode, BlockPos blocksize, BlockPos kidpos);

	~GameLive() {
		if (_scene != nullptr)
			delete _scene;
		if (_keys != nullptr)
			delete _keys;
	}
};