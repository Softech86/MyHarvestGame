#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

using std::vector;
using std::string;
using std::map;

class GameLive;
class GameBase;
class GamePaintPool;
class GameCommand;
class GamePrincipal;
class GameCalculate;
class GamePaint;
class GameUI;

struct PxPos;
struct BigBlockPos;
struct BlockPos;
struct LinkerSecond;
class AlphaBMP;
class GameObject;
class GameLinker;
class GameEvent;
class GameTranslator;

class GameLiveObject;
class GameLiveScene;
class GameLiveUI;

enum GamePress;
enum GameBasicCommand;
enum StuffCode;
enum SceneCode;
enum TransCode;
enum UICode;
enum WalkType; 
enum Walkable;
enum KeyName;
enum GameJudgeResult;

typedef int BlockType;
typedef float PxType;
typedef float NumType;
typedef int CodeType;
typedef int* LiveCodeType;

typedef std::shared_ptr<GameObject> ObjPtr;
typedef std::weak_ptr<GameObject> ObjWeak;
typedef std::shared_ptr<GameTranslator> TransPtr;
typedef std::shared_ptr<GameUI> UIPtr;
typedef std::shared_ptr<GameEvent> EventPtr;
typedef std::shared_ptr<GameLinker> LinkerPtr;
typedef std::shared_ptr<map<KeyName, NumType>> ValuePtr;

typedef std::shared_ptr<GameLiveObject> LiveObjPtr;
typedef std::weak_ptr<GameLiveObject> LiveObjWeak;
typedef std::shared_ptr<GameLiveUI> LiveUIPtr;

typedef std::vector<LiveObjPtr> LiveDot;

template<class Item>
inline Item* get(Item* array, BlockPos size, BlockPos pos) {
	if (pos.x >= 0 && pos.y >= 0 && pos.x < size.x && pos.y < size.y)
		return array + pos.x * size.y + pos.y;
	else
		return nullptr;
}
