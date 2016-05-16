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
	int _z = 0;  // TODO ����ʱ�����ټ�һ�¸߶ȣ�1z����һ���moveӦ��ҪΪ�˲�ͬ���˶���ʽ�����Ӳ�����������پ�û����
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
	BlockPos mazeSize; // scene�ڲ���size�������ƿ�������Ĵ�С�����������������Ӽ��������Ĵ�С������
	bool focusOnKid = true;
	LiveObjPtr root = nullptr;
	map<LiveCodeType, LiveObjPtr> dict;
	LiveDot* blockMap = nullptr;
	TransPtr defaultTranslator = nullptr;

	static const int detectSplit = 5;

	// ����ĺ�������ֻ����һ��object�ģ�
	void blockAdd(LiveObjPtr ptr);
	void blockRemove(LiveObjPtr ptr);
	void blockMove(LiveObjPtr oldptr, const BlockPos& vec);
	void blockReplace(LiveObjPtr oldptr, ObjPtr newobj);

	// �ڵ�ͼ��һ������ɾ��һ��ͼ��Ķ�Ӧ��Ϣ��������ɾ��ͼ������е㼴һ����ͼ������mapRemove
	static void dotRemoveLayer(LiveDot& ld, LiveObjPtr ptr);

	// ע�����е�inBind��������map
	static void addBind(LiveObjPtr outptr, LiveObjPtr inptr);
	static void removeBind(LiveObjPtr outptr, LiveObjPtr inptr);
	static void removeAllOutBind(LiveObjPtr ptr);
	static void removeAllInBind(LiveObjPtr ptr);

	// ����ļ�����������ֻ�����޸ĸ��ӵ�ͼ�����漰�滭����
	// ��������Ҫ����ά����Ʒ֮��Ķ�̬��ϵ�İ���dict��paint��ά����paint��scenexxx�����ã������ڵ���������ʲô����scenexxx��ά����
	// outBind�еݹ����ʱ�ı�ը���ܣ��ͺ�����Ƕ�׵�nodeһ����

	void mapAdd(LiveObjPtr ptr, bool recursive = false);
	// ע�����������Ҫ��Ҫ�޸Ĺ�ϵ
	void mapRemove(LiveObjPtr ptr, bool recursive = false);
	// ɾ�����е�outBind��ͬʱ�޸Ĺ�ϵ
	void mapRemoveOutBind(LiveObjPtr ptr, bool recursive = false);
	// �����˰��ƶ�
	void mapMove(LiveObjPtr ptr, const BlockPos& vec, bool recursive = false);
	void mapReplace(LiveObjPtr oldptr, ObjPtr newobj);

public:
	GameLiveScene() {}
private:
	GameLiveScene(CodeType sceneCode, BlockPos mazeSize, BlockPos viewPoint, BlockPos windowSize);

public:
	LiveObjPtr queryCode(LiveCodeType code);

	// dict��obj paint/erase����֮��ά����paint/erase��scenexxx������
	// ���ڵ���������ʲô����scenexxx��ά��
	// ��ʾ���ǻ�һ���յ�Scene�͵ײ�Node, Ȼ���������Ҫ��ʼ����ͼ��
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

	// �����ͼ���������Ŀǰ�ӵ�/���ڵ�λ�ã�ͨ��pos��ȥviewpoint�õ�
	BlockPos getWindowRelativePosition(const BlockPos& pos);
	
	// �����ͼ���������ĳ�����λ�ã�ͨ��pos��ȥptr��margin�õ�
	static BlockPos getObjectRelativePosition(const BlockPos& pos, LiveObjPtr ptr);
	
	// ��������������Ǽ��һ�����ϵĸ���ͼ��Ŀɷ��ж�������� out_jumpObj�����������ת���߻�������Ļ�������ת���Ǹ�ͼ�㰡
	// �ӹ�������˵�ǳ����Լ�֮���ͼ��ֻҪ��һ��˵�����߾Ͳ����ߣ�������ת���߻��е�������鵽���Ǹ�����ת���е����
	// ���е����ȼ�������ת
	Walkable detect(LiveObjPtr ptr, const LiveDot& ld, const BlockPos& current, LiveObjPtr& out_jumpObj);
	
	// ��������������Ǽ��һ��������ĳ����ɷ��˶��������У������������������ǵ�������������ƶ��Ļ��������ƶ�֮���λ��
	// �����ǳ����Լ�֮�������κ�һ�����κ�һ����Ʒ˵�������ǾͲ������ˣ����ܻ����ſ����忨λ��bug�𣿣�
	// ��������ͬ������һ����ĺ���
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
	// �����滹�������ݼ���scene���涫���仯�Ĳ��ְ�
	void api_kidJump(CodeType sceneCode, BlockPos blocksize, BlockPos kidpos);

	~GameLive() {
		if (_scene != nullptr)
			delete _scene;
		if (_keys != nullptr)
			delete _keys;
	}
};