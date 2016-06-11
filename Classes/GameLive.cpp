#include "GameLive.h"
#include "GameBase.h"
#include "GamePrincipal.h"
#include "cocos2d.h"
#include "GamePaint.h"
#include <cmath>
#include <list>
#include <thread>

// <----->
void GameLive::enter() {
	api_UIStart(UICode::startPageCode);
}
// <----->

GameLiveObject::GameLiveObject(ObjPtr pobj, bool copy) {
    if (pobj == nullptr) {
        _obj = ObjPtr(new GameObject(GameObject::origin));
    } else {
		if (copy)
			_obj = ObjPtr(pobj->SHCP());
		else
			_obj = pobj;
    }
}

GameLiveObject::GameLiveObject(ObjPtr pobj, const BlockPos& margin__, bool copy)
: GameLiveObject::GameLiveObject(pobj, copy) {
    this->margin() = margin__;
}

GameLiveObject::StickTo GameLiveObject::whereToStick(GameObject::BigType type) {
	switch (type)
	{
		break;
	case GameObject::background:
		return background;
	case GameObject::ground:
	case GameObject::seed:
		return flat;
		break;
	case GameObject::building:
	case GameObject::furniture:
	case GameObject::stuff:
	case GameObject::plant:
	case GameObject::animal:
	case GameObject::npc:
	case GameObject::kid:
		return cube;
		break;
	case GameObject::empty:
	case GameObject::combStatue:
		return cube;
		break;
	case GameObject::weather:
	case GameObject::bubble:
		return cloud;
		break;
	default:
		return cube;
		break;
	}
}

const int GameLiveObject::layerOrderMultiplier = 16;

BlockPos GameLiveObject::paintPos() {
    return this->MP() + BlockPos(0, this->zValue());
}

void GameLiveObject::decodePaintPos(BlockPos pos) {
	this->_margin = pos - BlockPos(0, this->zValue()) - this->padding();
}

int GameLiveObject::paintLayerOrder(int dotOrder) {
	if (dotOrder < 0)
		dotOrder = 0;
	return 40000 - this->MP().y * layerOrderMultiplier + dotOrder;
}

void GameLiveObject::setZOrder(float Zorder) {
	PAINT.objZOrder(this->paintCode(), Zorder);
	this->_order = Zorder;
}

void GameLiveObject::setZOrder(float Zorder, float timeSec, float delaySec) {
	PAINT.objZOrder(this->paintCode(), this->_order, Zorder, timeSec, delaySec);
	this->_order = Zorder;
}

void GameLiveObject::autoZOrder(int dotOrder) {
	if (dotOrder < 0)
		dotOrder = 0;
	this->setZOrder(this->paintLayerOrder(dotOrder));
}

void GameLiveObject::autoZOrder(int dotOrder, float timeSec, float delaySec) {
	if (dotOrder < 0)
		dotOrder = 0;
	this->setZOrder(this->paintLayerOrder(dotOrder), timeSec, delaySec);
}

void GameLiveObject::setAlpha(float alpha, float timeSec, float delaySec) {
	if (alpha < 0)
		alpha = 0;
	if (alpha > 1)
		alpha = 1;
	PAINT.objAlpha(this->paintCode(), this->_alpha, alpha, timeSec, delaySec);
	this->_alpha = alpha;
}

LiveCode GameLiveObject::paint(LiveCode father, int dotOrder) {
	if (dotOrder < 0)
		dotOrder = 0;
    BlockPos pos = this->paintPos();
    LiveCode result;
	if (this->getObj()->isCustomPaint()) {
		result = this->getObj()->customPaint(father, pos, dotOrder);
    }
    else {
        result = PAINT.objAddToObj(father, this->picture(), pos, scale(), alpha());
		this->paintCode() = result;
		this->autoZOrder(dotOrder);
    }
	if (result == nullptr)
		cocos2d::log("paint: paint code null : %s", this->picture().c_str());
	this->getObj()->afterPaint(result);
    return result;
}

void GameLiveObject::erase(LiveCode father) {
    PAINT.objRemove(this->paintCode(), father);
	this->paintCode() = nullptr;
}

void GameLiveObject::move(const BlockPos& vec, MoveType move, float timeSec, float delaySec) {
	cocos2d::log("%s (%d, %d) move: (%d, %d) + %f + %f", this->getObj()->name().c_str(), (this->MP().x), this->MP().y, vec.x, vec.y, timeSec, delaySec);

    BlockPos pos = this->paintPos() + vec;
	LiveCode node = this->paintCode();
	CocoFunc sch = [node, pos, move, timeSec](float dt) {
		PAINT.objMove(node, pos, move, timeSec);
	};
	LIVE.api_autoLock(this->_movingUntil, timeSec, doAfter, sch, "move", delaySec);
	/*
	float now = PAINT.clock();
	float delay = 0;
	if (_movingUntil <= now) {
		_movingUntil = now;
	}
	delay = _movingUntil - now + delaySec;
	_movingUntil += delaySec + timeSec;*/
}


void GameLiveObject::cleanMove(const BlockPos& vec, MoveType move, float timeSec, float delaySec) {
	CocoFunc sch = [this, vec, move, timeSec](float dt) {
		cocos2d::Vec2 vecpos = this->_paintCode->getPosition();
		PxPos px = PAINT.unmix(PxPos(vecpos.x, vecpos.y));
		BlockPos paintblock(px);
		this->decodePaintPos(paintblock);

		BlockPos pos = this->paintPos() + vec;
		LiveCode node = this->paintCode();
		PAINT.objMove(node, pos, move, timeSec);
	};
	LIVE.api_autoLock(this->_movingUntil, timeSec, doBreak, sch, "move", delaySec);
	//BlockPos pos = this->paintPos();
	//float now = PAINT.clock();
	//_movingUntil = now + delaySec + timeSec;
	//PAINT.objMove(this->paintCode(), pos + vec, move, timeSec, delaySec);
}

LiveCode GameLiveObject::repaint(LiveCode father, int dotOrder) {
	if (dotOrder < 0)
		dotOrder = 0;
	erase(father);
	return paint(father, dotOrder);
}

LiveCode GameLiveObject::changePicture(const string& newpic, LiveCode father, int dotOrder) {
	this->getObj()->picture() = newpic;
	return repaint(father, dotOrder);
}

void GameLiveObject::alpha(float newalpha, float timeSec, float delaySec) {
	/*float now = PAINT.clock();
	float delay = 0;
	if (_alphaUntil <= now) {
		_alphaUntil = now;
	}
	delay = _alphaUntil - now + delaySec;
	_alphaUntil += delaySec + timeSec;*/
	float oldalpha = this->alpha();
	LiveCode node = this->paintCode();
	this->alpha() = newalpha;
	CocoFunc sch = [node, oldalpha, newalpha, timeSec](float dt) {
		PAINT.objAlpha(node, oldalpha, newalpha, timeSec);
	};
	LIVE.api_autoLock(this->_alphaUntil, timeSec, doAfter, sch, "alpha", delaySec);
}

GameLiveUI::GameLiveUI(UIPtr ori) {
	if (ori == nullptr) {
		this->_ui = UIPtr(new GameUI(GameUI::origin));
	}
    else
        this->_ui = UIPtr(ori->SHCP());
}

ObjPtr* GameLiveHuman::hasStuffInPack(ObjPtr obj) {
	for (ObjPtr& packobj : this->_pack) {
		if (packobj->code() == obj->code() && packobj->getQuality() == obj->getQuality()) {
			return &packobj;
		}
	}
	return nullptr;
}

ObjPtr GameLiveHuman::putIntoPack() {
	if (this->handIsEmpty() == false) {
		ObjPtr* objp = hasStuffInPack(this->_handObject);
		if (objp == nullptr || *objp == nullptr) {
			if ((int)this->getPack().size() < this->_packMaxSize) {
				auto result = this->_handObject;
				this->getPack().push_back(this->_handObject);
				this->clearHand();
				return result;
			}
			else
				return nullptr;
		}
		else {
			(*objp)->setCount((*objp)->getCount() + this->_handObject->getCount());
			return (*objp);
		}
	}
	else
		return nullptr;
}

void GameLiveHuman::getRange(BlockPos& out_start, BlockPos& out_size) {
	if (this->_liveObj != nullptr) {
		if (this->rangeType == objectRelative) {
			BlockPos::Direction kidfacing = this->_liveObj->getFace();
			BlockPos start = this->_liveObj->MPC() + rangeCenter;
			BlockPos area;
			switch (kidfacing)
			{
			case BlockPos::three:
			case BlockPos::four:
			case BlockPos::six:
			case BlockPos::seven:
				area = rangeArea;
				break;
			case BlockPos::empty:
			case BlockPos::five:
			case BlockPos::one:
			case BlockPos::two:
			case BlockPos::eight:
			case BlockPos::nine:
			default:
				area = rangeArea.flip();
				break;
			}
			start -= area;
			area = area * BlockPos(2, 2);
			BlockPos::directionAreaSplit(start, area, kidfacing, 3, out_start, out_size);
		}
		else if (this->rangeType == zeroRelative) {
			out_start = rangeCenter - rangeArea;
			out_size = rangeArea * BlockPos(2, 2);
		}
	}
}

vector<GameLivePlant>::iterator GameLiveCreature::_findPlant(vector<GameLivePlant>& container, const BlockPos& margin) {
	for (auto lt = container.begin(); lt != container.end(); lt++) {
		if (lt->getMargin() == margin)
			return lt;
	}
	return container.end();
}

bool GameLiveCreature::_removePlant(vector<GameLivePlant>& container, const BlockPos& margin) {
	for (auto lt = container.begin(); lt != container.end();) {
		if (lt->getMargin() == margin) {
			lt = container.erase(lt);
			return true;
		}
		else
			lt++;
	}
	return false;
}

bool GameLiveCreature::hasPlant(BaseCode sceneCode, const BlockPos& position) {
	vector<GameLivePlant>* temp;
	if ((temp = getPlants(sceneCode)) != nullptr) {
		auto lt = _findPlant(*temp, position);
		if (lt != temp->end())
			return true;
		else
			return false;
	}
	else
		return false;
}

bool GameLiveCreature::removePlant(BaseCode sceneCode, const BlockPos& position) {
	vector<GameLivePlant>* temp;
	if ((temp = getPlants(sceneCode)) != nullptr) {
		return _removePlant(*temp, position);
	}
	else
		return false;
}

void GameLiveCreature::onSceneClose(GameLiveScene* oldscene) {
	for (auto &plant : plantData[oldscene->getCode()]) {
		plant.onSceneClose(oldscene);
	}
	this->removeAllHumanLive();
}

void GameLiveCreature::onSceneCreate(GameLiveScene* newscene) {
	for (auto &plant : plantData[newscene->getCode()]) {
		plant.onSceneCreate(newscene);
	}
}

void GameLiveCreature::onDayPass() {
	for (int i = sceneStart + 1; i < sceneEnd; i++) {
		for (auto &plant : plantData[i]) {
			plant.onDayPass();
		}
	}
}

BlockPos GameLiveScene::validize(const BlockPos& input) {
	BlockType dx, dy;
	if (input.x > mazeSize.x)
		dx = mazeSize.x;
	else if (input.x < 0)
		dx = 0;
	else
		dx = input.x;
	if (input.y > mazeSize.y)
		dy = mazeSize.y;
	else if (input.y < 0)
		dy = 0;
	else
		dy = input.y;
	return BlockPos(dx, dy);
}

struct RangeCache {
	LiveObjPtr ptr = nullptr;
	int layer = 0;
	RangeCache() {}
	RangeCache(LiveObjPtr ptr, int layer) : ptr(ptr), layer(layer) {}
};

// true means you need to access the map
inline bool useCache(std::list<RangeCache>& cache, const std::list<RangeCache>::iterator& end, LiveObjPtr obj, int layer, int count) {
	for (auto lt = cache.begin(); lt != end; lt++) {
		if ((*lt).ptr == obj) {
			if ((*lt).layer < layer) {
				(*lt).layer = layer;
				return true;
			}
			return false;
		}
	}
	if ((int)cache.size() == count || (int)cache.size() > count)
		cache.pop_front();
	cache.emplace_back(obj, layer);
	return true;
}

void GameLiveScene::rangeGetObjects(BlockPos start, BlockPos size, map<LiveObjPtr, int>& out_objects, LiveObjPtr itself) {
	// 因为这里不需要再切分所以validize是没有问题的，有待切分的地方用validize可是会失真的
	start = validize(start);
	BlockPos end = validize(start + size);
	// 我们来一波优化吧
	std::list<RangeCache> cache;
	auto endlt = cache.end();

	for (int ix = start.x; ix < end.x; ix++) {
		for (int iy = start.y; iy < end.y; iy++) {
			LiveDot* ld = get(blockMap, mazeSize, BlockPos(ix, iy));
			if (ld == nullptr)
				continue;
			else {
				for (int i = (int)ld->size() - 1; i > -1; i--){
					// 含有自己的点一律不统计
					if ((*ld)[i].lock() == itself)
						goto step_break_two;
				}
				for (int i = 0; i < (int)ld->size(); i++){
					LiveObjPtr ptr = (*ld)[i].lock();
					if (ptr == nullptr)
						cocos2d::log("[ERROR] nullptr in range");
					if (ptr != this->liveScene) {
						if (useCache(cache, endlt, ptr, i, 5)) {
							auto mapfind = out_objects.find(ptr);
							if (mapfind == out_objects.end()){
								auto pa = std::pair<LiveObjPtr, int>(ptr, i);
								out_objects.insert(pa);
								cocos2d::log("%d, %d : %s", ix, iy, ptr->getObj()->name().c_str());
							}
							else
								if (mapfind->second < i)
									out_objects[ptr] = i;
						}
					}
				}
			}
		step_break_two:
			;
		}
	}
}

void GameLiveScene::sortObjects(const BlockPos& kidPos, BlockPos::Direction kidDir, map<LiveObjPtr, int>& objects, vector<LiveObjPtr> &outResult) {
	for (auto& par : objects) {
		outResult.push_back(par.first);
	}
	//TODO
}

void GameLiveScene::kidRangeObjects(vector<LiveObjPtr>& out_result) {
	if (LIVE.api_kidGet() == nullptr)
		return;
	BlockPos start, size;
	LIVE.api_getKidHuman()->getRange(start, size);
	map<LiveObjPtr, int> objects;
	rangeGetObjects(start, size, objects, LIVE.api_kidGet());
	sortObjects(LIVE.api_kidGet()->MPC(), LIVE.api_kidGet()->getFace(), objects, out_result);
}

void GameLiveScene::blockMapClear() {
	int capa = mazeSize.x * mazeSize.y;
	for (int i = 0; i < capa; i++) {
		blockMap[i].clear();
	}
}

int GameLiveScene::insertPositionCompare(LiveObjPtr lhs, LiveObjPtr rhs) {
	if (lhs == nullptr) return -1;
	if (rhs == nullptr) return 1;
	if (lhs->zValue() != rhs->zValue())
		return lhs->zValue() - rhs->zValue();
	if (lhs->getObj()->type() != rhs->getObj()->type())
		return lhs->getObj()->type() - rhs->getObj()->type();
	return 0;
}

LiveDot::iterator GameLiveScene::findInsertPosition(LiveDot& ld, LiveObjPtr obj) {
	auto i = ld.begin();
	for (; i != ld.end(); ++i) {
		if (insertPositionCompare(i->lock(), obj) > 0)
			break;
	}
	return i;
}

int GameLiveScene::liveDotInsert(LiveDot &ld, LiveObjPtr obj) {
	auto pos = findInsertPosition(ld, obj);
	if (pos == ld.end()) {
		ld.push_back(obj);
		return ld.size() - 1;
	}
	else {
		int cache = std::distance(ld.begin(), pos);
		ld.insert(pos, obj);
		return cache;
	}
}

int GameLiveScene::liveDotOrderFind(LiveDot &ld, LiveObjPtr obj) {
	for (auto i = 0; i < (int)ld.size(); ++i) {
		if (ld[i].lock() == obj)
			return i;
	}
	return -1;
}

int GameLiveScene::blockAdd(const LiveObjPtr ptr) {
    if (ptr == nullptr)
        return -1;
	int result = -1;
    BlockPos drawpos = ptr->MP();
    for (int i = 0; i < ptr->size().x; i++) {
        for (int j = 0; j < ptr->size().y; j++) {
            auto ld = get(this->blockMap, this->mazeSize, drawpos + BlockPos(i, j));
            if (ld == nullptr)
                continue;
			else {
				int temp = liveDotInsert(*ld, ptr);
				if (i == 0 && j == 0)
					result = temp;
			}
        }
    }
	return result;
}

void GameLiveScene::dotRemoveLayer(LiveDot& ld, const LiveObjPtr ptr) {
    if (ptr == nullptr)
        return;
    for (auto i = ld.begin(); i != ld.end();) {
		auto tempptr = i->lock();
        if (tempptr == nullptr) {
            i = ld.erase(i);
        } else if (tempptr == ptr) {
            i = ld.erase(i);
            break;
        } else {
            i++;
        }
    }
}

void GameLiveScene::blockRemove(const LiveObjPtr ptr) {
    if (ptr == nullptr)
        return;
    BlockPos drawpos = ptr->MP();
    for (int i = 0; i < ptr->size().x; i++) {
        for (int j = 0; j < ptr->size().y; j++) {
            LiveDot *ld = get(this->blockMap, this->mazeSize, drawpos + BlockPos(i, j));
            if (ld == nullptr)
                continue;
            else
                dotRemoveLayer(*ld, ptr);
        }
    }
}

// 这里直接就修改live对象了
int GameLiveScene::blockMove(LiveObjPtr oldptr, const BlockPos& vec) {
    if (oldptr == nullptr)
        return -1;
    // 现在这东西很暴力，之后一定需要优化
    // 但是如果你想保留物品在移动之后的物品位置始终是最上面的话你是在这里不可以优化的啊
    // 嗯就是这样。
    blockRemove(oldptr);
    oldptr->margin() += vec;
    return blockAdd(oldptr);
}

// 这里直接就修改live对象了
int GameLiveScene::blockReplace(LiveObjPtr oldptr, ObjPtr newobj) {
    if (newobj == nullptr)
        return -1;
    else {
        blockRemove(oldptr);
		oldptr->setObj(newobj);
        return blockAdd(oldptr);
    }
}

int GameLiveScene::blockIndexQuery(LiveObjPtr ptr) {
	if (!ptr)
		return -1;
	auto ld = get(this->blockMap, this->mazeSize, ptr->MP());
	if (!ld)
		return -1;
	for (int i = 0; i < (int)(ld->size()); i++) {
		if ((*ld)[i].lock() == ptr)
			return i;
	}
	return -1;
}

void GameLiveScene::addBind(LiveObjPtr outptr, LiveObjPtr inptr) {
    if (outptr == nullptr || inptr == nullptr)
        return;
    outptr->outBind().push_back(inptr);
    inptr->inBind().push_back(outptr);
}

void GameLiveScene::removeBind(LiveObjPtr outptr, LiveObjPtr inptr) {
    if (outptr == nullptr || inptr == nullptr)
        return;
    for (auto i = outptr->outBind().begin(); i != outptr->outBind().end();) {
        LiveObjPtr tmp = (*i).lock();
        if (tmp == nullptr) {
            i = outptr->outBind().erase(i);
        } else if (tmp == inptr) {
            i = outptr->outBind().erase(i);
            break; // 想到break就觉得会有绑定好几遍的奇怪的bug发生
        } else {
            i++;
        }
    }
    for (auto i = inptr->inBind().begin(); i != inptr->inBind().end();) {
        LiveObjPtr tmp = (*i).lock();
        if (tmp == nullptr) {
            i = inptr->inBind().erase(i);
        } else if (tmp == outptr) {
            i = inptr->inBind().erase(i);
            break;
        } else {
            i++;
        }
    }
}

void GameLiveScene::removeAllOutBind(LiveObjPtr outptr) {
    if (outptr == nullptr)
        return;
    for (auto &i : outptr->outBind()) {
        LiveObjPtr inptr = i.lock();
        if (inptr == nullptr)
            continue;
        else {
            for (auto j = inptr->inBind().begin(); j != inptr->inBind().end();) {
                LiveObjPtr tmp = (*j).lock();
                if (tmp == nullptr) {
                    j = inptr->inBind().erase(j);
                } else if (tmp == outptr) {
                    j = inptr->inBind().erase(j);
                    break;
                } else {
                    j++;
                }
            }
        }
    }
    outptr->outBind().clear();
}

void GameLiveScene::removeAllInBind(LiveObjPtr inptr) {
    if (inptr == nullptr)
        return;
    for (auto &i : inptr->inBind()) {
        LiveObjPtr outptr = i.lock();
        if (outptr == nullptr)
            return;
        else {
            for (auto j = outptr->outBind().begin(); j != outptr->outBind().end();) {
                LiveObjPtr tmp = (*j).lock();
                if (tmp == nullptr) {
                    j = outptr->outBind().erase(j);
                } else if (tmp == inptr) {
                    j = outptr->outBind().erase(j);
                    break;
                } else {
                    j++;
                }
            }
        }
    }
    inptr->inBind().clear();
}

int GameLiveScene::mapAdd(LiveObjPtr ptr, bool recursive) {
    if (ptr == nullptr)
        return -1;
    else {
        int res = blockAdd(ptr);
        if (recursive) {
            for (LiveObjWeak &child : ptr->outBind()) {
                // 这里不用校验null了，调用的时候会处理的
                mapAdd(child.lock(), true);
            }
        }
		return res;
    }
}

void GameLiveScene::mapRemove(LiveObjPtr ptr, bool recursive) {
    if (ptr == nullptr)
        return;
    else {
        blockRemove(ptr);
        if (recursive) {
            for (LiveObjWeak &child : ptr->outBind()) {
                mapRemove(child.lock(), true);
            }
            removeAllOutBind(ptr);
			removeAllInBind(ptr);
        } else {
            for (auto &child : ptr->outBind())
                removeBind(ptr, child.lock());
            for (auto &child : ptr->inBind())
                removeBind(child.lock(), ptr);
        }
    }
}

void GameLiveScene::mapRemoveOutBind(LiveObjPtr ptr, bool recursive) {
    if (ptr == nullptr) {
        return;
    } else {
        for (auto &weak : ptr->outBind()) {
            mapRemove(weak.lock(), recursive);
        }
        ptr->outBind().clear();
    }
}

// 这里直接就修改live对象了
int GameLiveScene::mapMove(LiveObjPtr ptr, const BlockPos& vec, bool recursive) {
    if (ptr == nullptr)
        return -1;
    else {
        int res = blockMove(ptr, vec);
        if (recursive) {
            for (auto &child : ptr->outBind()) {
                mapMove(child.lock(), vec, true);
            }
        }
		return res;
    }
}

// 这里直接就修改live对象了
int GameLiveScene::mapReplace(LiveObjPtr oldptr, ObjPtr newptr) {
    return blockReplace(oldptr, newptr);
}

int GameLiveScene::getDotOrder(LiveObjPtr live) {
	if (live == nullptr)
		return -1;
	auto ld = get(blockMap, mazeSize, live->MP());
	if (ld == nullptr)
		return -1;
	return liveDotOrderFind(*ld, live);
}

void GameLiveScene::dotZOrderRefresh(const BlockPos& pos) {
	auto ldp = get(this->blockMap, this->mazeSize, pos);
	if (ldp == nullptr)
		return;
	auto& ld = *ldp;
	for (int i = 0; i < (int)ld.size(); i++) {
		auto tempptr = ld[i].lock();
		if (tempptr->MP() == pos) {
			tempptr->autoZOrder(i);
		}
	}
}

void GameLiveScene::dictAdd(LiveCode code, LiveObjPtr obj) {
    this->dict.insert(std::pair<LiveCode, LiveObjPtr>(code, obj));
}

void GameLiveScene::dictRemove(LiveCode code) {
    auto lt = this->dict.find(code);
    if(lt != this->dict.end()) {
        this->dict.erase(lt);
    }
}

int GameLiveScene::stuffFind(BaseCode stuff, const BlockPos& position) {
	auto ldp = get(this->blockMap, mazeSize, position);
	if (ldp == nullptr)
		return -1;
	else {
		LiveDot& ld = *ldp;
		for (int i = 0; i < (int)ld.size(); i++) {
			if (ld[i].lock()->getObj()->code() == stuff)
				return i;
		}
		return -1;
	}
}

bool GameLiveScene::stuffExistsOn(BaseCode stuffcode, const BlockPos& position) {
	if (stuffFind(stuffcode, position) == -1)
		return false;
	else
		return true;
}

LiveObjPtr GameLiveScene::queryCode(LiveCode code) {
    auto lt = this->dict.find(code);
    if(lt != this->dict.end()) {
        return nullptr;
    }
    else {
        return lt->second;
    }
}

// stick 参数要统一去掉
LiveObjPtr GameLiveScene::make(BaseCode ptr, bool scene, const BlockPos& margin, int z, float scale, float alpha) {
    ObjPtr temp;
    if(scene)
        temp = BASE.getScene(ptr);
    else
        temp = BASE.getStuff(ptr);
    return GameLiveScene::make(temp, margin, true, z, scale, alpha);
}

LiveObjPtr GameLiveScene::make(ObjPtr obj, const BlockPos& margin, bool copy, int z, float scale, float alpha) {
	BlockPos mppd = BlockPos::zero;
    if(this->scene != nullptr)
		mppd = this->scene->padding();
	return make_(obj, margin, mppd, BlockPos::zero, z, scale, alpha, copy);
}

LiveObjPtr GameLiveScene::make_(ObjPtr obj, const BlockPos& margin, const BlockPos& mappadding, BlockPos parentAdd, int z, float scale, float alpha, bool copy) {
	LiveObjPtr pt(new GameLiveObject(obj, margin + parentAdd + (obj->type() == GameObject::BigType::background ? BlockPos::zero : mappadding), copy));
	pt->zValue() = z;
	pt->scale() = scale;
	pt->alpha() = alpha;
	pt->setStick(GameLiveObject::whereToStick(obj->type()));

	this->cacheAdd(pt);

	parentAdd += pt->margin() + (obj->type() == GameObject::BigType::background ? BlockPos::zero : mappadding);
	for (int index = 0; index < (int)obj->children().size(); index++) {
		LiveObjPtr livechild = make_(obj->children()[index].lock(), obj->childrenPos()[index], mappadding, parentAdd, z, scale, alpha, copy);
		pt->getObj()->children()[index].lock() = livechild->getObj();
		addBind(pt, livechild);
	}
	return pt;
}

LiveCode GameLiveScene::getParent(LiveObjPtr obj) {
    LiveCode parent = nullptr;
	const auto& sti = obj->getStick();
	if (sti == GameLiveObject::StickTo::flat)
		parent = this->flatCode();
	else if (sti == GameLiveObject::StickTo::cube)
		parent = this->cubeCode();
	else if (sti == GameLiveObject::StickTo::cloud)
		parent = this->cloudCode();
	else if (sti == GameLiveObject::StickTo::background)
		parent = this->backgroundCode();
	return parent;
}

void GameLiveScene::init(const BlockPos& mazeSize) {
	this->codeRoot = PAINT.nodeNew();
	this->codeStuff = PAINT.objAddToObj(this->codeRoot, "", PxPos::zero);
	this->codeEffect = PAINT.objAddToObj(this->codeRoot, "", PxPos::zero);

	this->codeDim = PAINT.objAddToObj(this->codeEffect, "", PxPos::zero);
	this->codeLens = PAINT.objAddToObj(this->codeEffect, "", PxPos::zero);

	this->codeBackground = PAINT.objAddToObj(this->codeStuff, "", PxPos::zero);
    this->codeFlat = PAINT.objAddToObj(this->codeStuff, "", PxPos::zero);
    this->codeCube = PAINT.objAddToObj(this->codeStuff, "", PxPos::zero);
	this->codeCloud = PAINT.objAddToObj(this->codeStuff, "", PxPos::zero);
    
    this->mazeSize = mazeSize;
    this->blockMap = new LiveDot[mazeSize.x * mazeSize.y];
}

void GameLiveScene::setScene(BaseCode scene) {
	if (scene == -1)
		return;
	this->scene = BASE.getScene(scene);
	// scene.margin() always be zero
	this->liveScene = this->add(this->scene, BlockPos::zero);
	// viewPoint initialize to the left-under corner of the area you can see in this scene
	this->viewPoint = this->scene->padding();
	this->setTime = LIVE.api_getGameTime();
}

void GameLiveScene::resetScene() {
	blockMapClear();
	this->codeRoot = PAINT.nodeNew();
	this->codeStuff = PAINT.objAddToObj(this->codeRoot, "", PxPos::zero);
	this->codeEffect = PAINT.objAddToObj(this->codeRoot, "", PxPos::zero);

	this->codeDim = PAINT.objAddToObj(this->codeEffect, "", PxPos::zero);
	this->codeLens = PAINT.objAddToObj(this->codeEffect, "", PxPos::zero);

	this->codeBackground = PAINT.objAddToObj(this->codeStuff, "", PxPos::zero);
	this->codeFlat = PAINT.objAddToObj(this->codeStuff, "", PxPos::zero);
	this->codeCube = PAINT.objAddToObj(this->codeStuff, "", PxPos::zero);
	this->codeCloud = PAINT.objAddToObj(this->codeStuff, "", PxPos::zero);

	dict.clear();
	this->liveScene = nullptr;
	nodeCache.clear();
	viewMovingUntil = 0;
	setScene(getCode());
}

LiveObjPtr GameLiveScene::getStuff(const BlockPos& position, int index) {
	LiveDot* ld = get(this->blockMap, this->mazeSize, position);
	if (ld == nullptr)
		return nullptr;
	if (index < (int)ld->size() && index > -1) {
		return (*ld)[index].lock();
	}
	else
		return nullptr;
}

LiveObjPtr GameLiveScene::add(ObjPtr obj, const BlockPos& margin, bool copy, int z) {
	LiveObjPtr live;
	live = make(obj, margin, copy, z);
    add(live);
	return live;
}

LiveObjPtr GameLiveScene::findReplace(BaseCode oldobject, ObjPtr newobject, const BlockPos& mp, bool copy, int z, bool removeRecursive) {
	int ind = this->stuffFind(oldobject, mp);
	if (ind == -1)
		return nullptr;
	auto old = getStuff(mp, ind);
	if (old == nullptr)
		return nullptr;
	remove(old, removeRecursive);
	return add(newobject, mp - newobject->padding(), copy, z);
}

void GameLiveScene::cacheAdd(LiveObjPtr live) {
	this->nodeCache.push_back(live);
}

void GameLiveScene::cacheRemove(LiveObjPtr live) {
	for (auto lt = this->nodeCache.begin(); lt != this->nodeCache.end(); lt++) {
		if (*lt == live) {
			this->nodeCache.erase(lt);
			return;
		}
	}
}

void GameLiveScene::add(LiveObjPtr live) {
    int indx = mapAdd(live, false);
	if (indx == -1)
		return;
	dotZOrderRefresh(live->MP());

    LiveCode parent = this->getParent(live);
	if(parent == nullptr)
		return;

	LiveCode code = live->paint(parent, indx);
	if(code == nullptr)
		return;
	
    dictAdd(code, live);
	
	this->cacheRemove(live);

    for(auto childptr : live->outBind()) {
        add(childptr.lock());
    }

	if (this->getFocus() == live) {
		focusMoveViewPoint(live, PxPos::zero, live->MP(), true, 1);
	}
	return;
}

void GameLiveScene::remove(LiveObjPtr live, bool recursive) {
	if (live == nullptr)
		return;
    if(recursive) {
        mapRemove(live, true);
        for(auto childptr : live->outBind()) {
            remove(childptr.lock(), true);
        }
    } else {
        mapRemove(live, false);
    }
    dictRemove(live->paintCode());
	live->erase(this->getParent(live));
}

void GameLiveScene::movemove(LiveObjPtr ptr, const BlockPos& vec, MoveType move, float timeSec, bool recursive, bool faceChange) {
	BlockPos oldpos = ptr->MP();
	if (faceChange) {
		BlockPos::Direction facing = vec;
		ptr->setFace(ptr, facing);
	}
	ptr->move(vec, move, timeSec);

	int ind = mapMove(ptr, vec, false);
	dotZOrderRefresh(oldpos + vec);
	ptr->autoZOrder(ind, timeSec, 0);

	if (recursive) {
		for (auto childptr : ptr->outBind())
			movemove(childptr.lock(), vec, move, timeSec, true, faceChange);
	}

	if (this->getFocus() == ptr) {
		focusMoveViewPoint(ptr, oldpos, ptr->MP(), false, vec.distance() / timeSec);
	}
}

void GameLiveScene::replace(LiveObjPtr oldptr, ObjPtr newptr) {
	if (oldptr != nullptr && newptr != nullptr) {
		int indx = mapReplace(oldptr, newptr);
		dotZOrderRefresh(oldptr->MP());
		dictRemove(oldptr->paintCode());
		LiveCode code = oldptr->repaint(this->getParent(oldptr), indx);
		dictAdd(code, oldptr);
		// I guess there should be no problem
	}
}

void GameLiveScene::changePicture(LiveObjPtr ptr, const string& picture) {
	int indx = blockIndexQuery(ptr);
	dictRemove(ptr->paintCode());
	LiveCode code = ptr->changePicture(picture, this->getParent(ptr), indx);
	dictAdd(code, ptr);
}

GameLiveScene::LineReturn GameLiveScene::DistanceToCentralLine(const PxPos& windowRelative, const PxPos& direction, PxPos& outResult) {
	if (direction == PxPos::zero)
		return NEVER;
	PxPos window = PAINT.unmix(this->windowSize);
	float lineY = window.y / 2, lineX = window.x / 2;
	float disY = lineY - windowRelative.y, disX = lineX - windowRelative.x;
	float timeY = 0, timeX = 0;
	if (direction.y == 0) {
		timeX = disX / direction.x;
		timeY = -1;
	}
	else if (direction.x == 0) {
		timeY = disY / direction.y;
		timeX = -1;
	}
	else {
		timeY = disY / direction.y;
		timeX = disX / direction.x;
	}
	if (timeY < 0 && timeX < 0)
		return NEVER;
	PxPos result = direction;
	LineReturn retValue;
	if ((timeX > 1 || timeX < 0) && (timeY < 0 || timeY > 1)) {
		return NEVER;
	}
	if (timeY < 0) {
		result = result * timeX;
		retValue = XDIR;
	}
	else if (timeX < 0) {
		result = result * timeY;
		retValue = YDIR;
	}
	else {
		result = result * (timeX < timeY ? timeX : timeY);
		retValue = timeX < timeY ? XDIR : YDIR;
	}
	outResult = result;
	return retValue;
}

GameLiveScene::LineReturn GameLiveScene::DistanceToTheOtherLine(const PxPos& windowRelative, const PxPos& direction, PxPos& outResult) {
	if (direction == BlockPos::zero)
		return NEVER;
	PxPos window = PAINT.unmix(this->windowSize);
	float lineY = window.y / 2, lineX = window.x / 2;
	float disY = lineY - windowRelative.y, disX = lineX - windowRelative.x;
	float dis, velocity;
	LineReturn retValue;
	if (disY == 0) {
		dis = disX;
		velocity = direction.x;
		retValue = XDIR;
	}
	else {
		dis = disY;
		velocity = direction.y;
		retValue = YDIR;
	}
	if (velocity == 0)
		return NEVER;
	float ratio = dis / velocity;
	if (ratio < 0 || ratio > 1)
		return NEVER;
	else {
		PxPos result = direction;
		result = result * ratio;
		outResult = result;
		return retValue;
	}
}

PxPos GameLiveScene::distProcess(const PxPos& dist, LineReturn line, const PxPos& moveAll) {
	if (line == NEVER) {
		return PxPos::zero;
	}
	else if (line == YDIR) {
		if (std::abs(dist.x) < std::abs(moveAll.x))
			return PxPos(dist.x, 0);
		else
			return PxPos(moveAll.x, 0);
	}
	else if (line == XDIR) {
		if (std::abs(dist.y) < std::abs(moveAll.y))
			return PxPos(0, dist.y);
		else
			return PxPos(0, moveAll.y);
	}
	else
		return PxPos::zero;
}

PxPos GameLiveScene::moveBreak(const PxPos& moveAll, const PxPos& move2, const PxPos& direction) {
	PxPos mr = moveAll - move2;
	float xt = (float)mr.x / (float)direction.x;
	float yt = (float)mr.y / (float)direction.y;
	if (xt < 0 || yt < 0)
		return mr;
	if (xt < yt) {
		return direction * xt;
	}
	else {
		return direction * yt;
	}
}

PxPos GameLiveScene::focus(const PxPos& newpos) {
	PxPos winSze = PAINT.unmix(this->windowSize);
	PxPos scSze = this->scene->size();
	PxPos scPad = this->scene->padding();
	PxPos result;

	if (newpos.x + winSze.x / 2 > scSze.x + scPad.x) {
		result.x = (int)(scSze.x + scPad.x - winSze.x);
		if (result.x < scPad.x)
			result.x = scPad.x;
	}
	else if (newpos.x - winSze.x / 2 < scPad.x) {
		result.x = (int)(scPad.x);
	}
	else {
		result.x = (int)(newpos.x - winSze.x / 2);
	}

	if (scSze.x < winSze.x)
		result.x = -(winSze.x - scSze.x) / 2;
	

	if (newpos.y + winSze.y / 2 > scSze.y + scPad.y) {
		result.y = (int)(scSze.y + scPad.y - winSze.y);
		if (result.y < scPad.y)
			result.y = scPad.y;
	}
	else if (newpos.y - winSze.y / 2 < scPad.y) {
		result.y = (int)(scPad.y);
	}
	else {
		result.y = (int)(newpos.y - winSze.y / 2);
	}

	if (scSze.y < winSze.y)
		result.y = -(winSze.y - scSze.y) / 2;
	return result;
}

void GameLiveScene::focusMoveViewPoint(LiveObjPtr obj, const PxPos& oldpos, const PxPos& newpos, bool flash, float speed) {
	if(obj == nullptr)
		return;
	PxPos dist1 = PxPos::zero;
	PxPos centeradd = (PxPos)(obj->size()) * 0.5;
	PxPos centerold = oldpos + centeradd, centernew = newpos + centeradd;
	PxPos oldRela = getWindowRelativePosition(centerold);
	PxPos kidMove = newpos - oldpos;
	PxPos result = focus(centernew);
	PxPos cache = this->viewPoint;
	GameLiveScene::LineReturn flag1 = this->DistanceToCentralLine(oldRela, kidMove, dist1);
	PxPos moveAll = result - cache;
	double time1 = PxPos::time(dist1, speed * SMALL_BLOCK_PX);
	double sqrSum = PxPos::distance(PxPos::zero, kidMove);
	double timeAll = PxPos::time(kidMove, speed * SMALL_BLOCK_PX);

	//log
	cocos2d::log("\nViewPointMove [%f, %f] {", cache.x, cache.y);

	if (flash) {
		setViewPoint(result);
	}
	else if (result == cache) {
		moveViewPoint(result, 0, timeAll);
	}
	else {
		if (moveAll.x == 0 || moveAll.y == 0) {
			double time3;
			if (kidMove == PxPos::zero)
				time3 = 0;
			else if ((int)std::abs(moveAll.x) > 0) {
				time3 = std::abs(moveAll.x) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.x);
			}
			else {
				time3 = std::abs(moveAll.y) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.y);
			}
			moveViewPoint(result, time3, time1);
			double ta31 = timeAll - time3 - time1;
			if (ta31 > WUCHA)
				moveViewPoint(result, 0, ta31);

		}
		else {
			PxPos onelineRela = oldRela + dist1;
			PxPos dist2 = PxPos::zero;
			GameLiveScene::LineReturn flag2 = DistanceToTheOtherLine(onelineRela, kidMove, dist2);
			PxPos move2 = distProcess(dist2, flag2, moveAll);
			double time2;
			if (kidMove == PxPos::zero)
				time2 = 0;
			else if ((int)std::abs(move2.x) > 0) {
				time2 = std::abs(move2.x) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.x);
			}
			else {
				time2 = std::abs(move2.y) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.y);
			}
			if (time2 > WUCHA && time1 > WUCHA)
				moveViewPoint(cache + move2, time2, time1);
			if (flag2 == NEVER) {
				double ta21 = timeAll - time2 - time1;
				if (ta21 > WUCHA)
					moveViewPoint(result, 0, ta21);
				return;
			}
			else {
				double time25 = PxPos::time(dist2, speed * SMALL_BLOCK_PX);
				double time255 = time25 - time2;
				if (time255 > WUCHA) {
					moveViewPoint(cache + move2, 0, time255);
				}
				PxPos move3 = moveBreak(moveAll, move2, kidMove);
				PxPos dist3 = move3;
				double time3 = PxPos::time(dist3, speed * SMALL_BLOCK_PX);
				if (time3 > WUCHA)
					moveViewPoint(cache + move2 + move3, time3, 0);

				PxPos move4 = (PxPos)moveAll - move2 - move3;
				double time4;
				if (kidMove == PxPos::zero)
					time4 = 0;
				else if ((int)std::abs(move4.x) > 0) {
					time4 = std::abs(move4.x) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.x);
				}
				else {
					time4 = std::abs(move4.y) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.y);
				}
				PxPos dist4 = kidMove - dist1 - dist2 - dist3;
				if (time4 > WUCHA)
					moveViewPoint(result, time4, 0);
				double time45 = PxPos::time(dist4, speed * SMALL_BLOCK_PX);
				double time455 = time45 - time4;
				if (time455 > WUCHA)
					moveViewPoint(result, 0, time455);
			}
		}
	}
	//log
	cocos2d::log("}\n");
}

void GameLiveScene::setViewPoint(const PxPos& point) {
	PAINT.objMove(this->stuffCode(), -point, MoveType::linear, 0);
	this->viewPoint = point;
}

// TODO breakpause 打断之后的操作
void GameLiveScene::setFocus(const LiveObjPtr ptr, bool breakpause) {
	this->focusOn = ptr;
	focusMoveViewPoint(ptr, PxPos::zero, ptr->MP(), true, 1);
}

bool GameLiveScene::isFocusOnHuman(BaseCode humancode) {
	auto temp = LIVE.api_humanGet(humancode);
	if (temp == nullptr) return false;
	else if (this->getFocus() == temp) return true;
	else return false;
}

// rollback是一个对于dalyaSec为负数的确认提示
void GameLiveScene::moveViewPoint(const PxPos& point, float timeSec, float delaySec, bool rollback) {
	//log
	float now = PAINT.clock();
	cocos2d::log("\t@%f ->[%f, %f] + %lf + %lf", now, point.x, point.y, timeSec, delaySec);
	if (!rollback && delaySec < 0)
		delaySec = 0;
	if (this->viewMovingUntil <= now) {
		this->viewMovingUntil = now;
	}
	PAINT.objMove(this->stuffCode(), -point, MoveType::linear, timeSec, this->viewMovingUntil - now  + delaySec);
	this->viewMovingUntil += delaySec + timeSec;
	this->viewPoint = point;
}

// 这个函数仅仅复杂scene里面的human维护，creature是不修改的
LiveObjPtr GameLiveScene::humanSet(BaseCode humancode, ObjPtr child, const BlockPos& margin, bool copy) {
	if (child == nullptr)
		return nullptr;
	auto temp = LIVE.api_humanGet(humancode);
	if (temp == nullptr) {
		LiveObjPtr kind = GameLiveScene::make(child, margin, copy);
		add(kind);
		return kind;
	}
	else
		return temp; //现在的设定是humanSet只在没有human的时候有效。 // 注意换场景的时候清空creature的人物！ TODO
}

void GameLiveScene::humanMove(BaseCode humancode, const BlockPos& vec, MoveType type, float time, bool recursive, bool faceChange){
	if (LIVE.api_humanGet(humancode) == nullptr)
		return;
	else {
		BlockPos temp = LIVE.api_humanGet(humancode)->MP();
		movemove(LIVE.api_humanGet(humancode), vec, type, time, recursive, faceChange);
	}
}

void GameLiveScene::humanWalk(BaseCode humancode, const BlockPos& vec, bool faceChange) {
	if (vec != BlockPos::zero) {
		float time = BlockPos::time(vec, walkSpeed);
		humanMove(humancode, vec, MoveType::linear, time, true, faceChange);
	}
}

void GameLiveScene::humanRun(BaseCode humancode, const BlockPos& vec, bool faceChange) {
	if (vec != BlockPos::zero) {
		float time = BlockPos::time(vec, walkSpeed * runComparedToWalk);
		humanMove(humancode, vec, MoveType::linear, time, true, faceChange);
	}
}

void GameLiveScene::humanRemove(BaseCode humancode, bool recursive) {
	if (LIVE.api_humanGet(humancode) == nullptr)
		return;
	else{
		remove(LIVE.api_humanGet(humancode), recursive);
	}
}

void GameLiveScene::humanReplace(BaseCode humancode, ObjPtr newhuman) {
	if (LIVE.api_humanGet(humancode) == nullptr)
		return;
	else {
		replace(LIVE.api_humanGet(humancode), newhuman);
	}
}

LiveObjPtr GameLiveScene::humanAddObject(BaseCode humancode, ObjPtr obj, const BlockPos& marginRelative, bool copy, int z) {
	if (LIVE.api_humanGet(humancode)) {
		BlockPos humanpos = LIVE.api_humanGet(humancode)->MP();
		LiveObjPtr live = add(obj, marginRelative + humanpos, copy, z);
		addBind(LIVE.api_humanGet(humancode), live);
		return live;
	}
	return nullptr;
}

void GameLiveScene::switchFromSurroundingsToHuman(BaseCode humancode, LiveObjPtr obj, const BlockPos& margin, bool recursive, int z) {
	remove(obj, recursive);
	obj->margin() = margin;
	obj->zValue() = z;
	add(obj);
	addBind(LIVE.api_humanGet(humancode), obj);
}

void GameLiveScene::switchFromHumanToSurroundings(LiveObjPtr obj, const BlockPos& margin, bool recursive) {
	remove(obj, recursive);
	obj->margin() = margin;
	add(obj);
}

void GameLiveScene::allDim(bool black, float timeSec) {
	removeDim();
	auto color = black ? cocos2d::Color4B(0, 0, 0, 0) : cocos2d::Color4B(255, 255, 255, 0);
	auto colorLayer = cocos2d::LayerColor::create(color);
	auto effects = this->codeDim;
	effects->addChild(colorLayer);
	PAINT.objAlpha(colorLayer, 0, 1, timeSec, 0);
}

void GameLiveScene::allDimFrom(bool black, float timeSec) {
	removeDim();
	auto color = black ? cocos2d::Color4B(0, 0, 0, 255) : cocos2d::Color4B(255, 255, 255, 255);
	auto colorLayer = cocos2d::LayerColor::create(color);
	auto effects = this->codeDim;
	effects->addChild(colorLayer);
	PAINT.objAlpha(colorLayer, 1, 0, timeSec, 0);
}

void GameLiveScene::removeDim(float delaySec) {
	if (delaySec == 0)
		this->codeDim->removeAllChildren();
	else {
		auto dim = this->codeDim;
		CocoFunc sch = [dim](float dt) {
			dim->removeAllChildren();
		};
		GameLive::api_autoLock(this->_animeUntil, 0, doAfter, sch, "SceneDimRemove", delaySec);
	}
}

void GameLiveScene::allClear() {
	PAINT.nodeRemoveAllChildren(this->backgroundCode());
	PAINT.nodeRemoveAllChildren(this->flatCode());
	PAINT.nodeRemoveAllChildren(this->cubeCode());
	PAINT.nodeRemoveAllChildren(this->cloudCode());
	PAINT.nodeRemoveAllChildren(this->codeDim);
	PAINT.nodeRemoveAllChildren(this->codeLens);
}

BlockPos GameLiveScene::getWindowRelativePosition(const BlockPos& pos) {
    return pos - this->viewPoint;
}

BlockPos GameLiveScene::getObjectRelativePosition(const BlockPos& pos, LiveObjPtr ptr) {
    return pos - ptr->MP();
}

Walkable GameLiveScene::detect(LiveObjPtr ptr, const LiveDot& ld, const BlockPos& current, LiveObjPtr& out_jumpObj) {
	LiveObjPtr stopLayer = nullptr;
    LiveObjPtr jumpLayer = nullptr;
    LiveObjPtr slideLayer = nullptr;
    for (auto i = ld.begin(); i != ld.end(); i++) {
        // variable i cannot be null because it's an iterator, and it's not the end
        // so *i is ok
        LiveObjPtr live = i->lock();
        if (live == nullptr)
            continue;
        else {
			if (live == ptr)
				continue;
            WalkType wt = live->walktype();
            if (wt == WalkType::allWalk)
                continue;
            else if (wt == WalkType::noneWalk) {
				stopLayer = live;
                break;
            } else if (wt == WalkType::alphaWalk) {
                BlockPos rela = getObjectRelativePosition(current, live);
                std::shared_ptr<GameAlpha> &alpha = live->walkBMP();
				if (alpha == nullptr)
					return canWalk;
                Walkable able = alpha->getWalk(rela);
                if (able == Walkable::nullWalk)
                    continue;
				else if (able == Walkable::noWalk) {
					stopLayer = live;
                    break;
                } else if (able == Walkable::canWalk) {
                    continue;
                } else if (able == Walkable::jump) {
                    jumpLayer = live;
                } else if (able == Walkable::slide) {
                    slideLayer = live;
                } else {
                }
            } else {
            }
        }
    }
    Walkable result = Walkable::nullWalk;
    if (stopLayer != nullptr) {
        result = Walkable::noWalk;
		out_jumpObj = stopLayer;
    } else if (slideLayer != nullptr) {
        result = Walkable::slide;
        out_jumpObj = slideLayer;
    } else if (jumpLayer != nullptr) {
        result = Walkable::jump;
        out_jumpObj = jumpLayer;
    } else {
        result = Walkable::canWalk;
    }
    return result;
}

Walkable GameLiveScene::detect(LiveObjPtr ptr, const BlockPos& vec, const BlockPos::Direction& dir, LiveObjPtr& out_jumpObj) {
    if (ptr == nullptr)
        return Walkable::nullWalk; // if it returns this, there seems to be a bug, so we need to print a warning.
    BlockPos start;
    BlockType dx, dy;

	BlockPos out_size, out_start;
	//BlockPos::directionAreaSplit(newptr->MP() + vec, newptr->size(), dir, detectSplit, out_start, out_size);
	out_size = ptr->size();
	out_start = ptr->MP(); //我先让他全域监测吧
	dx = out_size.x;
	dy = out_size.y;

	//这里我们要干一点特殊的事情就是如果他一开始就是卡位的话，就是有点是不可动的话且没有别的物件再阻碍他的话，是的我们现在只是检测一层，那么就要允许他动
	LiveObjPtr preBlockLayer = nullptr;
	for (BlockType i = 0; i < dx; i++) {
		for (BlockType j = 0; j < dy; j++) {
			auto current = out_start + BlockPos(i, j);
			LiveDot *ld = get(this->blockMap, this->mazeSize, current);
			if (ld == nullptr)
				continue;
			else {
				auto dotres = detect(ptr, *ld, current, out_jumpObj);
				if (dotres == Walkable::noWalk) {
					preBlockLayer = out_jumpObj;
					goto step_second;
				}
			}
		}
	}

step_second:
	out_start += vec;
    bool noPass = false;
    LiveObjPtr jumpLayer = nullptr;
    LiveObjPtr slideLayer = nullptr;
    for (BlockType i = 0; i < dx; i++) {
        for (BlockType j = 0; j < dy; j++) {
            auto current = out_start + BlockPos(i, j);
            LiveDot *ld = get(this->blockMap, this->mazeSize, current);
            if (ld == nullptr)
                continue;
            else {
                auto dotres = detect(ptr, *ld, current, out_jumpObj);
                if (dotres == Walkable::nullWalk)
                    continue;
                else if (dotres == Walkable::noWalk) {
					if (out_jumpObj != preBlockLayer) {
						noPass = true;
						goto step_out;
					}
					else
						continue;
                } else if (dotres == Walkable::canWalk) {
                    continue;
                } else if (dotres == Walkable::jump) {
                    jumpLayer = out_jumpObj;
                } else if (dotres == Walkable::slide) {
                    slideLayer = out_jumpObj;
                } else {
                }
            }
            // 这玩意好复杂啊，很复杂的代码就算你打错了你也不知道，这才是最吓人的。
        }
    }

step_out:
    Walkable result = Walkable::nullWalk;
    if (noPass)
        result = Walkable::noWalk;
    else if (slideLayer != nullptr) {
        result = Walkable::slide;
        out_jumpObj = slideLayer;
    } else if (jumpLayer != nullptr) {
        result = Walkable::jump;
        out_jumpObj = jumpLayer;
    } else {
        result = Walkable::canWalk;
    }
    return result;
}

bool GameLiveScene::getJump(LiveObjPtr human, const BlockPos& dist, LiveObjPtr eventLayer, BaseCode& out_scenecode, BlockPos& out_kidpos, bool getSlide) {
	BlockPos relative = getObjectRelativePosition(human->MP(), eventLayer) + dist;
	return eventLayer->getObj()->getJump(relative, human->size(),  out_scenecode, out_kidpos);
}

GameLive::GameLive() {
	this->_creature = new GameLiveCreature();
}

void GameLive::keySet() {
    auto el = cocos2d::EventListenerKeyboard::create();
    el->onKeyPressed = [](cocos2d::EventKeyboard::KeyCode kc, cocos2d::Event * event) {
		switch (kc) {
		case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
			LIVE.press()[GameKeyPress::buttonUp] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			LIVE.press()[GameKeyPress::buttonLeft] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			LIVE.press()[GameKeyPress::buttonDown] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			LIVE.press()[GameKeyPress::buttonRight] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_Z:
			LIVE.press()[GameKeyPress::buttonA] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_X:
			LIVE.press()[GameKeyPress::buttonB] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_C:
			LIVE.press()[GameKeyPress::buttonC] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_U:
			LIVE.press()[GameKeyPress::buttonU] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_I:
			LIVE.press()[GameKeyPress::buttonI] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_O:
			LIVE.press()[GameKeyPress::buttonO] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_P:
			LIVE.press()[GameKeyPress::buttonP] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_N:
			LIVE.press()[GameKeyPress::buttonN] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_M:
			LIVE.press()[GameKeyPress::buttonM] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_ENTER:
			LIVE.press()[GameKeyPress::buttonStart] = true;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
			LIVE.press()[GameKeyPress::buttonSpace] = true;
			break;
		default:
			break;
		}
    };
    el->onKeyReleased = [](cocos2d::EventKeyboard::KeyCode kc, cocos2d::Event * event) {
		switch (kc) {
		case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
			LIVE.press()[GameKeyPress::buttonUp] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			LIVE.press()[GameKeyPress::buttonLeft] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			LIVE.press()[GameKeyPress::buttonDown] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			LIVE.press()[GameKeyPress::buttonRight] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_Z:
			LIVE.press()[GameKeyPress::buttonA] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_X:
			LIVE.press()[GameKeyPress::buttonB] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_C:
			LIVE.press()[GameKeyPress::buttonC] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_U:
			LIVE.press()[GameKeyPress::buttonU] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_I:
			LIVE.press()[GameKeyPress::buttonI] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_O:
			LIVE.press()[GameKeyPress::buttonO] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_P:
			LIVE.press()[GameKeyPress::buttonP] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_N:
			LIVE.press()[GameKeyPress::buttonN] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_M:
			LIVE.press()[GameKeyPress::buttonM] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_ENTER:
			LIVE.press()[GameKeyPress::buttonStart] = false;
			break;
		case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
			LIVE.press()[GameKeyPress::buttonSpace] = false;
			break;
		default:
			break;
		}
    };
    cocos2d::Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(el, 1);
}


BlockPos GameLiveScene::nextVectorToApproachALine(const BlockPos& lineTarget, const BlockPos& now) {
	// 这要分八种情况啊。
	return BlockPos::zero;
}

GameLiveScene::~GameLiveScene() {
	PAINT.nodeRelease(codeRoot);
	if (blockMap != nullptr)
		delete[] blockMap;
}

void GameTime::timeAdd(int _minutes) {
	this->minute += _minutes;
	if (this->minute > BASE.MINUTES_IN_HOUR) {
		int hours_ = this->minute / BASE.MINUTES_IN_HOUR;
		this->minute -= hours_ * BASE.MINUTES_IN_HOUR;
		hourAdd(hours_);
	}
}

void GameTime::hourAdd(int _hours) {
	this->hour += _hours;
	if (this->hour > BASE.HOUR_IN_DAY) {
		int days_ = this->hour / BASE.HOUR_IN_DAY;
		this->hour -= days_ * BASE.HOUR_IN_DAY;
		dayAdd(days_);
	}
}

void GameTime::dayAdd(int _days) {
	cocos2d::log("DateAdd Before %d-%d-%d %d:%d", this->year, this->season, this->date, this->hour, this->minute);
	this->date += _days;
	if (this->date > BASE.DAYS_IN_SEASON) {
		int seasons_ = this->date / BASE.DAYS_IN_SEASON;
		this->date -= seasons_ * BASE.DAYS_IN_SEASON;
		seasonAdd(seasons_);
	}
	cocos2d::log("DateAdd After %d-%d-%d %d:%d", this->year, this->season, this->date, this->hour, this->minute);
}

void GameTime::seasonAdd(int _seasons) {
	this->season += _seasons;
	if (this->season > SeasonType::fuyu) {
		int years_ = this->season / 4;
		this->year += years_;
		this->season -= years_ * 4;
	}
}

GameLivePlant::GameLivePlant(BaseCode plant, BaseCode plantScene, const BlockPos& plantPos) {
	this->_plant = plant;
	this->_plantScene = plantScene;
	this->_plantPos = plantPos;
	this->_plantTime = LIVE.api_getGameTime();
	if (_refresh)
		this->levelRefresh();
}

void GameLivePlant::levelRefresh() {
	auto tmp = BASE.getPlant(_plant);
	if (tmp)
		_stage = tmp->levelUp(_water, _sun, LIVE.api_getSeason());
}

const string& GameLivePlant::getCSB() const {
	auto tmp = BASE.getPlant(_plant);
	if (tmp)
		return tmp->getCSB(this->_stage);
	else
		return BASE.EMPTY_STRING;
}

ObjPtr GameLivePlant::getObject() const {
	auto tmp = BASE.getPlant(_plant);
	if (tmp)
		return tmp->getObject(this->_stage);
	else {
		cocos2d::log("[ERROR] GameLivePlant::getObject : plant %d at stage %d", _plant, _stage);
		return nullptr;
	}
}

void GameLivePlant::onSceneClose(GameLiveScene* oldscene) {
	if (oldscene == nullptr || this->_plantScene != oldscene->getCode())
		return;
	auto padding = BASE.getPlant(this->_plant)->getObject(this->_stage)->padding();
	if (this->_added == false && oldscene->stuffExistsOn(soilWateredCode, this->getMargin() + padding)) {
		this->add(1, 1);
		this->_added = true;
	}
}

void GameLivePlant::onSceneCreate(GameLiveScene* newscene) {
	if (newscene == nullptr || this->_plantScene != newscene->getCode())
		return;
	newscene->add(this->getObject(), this->_plantPos, true);
	if (this->_added)
		newscene->findReplace(soilOriginCode, BASE.getStuff(soilWateredCode), this->_plantPos, true);
	else
		newscene->findReplace(soilOriginCode, BASE.getStuff(soilHoedCode), this->_plantPos, true);
}

void GameLivePlant::onDayPass() {
	this->_added = false;
	if (this->_refresh) {
		levelRefresh();
	}
}

GameLiveHuman::GameLiveHuman(BaseCode humancode) {
	this->copyFrom(BASE.getHuman(humancode));
}

void GameLiveHuman::copyFrom(HumanPtr human) {
	if (human == nullptr)
		return;
	this->_energy = human->fullEnergy;
	this->_name = human->name;
	// 未完待续 TODO
}

LiveCode GameStoryLoader::next() {
	this->stop();
	auto ptEle = this->_content.next(_index);
	if (ptEle == nullptr) {
		LIVE.api_stopStory(this);
		return nullptr;
	}
	return this->_activeNode = ptEle->action(*this);
}

void GameStoryLoader::stop() {
	auto ptEle = this->_content.now(_index);
	if (ptEle != nullptr) {
		ptEle->stop(*this);
		this->_activeNode = nullptr;
	}
}

void GameStoryLoader::end() {
	LIVE.api_stopStory(this);
}

int GameLive::keyAddTime() {
	int cnt = 0;
	for (int i = 0; i < KEY_COUNT; i++) {
		if (_press[i]) {
			_keys[i] += _loopfreq;
			cnt++;
		}
		else {
			_keys[i] = 0;
		}
	}
	return cnt;
}

const string GameLive::KEY_LOOP_NAME = "keyboardLoop";

void GameLive::keyLoop() {
    auto judgeSch = [](float dt) {
		if (LIVE._close)
            cocos2d::Director::getInstance()->getScheduler()->unschedule(KEY_LOOP_NAME, &LIVE);
		else {
			int keycnt = LIVE.keyAddTime();
			if (LIVE.noPressStillJudge || keycnt)
				LIVE.judge();
		}
    };
	cocos2d::Director::getInstance()->getScheduler()->schedule(judgeSch, &LIVE, _loopfreq, false, KEY_LOOP_NAME);
}

void GameLive::init() {
	if (_keys == nullptr) {
		_keys = new float[KEY_COUNT];
		for (int i = 0; i < KEY_COUNT; i++)
			_keys[i] = 0.0f;
	}
    if (_keys == nullptr)
        return;
	if (_press == nullptr) {
		_press = new bool[KEY_COUNT];
		for (int i = 0; i < KEY_COUNT; i++)
			_press[i] = false;
	}
	if (_keys == nullptr) {
		return;
	}

	this->_creature->init();
    this->keySet();
    this->enter();
    this->keyLoop();
}

bool GameLive::api_setSceneSize(const BlockPos& size) {
	if (this->_scene == nullptr)
		return false;
	else {
		this->_scene->setWindowSize(size);
		return true;
	}
}

LiveCode GameLive::api_UIStart(BaseCode uicode) {
    UIPtr uip = BASE.getUI(uicode);
    return api_UIStart(uip);
}

LiveCode GameLive::api_UIStart(UIPtr uip) {
    if (uip == nullptr)
        return nullptr;
    LiveUIPtr glu(new GameLiveUI(uip));
    if (uip->type() == GameUI::up) {
        this->_UIUp.push_back(glu);
    } else if (uip->type() == GameUI::down) {
        this->_UIDown.push_back(glu);
    }
    glu->id() = glu->UI()->start(); // 所以就这样子直接调用了？不知道。
	if (glu->id() != nullptr)
		glu->id()->setLocalZOrder(_UIUp.size() + _UIDown.size() + 1);
	PAINT.nodeDisplay(glu->id());
	return glu->id();
}

vector<LiveUIPtr>::iterator GameLive::_UIPtrQuery(LiveCode id, GameUI::UIType& out_type) {
	out_type = GameUI::empty;
	for (auto lt = this->_UIUp.begin(); lt != this->_UIUp.end(); lt++) {
		if (*lt != nullptr && (*lt)->id() == id) {
			out_type = GameUI::up;
			return lt;
		}
	}
	for (auto lt = this->_UIDown.begin(); lt != this->_UIDown.end(); lt++) {
		if (*lt != nullptr && (*lt)->id() == id) {
			out_type = GameUI::down;
			return lt;
		}
	}
	return this->_UIUp.end();
}

vector<LiveUIPtr>::iterator GameLive::_UIPtrQuery(BaseCode code, GameUI::UIType& out_type) {
	out_type = GameUI::empty;
	UIPtr uip = BASE.getUI(code);
	if (uip->type() == GameUI::up) {
		for (auto lt = this->_UIUp.begin(); lt != this->_UIUp.end(); lt++) {
			if (*lt != nullptr && (*lt)->UI()->code() == code) {
				out_type = GameUI::up;
				return lt;
			}
		}
	}
	else if (uip->type() == GameUI::down) {
		for (auto lt = this->_UIDown.begin(); lt != this->_UIDown.end(); lt++) {
			if (*lt != nullptr && (*lt)->UI()->code() == code) {
				out_type = GameUI::down;
				return lt;
			}
		}
	}
	return this->_UIUp.end();
}

void GameLive::api_UIStop(LiveCode id) {
	GameUI::UIType type = GameUI::UIType::empty;
	auto lt = _UIPtrQuery(id, type);
	if (type != GameUI::empty && *lt != nullptr){
		(*lt)->UI()->stop(id);
		PAINT.objRemove((*lt)->id(), PAINT.mainsc);
	}
	if (type == GameUI::empty)
		return;
	else if (type == GameUI::up)
		this->_UIUp.erase(lt);
	else if(type == GameUI::down)
		this->_UIDown.erase(lt);
}

void GameLive::api_UIStop(BaseCode id) {
	GameUI::UIType type = GameUI::UIType::empty;
	auto lt = _UIPtrQuery(id, type);
	if (type != GameUI::empty && *lt != nullptr){
		(*lt)->UI()->stop((*lt)->id());
		PAINT.objRemove((*lt)->id(), PAINT.mainsc);
	}
	if (type == GameUI::empty)
		return;
	else if (type == GameUI::up)
		this->_UIUp.erase(lt);
	else if (type == GameUI::down)
		this->_UIDown.erase(lt);
}

LiveUIPtr GameLive::api_getUI(BaseCode code) {
	GameUI::UIType tmp;
	auto lt = _UIPtrQuery(code, tmp);
	if (tmp == GameUI::UIType::empty)
		return nullptr;
	else
		return *lt;
}

void GameLive::api_eventStart(BaseCode eveCode, LiveObjPtr obj) {
	EventPtr eve = BASE.getEvent(eveCode);
	api_eventStart(eve, obj);
}

void GameLive::api_eventStart(EventPtr eve, LiveObjPtr obj) {
    if (eve == nullptr)
        return;
    else {
        auto tmp = eve->start(obj); // 所以就这样子直接调用了？不知道。
    }
}

bool GameLive::api_sceneSwitch(BaseCode sceneCode) {
	if (this->_scene->getCode() == sceneCode)
		return true;
	for (auto psc : this->_archiveScene) {
		if (psc->getCode() == sceneCode && psc != nullptr) {
			this->_scene = psc;
			return true;
		}
	}
	this->_scene = nullptr;
	return false;
}

void GameLive::api_sceneInit(BaseCode sceneCode) {
	if (api_sceneSwitch(sceneCode)) {
		this->_scene->resetScene();
	}
	else {
		this->_scene = new GameLiveScene();
		ObjPtr scene = BASE.getScene(sceneCode);
		if (scene == nullptr)
			return;
		this->_archiveScene.push_back(this->_scene);
		this->_scene->init(scene->getMixedSize());
		this->_scene->setScene(sceneCode);
	}
}

void GameLive::api_sceneDisplay() {
	this->_scene->rootCode()->setLocalZOrder(0);
	PAINT.nodeDisplay(this->_scene->rootCode());
}

void GameLive::api_sceneCalculate() {
	if (this->_creature)
		this->_creature->onSceneCreate(this->_scene);
	// TODO npc position calculate, scene object calculate, and more
}

void GameLive::api_sceneICD(BaseCode sceneCode, const BlockPos& windowSize) {
	api_sceneInit(sceneCode);
	api_sceneCalculate();
	api_sceneDisplay();
	api_setSceneSize(windowSize);
}

void GameLive::api_sceneSCD(BaseCode sceneCode, const BlockPos& windowSize) {
	if (api_sceneSwitch(sceneCode)) {
		api_sceneCalculate();
		api_sceneDisplay();
		api_setSceneSize(windowSize);
	}
	else
		api_sceneICD(sceneCode, windowSize);
}

void GameLive::api_sceneDCD(BaseCode sceneCode, const BlockPos& windowSize) {
	if (api_sceneSwitch(sceneCode)) {
		auto &settime = _scene->setTime;
		auto nowtime = api_getGameTime();
		if (settime.year == nowtime.year && settime.season == nowtime.season && settime.date == nowtime.date) {
			api_sceneSCD(sceneCode, windowSize);
		}
		else
			api_sceneICD(sceneCode, windowSize);
	}
	else
		api_sceneICD(sceneCode, windowSize);
}

void GameLive::api_sceneClose() {
	if (this->_scene != nullptr) {
		if (this->_creature)
			this->_creature->onSceneClose(this->_scene);
		PAINT.nodeRetain(this->_scene->rootCode());
		PAINT.nodeRemove(this->_scene->rootCode());
	}
}

void GameLive::api_sceneChange(BaseCode sceneCode, const BlockPos& windowSize, bool kidSet, const BlockPos& kidposnew) {
	if (this->_scene) {
		api_allDim(true, 0.5);
		CocoFunc sch = [this, sceneCode, windowSize, kidSet, kidposnew](float) {
			LiveObjPtr human = LIVE.api_humanGet(kidHumanCode);
			bool flag = false;
			BaseCode stuff;
			BlockPos::Direction facing;
			bool focus = false;
			if (human != nullptr) {
				flag = true && kidSet;
				stuff = human->getObj()->code();
				focus = this->_scene->isFocusOnHuman(kidHumanCode);
				facing = human->getFace();
				api_humanRemove(kidHumanCode, true);
			}

			api_sceneClose();
			api_sceneDCD(sceneCode, windowSize);

			if (flag) {
				LiveObjPtr kid2 = api_kidSet(stuff, kidposnew, focus);
				kid2->setFace(kid2, facing);
			}

			api_allDimFrom(true, 0.5);
		};
		api_autoLock(this->_animeUntil, 1.5f, doAfter, sch, "Scene Change", 0.7f);
	}
}

void GameLive::trashClear() {
	unsigned int sz = _trash.size();
	for (unsigned int i = 0; i < sz; i++)
		delete _trash[i]; 
	_trash.clear(); 
}

void GameLive::api_trashClear() {
	std::thread t1(std::bind(&GameLive::trashClear, this));
	t1.detach();
}

void GameLive::api_dayPass() {
	this->_time.dayAdd();
	if (this->_scene) {
		// 这个目前非常地混乱粗糙，还原度很低，其实是有很大一部分应该放在GameLiveHuman里面完成的，比如人消失时候的数据保存和再现
		// dim
		api_allDim(true, 1);
		CocoFunc sch = [this](float dt) {
			//kid capture
			auto kid = api_kidGet();
			bool flag = false;
			BlockPos kidpos;
			BaseCode stuff;
			bool focus = false;
			if (kid != nullptr) {
				flag = true;
				stuff = kid->getObj()->code();
				kidpos = kid->MP();
				focus = this->_scene->isFocusOnHuman(kidHumanCode);
			}

			// close and init
			auto sccode = this->_scene->getCode();
			auto mazesize = this->_scene->mazeSize;
			auto windowsize = this->_scene->windowSize;
			api_sceneClose();
			if (this->_creature) {
				this->_creature->onDayPass();
			}
			api_sceneICD(sccode, windowsize);

			//kid restore
			if (flag == true) {
				//api_kidSet(stuff, (BlockPos)BigBlockPos(13, 6), focus);
				api_kidSet(stuff, kidpos, focus);
			}

			//dim back
			api_allDimFrom(true, 1);
		};
		api_autoLock(this->_animeUntil, 3, doAfter, sch, "Day Pass", 1.1f);
		api_trashClear();
	}
}

void GameLive::api_removeDim(float delaySec) {
	if (this->_codeDim == nullptr) {
		this->_codeDim = PAINT.nodeNew();
		_codeDim->setLocalZOrder(40000);
		PAINT.nodeDisplay(_codeDim);
	}
	if (delaySec == 0)
		this->_codeDim->removeAllChildren();
	else {
		auto dim = this->_codeDim;
		CocoFunc sch = [dim](float dt) {
			dim->removeAllChildren();
		};
		GameLive::api_autoLock(this->_animeUntil, 0, doAfter, sch, "LiveDimRemove", delaySec);
	}
}

void GameLive::api_allDim(bool black, float timeSec) {
	api_removeDim();
	auto color = black ? cocos2d::Color4B(0, 0, 0, 0) : cocos2d::Color4B(255, 255, 255, 0);
	auto colorLayer = cocos2d::LayerColor::create(color);
	auto effects = this->_codeDim;
	effects->addChild(colorLayer);
	PAINT.objAlpha(colorLayer, 0, 1, timeSec, 0);
}

void GameLive::api_allDimFrom(bool black, float timeSec) {
	api_removeDim();
	auto color = black ? cocos2d::Color4B(0, 0, 0, 255) : cocos2d::Color4B(255, 255, 255, 255);
	auto colorLayer = cocos2d::LayerColor::create(color);
	auto effects = this->_codeDim;
	effects->addChild(colorLayer);
	PAINT.objAlpha(colorLayer, 1, 0, timeSec, 0);
}


LiveObjPtr GameLive::api_humanSet(BaseCode humancode, BaseCode stuffCode, const BlockPos& pos, bool focus) {
	ObjPtr pt = BASE.getStuff(stuffCode);
	return api_humanSet(humancode, pt, pos, focus);
}

LiveObjPtr GameLive::api_humanSet(BaseCode humancode, ObjPtr ptr, const BlockPos& pos, bool focus, bool copy) {
	LiveObjPtr result = nullptr;
	if (this->_scene != nullptr) {
		this->_creature->setHumanPtr(humancode, result = this->_scene->humanSet(humancode, ptr, pos, copy));
		auto human = api_getHuman(humancode);
		if (human != nullptr)
			human->setHandLive(LIVE.api_humanAddObject(kidHumanCode, human->getHand(), BlockPos::zero, false, pickZValue(ptr)));
		if (focus)
			this->_scene->setFocus(api_humanGet(humancode));
	}
	return result;
}

LiveObjPtr GameLive::api_kidSet(BaseCode stuffCode, const BlockPos& pos, bool focus) {
	return api_humanSet(kidHumanCode, stuffCode, pos, focus);
}

LiveObjPtr GameLive::api_kidSet(ObjPtr ptr, const BlockPos& pos, bool focus, bool copy) {
	return api_humanSet(kidHumanCode, ptr, pos, focus);
}

void GameLive::api_humanWalk(BaseCode humancode, const BlockPos& vec, bool faceChange) {
	if (this->_scene != nullptr) {
		this->_scene->humanWalk(humancode, vec, faceChange);
	}
}

void GameLive::api_humanRun(BaseCode humancode, const BlockPos& vec, bool faceChange) {
	if (this->_scene != nullptr) {
		this->_scene->humanRun(humancode, vec, faceChange);
	}
}

void GameLive::api_kidWalk(const BlockPos& vec, bool faceChange) {
	api_humanWalk(kidHumanCode, vec, faceChange);
}

void GameLive::api_kidRun(const BlockPos& vec, bool faceChange) {
	api_humanRun(kidHumanCode, vec, faceChange);
}

LiveObjPtr GameLive::api_addObject(BaseCode code, const BlockPos& margin, bool autoAdd, int z) {
	return api_addObject(BASE.getStuff(code), margin, autoAdd, true, z);
}

LiveObjPtr GameLive::api_addObject(ObjPtr object, const BlockPos& margin, bool autoAdd, bool copy, int z) {
	if (this->_scene != nullptr) {
		if (autoAdd) {
			BaseCode sctemp = api_getSceneCode();
			if (this->_creature 
				&& object->code() > plantStuffStart
				&& object->code() < plantStuffEnd
				&& sctemp > sceneStart
				&& sctemp < sceneEnd) {
				PlantCode pc = BASE.stuffToPlant(object->code());
				if (this->_creature->addPlant(pc, sctemp, margin) == false) {
					cocos2d::log("[FAILED] add object: plant");
					return nullptr;
				}
			}
		}
		return this->_scene->add(object, margin, copy, z);
	}
	else {
		cocos2d::log("[FAILED] add object: no scene");
		return nullptr;
	}
}

void GameLive::api_replaceObject(LiveObjPtr obj, ObjPtr ptr) {
	if (this->_scene != nullptr) {
		this->_scene->replace(obj, ptr);
	}
}

void GameLive::api_humanMoveStep(BaseCode humancode, BlockPos::Direction dir, LockType lock, bool faceChange, bool walk) {
	if (this->_scene != nullptr) {
		LiveObjPtr human = LIVE.api_humanGet(humancode);
		if (human == nullptr)
			return;
		BlockPos dist = this->_scene->getStepDist(dir);
		float timeSec = LIVE.api_getScenePtr()->getWalkTime(dist);

		LiveObjPtr jumpLayer;
		auto detres = _scene->detect(human, dist, dir, jumpLayer);
		if (detres == Walkable::canWalk) {
			CocoFunc sch = [humancode, dist, faceChange, walk](float dt) {
				if (walk)
					LIVE.api_humanWalk(humancode, dist, faceChange);
				else
					LIVE.api_humanRun(humancode, dist, faceChange);
			};
			LIVE.api_autoActionLock(human, timeSec, lock, sch, "kidWalk");
		}
		else if (detres == Walkable::jump) {
			if (humancode == kidHumanCode) {
				BaseCode scenecode;
				BlockPos kidposnew;
				bool doOrNot = _scene->getJump(human, dist, jumpLayer, scenecode, kidposnew);
				if (doOrNot) {
					CocoFunc sch = [this, scenecode, kidposnew](float dt) {
						api_sceneChange(scenecode, BASE.SCENE_DISPLAY_SIZE, true, kidposnew);
					};
					api_autoActionLock(human, 3, lock, sch, "func kid move scene change");
				}
				else
					return;
			}
			else {
				api_humanRemove(humancode, true);
			}
		}
		else if (detres == Walkable::noWalk) {
			if (faceChange)	{
				BlockPos::Direction facing = dist;
				human->setFace(human, facing);
			}
		}
		else {
		}
		//TODO slide?
	}
}

void GameLive::api_humanRemove(BaseCode humancode, bool recursive) {
	if (this->_scene) {
		this->_scene->humanRemove(humancode, recursive);
	}
}


void GameLive::api_kidMoveStep(BlockPos::Direction dir, bool faceChange, bool walk) {
	api_humanMoveStep(kidHumanCode, dir, doNothing, faceChange, walk);
}

int GameLive::pickZValue(ObjPtr human) {
	return human->getMixedSize().y;
}

BlockPos GameLive::pickPosition(LiveObjPtr human) {
	BlockPos pos2 = human->paintPos();
	pos2.y += pickZValue(human->getObj());
	return pos2;
}

BlockPos GameLive::dropPosition(LiveObjPtr human) {
	BlockPos pos2 = human->paintPos();
	pos2 += BlockPos::dirToBlock(human->getFace()) * BlockPos(BIG_TO_SMALL, BIG_TO_SMALL);
	return pos2;
}

void GameLive::pickAnimation(BaseCode humancode, LiveObjPtr stuff) {
	auto human = api_humanGet(humancode);
	if (human == nullptr || stuff == nullptr || human->getObj() == nullptr)
		return;
	BlockPos pos1 = stuff->paintPos();
	BlockPos pos2 = pickPosition(human);
	LiveCode cloud = _scene->codeCloud;
	LiveCode tempstuff = stuff->paint(cloud, 2);
	stuff->move(pos2 - pos1, linear, BASE.PICK_STUFF_TIME - 0.1f);

	CocoFunc sch = [tempstuff, cloud](float) {
		cloud->removeChild(tempstuff);
	};
	LIVE.api_autoAnimeLock(human, 0, doNothing, sch, "pick animation", BASE.PICK_STUFF_TIME);
}

void GameLive::dropAnimation(BaseCode humancode, LiveObjPtr stuff, const BlockPos& dropPosition) {
	auto human = api_humanGet(humancode);
	if (human == nullptr || stuff == nullptr || human->getObj() == nullptr)
		return;
	BlockPos pos1 = pickPosition(human);
	LiveCode cloud = _scene->codeCloud;

	auto oripos = stuff->margin();
	auto origin = stuff->paintCode();
	auto orialpha = stuff->alpha();

	stuff->alpha() = 1;
	stuff->margin() = pos1;
	LiveCode tempstuff = stuff->paint(cloud, 2);
	stuff->move(dropPosition - pos1, linear, BASE.DROP_STUFF_TIME - 0.1f);

	stuff->_paintCode = origin;
	stuff->margin() = oripos;
	stuff->alpha() = orialpha;

	CocoFunc sch = [tempstuff, cloud](float) {
		cloud->removeChild(tempstuff);
	};
	LIVE.api_autoAnimeLock(human, 0, doNothing, sch, "drop animation", BASE.DROP_STUFF_TIME);
}

GameLive::ActionResult GameLive::api_humanPick(BaseCode humancode, LiveObjPtr stuff) {
	auto humanlive = api_humanGet(humancode);
	TimeFunc sche = [stuff, humancode, humanlive, this]() {
		auto human = api_getHuman(humancode);
		if (humanlive == nullptr
			|| humanlive->getObj() == nullptr
			|| human == nullptr
			|| stuff == nullptr
			|| human->pickHand(stuff->getObj()) == false) {
			return GameLive::TIMEFUNC_ROLLBACK;
		}
		LiveObjPtr temp;
		if ((temp = human->getHandLive()) != nullptr) {
			api_removeObject(temp);
			human->setHandLive(nullptr);
		}
		api_removeObject(stuff);
		pickAnimation(humancode, stuff);
		BlockPos pos = humanlive->MP();
		int zValue = pickZValue(humanlive->getObj());
		ObjPtr ptr = stuff->getObj();
		CocoFunc sch = [zValue, pos, ptr, human](float dt) {
			human->setHandLive(LIVE.api_humanAddObject(kidHumanCode, ptr, BlockPos::zero, false, zValue));
		};
		api_autoActionLock(humanlive, 0, doAfter, sch, "pick2", 0);
		return 0;
	}; 
	int tempres = 0;
	bool timepass = api_delegateActionTimeCompare(humanlive, BASE.PICK_STUFF_TIME, sche, tempres, "pick1");

	if (timepass)
		if (tempres == TIMEFUNC_ROLLBACK)
			return ActionResult::cannotDo;
		else
			return ActionResult::done;
	else
		return ActionResult::timeBlocked;
}

GameLive::ActionResult GameLive::api_humanDrop(BaseCode humancode) {
	auto humanlive = api_humanGet(humancode);
	TimeFunc sche = [humanlive, humancode, this]() {
		auto human = api_getHuman(humancode);
		if (humanlive == nullptr
			|| humanlive->getObj() == nullptr
			|| human == nullptr)
			return GameLive::TIMEFUNC_ROLLBACK;

		ObjPtr obj = human->dropHand();
		// 如果手上空了的话
		if (human->handIsEmpty()) {
			api_removeObject(human->getHandLive());
			human->setHandLive(nullptr);
		}
		if (obj == nullptr)
			return GameLive::TIMEFUNC_ROLLBACK;

		BlockPos pos2 = dropPosition(humanlive);
		LiveObjPtr objlive = api_addObject(obj, pos2, true, false, 0);
		if (objlive == nullptr)
			return GameLive::TIMEFUNC_ROLLBACK;

		api_removeObject(objlive);
		dropAnimation(humancode, objlive, pos2);
		CocoFunc sch = [obj, pos2, this](float) {
			api_addObject(obj, pos2, true, false, 0);
		};
		api_autoActionLock(humanlive, 0, doAfter, sch, "drop2", 0);
		return 0;
	};
	int tempres = 0;
	bool timepass = api_delegateActionTimeCompare(humanlive, BASE.DROP_STUFF_TIME, sche, tempres, "drop1");

	if (timepass)
		if (tempres == TIMEFUNC_ROLLBACK)
			return ActionResult::cannotDo;
		else
			return ActionResult::done;
	else
		return ActionResult::timeBlocked;
}

GameLive::ActionResult GameLive::api_humanPutIntoPack(BaseCode humancode) {
	LiveObjPtr live = api_humanGet(humancode);
	if (live == nullptr)
		return ActionResult::cannotDo;
	int out_;
	TimeFunc sch = [this, humancode]() {
		auto humanp = api_getHuman(humancode);
		if (humanp == nullptr)
			return TIMEFUNC_ROLLBACK;
		if (humanp->putIntoPack() == nullptr)
			return TIMEFUNC_ROLLBACK;
		else {
			if (humanp->getHandLive() != nullptr)
				LIVE.api_removeObject(humanp->getHandLive());
			return 0;
		}
	};
	if (api_delegateActionTimeCompare(live, 0.5, sch, out_, "putPack") == false)
		return ActionResult::timeBlocked;
	else if (out_ == TIMEFUNC_ROLLBACK)
		return ActionResult::cannotDo;
	else
		return ActionResult::done;
}

GameLive::ActionResult GameLive::api_kidPick(LiveObjPtr stuff) {
	return api_humanPick(kidHumanCode, stuff);
}

LiveObjPtr GameLive::api_humanAddObject(BaseCode humancode, ObjPtr obj, const BlockPos& marginRelative, bool copy, int z) {
	if (this->_scene && obj != nullptr)
		return this->_scene->humanAddObject(humancode, obj, marginRelative, copy, z);
	return nullptr;
}

void GameLive::api_moveObject(LiveObjPtr obj, const BlockPos& vec, MoveType type, float timeSec, bool recursive, bool faceChange) {
	if (this->_scene) {
		this->_scene->movemove(obj, vec, type, timeSec, recursive, faceChange);
	}
}

void GameLive::api_changePicture(LiveObjPtr obj, const string& newcsb) {
	if (this->_scene) {
		this->_scene->changePicture(obj, newcsb);
	}
}

void GameLive::api_removeObject(LiveObjPtr object, bool recursive, bool autoRemove) {
	if (this->_scene && object != nullptr) {
		if (autoRemove) {
			BaseCode sctemp = api_getSceneCode();
			if (this->_creature
				&& object->getObj()->code() > plantStuffStart
				&& object->getObj()->code() < plantStuffEnd
				&& sctemp > sceneStart
				&& sctemp < sceneEnd) {
				this->_creature->removePlant(sctemp, object->margin());
			}
			this->_scene->remove(object, recursive);
		}
	}
}

bool GameLive::api_hasPlant(BaseCode sceneCode, const BlockPos& position) {
	if (this->_creature && sceneCode > sceneStart && sceneCode < sceneEnd)
		return this->_creature->hasPlant(sceneCode, position);
	else
		return false;
}

void GameLive::api_toolLose(BaseCode humanCode) {
	if (this->_creature) {
		this->_creature->getHuman(humanCode)->toolLoseStep();
		auto tempui = this->api_getUI(toolUICode);
		if (tempui != nullptr && tempui->UI() != nullptr) {
			auto tool = std::dynamic_pointer_cast<ToolUI>(tempui->UI());
			tool->toolPaint(tempui->id());
		}
	}
}

GameStoryLoader* GameLive::api_startStory(BaseCode storyCode) {
	GameStory* sto = BASE.getStory(storyCode);
	if (sto == nullptr)
		return nullptr;
	return api_startStory(*sto);
}

GameStoryLoader* GameLive::api_startStory(const GameStory& content) {
	GameStoryLoader* loader = new GameStoryLoader(content);
	if (loader == nullptr)
		return nullptr;
	this->_story.push_back(loader);
	loader->next();
	return loader;
}

bool GameLive::api_stopStory(GameStoryLoader* loader) {
	if (loader != nullptr) {
		loader->stop();
		for (auto lt = _story.begin(); lt != _story.end();) {
			if (*lt == loader) {
				lt = _story.erase(lt);
				delete loader;
				return true;
			}
			else
				lt++;
		}
		return false;
	}
	return false;
}

void GameLive::api_delayTime(const CocoFunc& func, float delaySec, const string& key,void* refer, int repeat, int interval) {
	if (refer == nullptr)
		refer = &LIVE;
	if (delaySec == 0 && repeat == 0) {
		func(0);
	}
	else {
		cocos2d::Director::getInstance()->getScheduler()->schedule(func, refer, interval, repeat, delaySec, false, key);
	}
}
void GameLive::api_undelay(void* refer, const string &key) {
	if (refer == nullptr)
		refer = &LIVE;
	cocos2d::Director::getInstance()->getScheduler()->unschedule(key, refer);
}

void GameLive::api_undelayAll(void* refer) {
	if (refer == nullptr)
		refer = &LIVE;
	cocos2d::Director::getInstance()->getScheduler()->unscheduleAllForTarget(refer);
}

void GameLive::api_addActionLock(LiveObjPtr obj, float lockAdd) {
	float now = PAINT.clock();
	float loc = obj->getActionLock();
	obj->setActionLock(loc > now ? loc : now + lockAdd);
}

bool GameLive::api_autoLock(float& lockValue, float timeSec, LockType type, const CocoFunc& func, const string& key, float delaySec, int repeat, int interval) {
	// 规则有点麻烦不是很想动手，要融合各种情况的，查询和可能，还有延时什么的。讨论下来新的locktime需要立刻修改完，别的什么倒是可以延时操作没有冲突
	// 完成啦！
	// 返回有没有执行操作
	// LockType: 
	// doAfter动作追加
	// doBreak从现在开始经过delaySec后清空任务立刻执行
	// doClearBreak现在清空开始计时
	// doOverwrite直接复写
	// doNothing现在lock时不操作
	float now = PAINT.clock();
	float* lockptr = &lockValue;
	if (now + delaySec >= lockValue || type == LockType::doAfter) {
		float delay = delaySec;
		if (now >= lockValue)
			lockValue = now;
		else
			delay += lockValue - now;
		lockValue += delaySec + timeSec;
		cocos2d::log("[Succeeded] %s @%f : %f + %f -> %f <%d, %d>", key.c_str(), now, timeSec, delaySec, lockValue, repeat, interval);
		LIVE.api_delayTime(func, delay, key, lockptr, repeat, interval);
		return true;
	}
	else { // 这里lockValue一定大于now+delaySec
		if (type == LockType::doBreak) {
			lockValue = now + delaySec + timeSec;
			cocos2d::log("[Succeeded] %s @ %f : %f + %f -> %f <%d, %d>", key.c_str(), now, timeSec, delaySec, lockValue, repeat, interval);
			CocoFunc sch = [func, key, repeat, interval, lockptr](float dt) {
				LIVE.api_undelayAll(lockptr);
				LIVE.api_delayTime(func, 0, key, lockptr, repeat, interval);
			};
			LIVE.api_delayTime(sch, delaySec, "AutoLock" + key, lockptr);
			return true;
		}
		else if (type == LockType::doClearBreak || type == LockType::doOverwrite) {
			lockValue = now + delaySec + timeSec;
			cocos2d::log("[Succeeded] %s @ %f : %f + %f -> %f <%d, %d>", key.c_str(), now, timeSec, delaySec, lockValue, repeat, interval);
			if (type == LockType::doClearBreak)
				LIVE.api_undelayAll(lockptr);
			LIVE.api_delayTime(func, delaySec, key, lockptr, repeat, interval);
			return true;
		}
		else if (type == LockType::doNothing) {
			cocos2d::log("[Cancelled] %s @ %f : %f + %f # %f <%d, %d>", key.c_str(), now, timeSec, delaySec, lockValue, repeat, interval);
			return false;
		}
		else {
			cocos2d::log("[Cancelled] %s @ %f : %f + %f # %f <%d, %d>", key.c_str(), now, timeSec, delaySec, lockValue, repeat, interval);
			return false;
		}
	}
}

bool GameLive::api_autoActionLock(LiveObjPtr obj, float timeSec, LockType type, const CocoFunc& func, const string& key, float delaySec, int repeat, int interval) {
	return LIVE.api_autoLock(obj->_lockUntil, timeSec, type, func, key, delaySec, repeat, interval);
}

bool GameLive::api_autoAnimeLock(LiveObjPtr obj, float timeSec, LockType type, const CocoFunc& func, const string& key, float delaySec, int repeat, int interval) {
	return LIVE.api_autoLock(obj->_animeUntil, timeSec, type, func, key, delaySec, repeat, interval);
}

bool GameLive::api_delegateTimeCompare(float& lockvalue, const float& timeSec, const TimeFunc& func, int& out_return, const string& logname) {
	float now = PAINT.clock();
	if (lockvalue > now) {
		cocos2d::log("[Cancelled] %s @ %f : + %f # %f", logname.c_str(), now, timeSec, lockvalue);
		return false;
	}
	else {
		lockvalue = now + timeSec;
		cocos2d::log("[Trying] %s @ %f : + %f -> %f", logname.c_str(), now, timeSec, lockvalue);
		out_return = func();
		if (out_return == TIMEFUNC_ROLLBACK) {
			lockvalue = now;
			cocos2d::log("[Rolling Back] -> %f", lockvalue);
		}
		else
			cocos2d::log("[Returning] %d", out_return);
		return true;
	}
}

bool GameLive::api_delegateActionTimeCompare(LiveObjPtr& obj, const float& timeSec, const TimeFunc& func, int& out_return, const string& logname) {
	if (obj != nullptr)
		return api_delegateTimeCompare(obj->_lockUntil, timeSec, func, out_return, logname);
	else
		return false;
}

bool GameLive::api_delegateAnimeTimeCompare(LiveObjPtr& obj, const float& timeSec, const TimeFunc& func, int& out_return, const string& logName) {
	if (obj != nullptr)
		return api_delegateTimeCompare(obj->_animeUntil, timeSec, func, out_return, logName);
	else
		return false;
}

LiveObjPtr GameObjectJudge::getHumanPtr()  {
	return _live->api_humanGet(this->_humancode); 
}

JudgeReturn GameObjectJudge::judge(float* keyarray, GameLiveScene& scene, vector<LiveObjPtr>& objects) {
	autoTranslate(keyarray);
step_two:
	for (auto ind = 0; ind < (int)objects.size(); ind++) {
		if (objects[ind] == nullptr)
			continue;
		setObject(objects[ind]);
		auto obj = getObjectPtrJudgedNow()->getObj();
		if (obj == nullptr)
			continue;
		GameCommand objcmd;
		if (obj->isCustomTranslate())
			objcmd = obj->translate(keyarray);
		else
			objcmd = getCmdCache();
		EventPtr eve;
		float timeSec, delaySec;
		LockType lock;
		JudgeReturn jud = obj->link(objcmd, eve, *this, timeSec, delaySec, lock);
		if (eve != nullptr) {
			GameLive* live = this->_live;
			CocoFunc sch = [live, eve](float) {
				live->api_eventStart(eve, nullptr);
			};
			_live->api_autoActionLock(this->getHumanPtr(), timeSec, lock, sch, "event" + std::to_string(getHumanCode()), delaySec);
		}
		switch (jud)
		{
		case judgeEnd:
			return jud;
			break;
		case judgeNextObject:
			continue;
			break;
		case judgeNextLayer:
			return jud;
			break;
		case judgePreviousObject:
			if (ind > 1)
				ind -= 2;
			break;
		case judgeObjectLayer:
			goto step_two;
			break;
		case judgeResetLayer:
			goto step_two;
			break;
		case judgeResetAll:
			return jud;
			break;
		default: // 相当于continue
			break;
		}
	}
	return judgeEnd;
}

void GameLive::judge() {
	GameObjectJudge objJudge(this, kidHumanCode, GameBase::DEFAULT_COMMAND);
step_one:
	for (int i = _UIUp.size() - 1; i >= 0; i--) {
		if (_UIUp[i] == nullptr)
			continue;
		else {
			EventPtr eve = nullptr;
			LiveUIPtr ptt = _UIUp[i];
			_UIJudgeNow = ptt;
			JudgeReturn jud = ptt->UI()->action(ptt->id(), this->_keys, objJudge);
			if (eve != nullptr) {
				api_eventStart(eve, nullptr);
			}
			if (jud == judgeEnd) {
				return;
			}
			else if (jud == judgeNextObject) {
				continue;
			}
			else if (jud == judgeNextLayer) {
				break;
			}
			else if (jud == judgePreviousObject) {
				i += 2;
				if (i >= (int)_UIUp.size())
					i -= 2; // 超出的话视作NextObject处理
			}
			else if (jud == judgeResetLayer || jud == judgeResetAll) {
				goto step_one;
			}
			else if (jud == judgeObjectLayer) {
				goto step_two;
			}
			else {
			}
		}
	}
	_UIJudgeNow = nullptr;

step_two:

	if (this->_scene) {
		vector<LiveObjPtr> objects;
		this->_scene->kidRangeObjects(objects);
		auto jud = objJudge.judge(this->keys(), *(this->_scene), objects);
		if (jud == judgeEnd)
			return;
		else if (jud == judgeNextLayer)
			goto step_three;
		else if (jud == judgeResetAll)
			goto step_one;
		else {
		}
	}

step_three:
    for (int i = _UIDown.size() - 1; i >= 0; i--) {
        if (_UIDown[i] == nullptr)
            continue;
        else {
            EventPtr eve = nullptr;
			LiveUIPtr ptt = _UIUp[i];
			_UIJudgeNow = ptt;
			JudgeReturn jud = ptt->UI()->action(ptt->id(), this->_keys, objJudge);
            if (eve != nullptr) {
                api_eventStart(eve, nullptr);
            }
            if (jud == judgeEnd) {
                return;
            } else if (jud == judgeNextObject) {
                continue;
            } else if (jud == judgeNextLayer) {
                break;
            } else if (jud == judgePreviousObject) {
                i += 2;
                if (i >= (int) _UIDown.size())
                    i -= 2; // 超出的话视作NextObject处理
			}
			else if (jud == judgeObjectLayer) {
				goto step_two;
			}
			else if (jud == judgeResetLayer) {
                goto step_three;
            } else if (jud == judgeResetAll) {
                goto step_one;
            } else {
            }
        }
	}
	_UIJudgeNow = nullptr;
}

bool GameLive::keyPushed(float* keyarray, GameKeyPress gkp, bool only) {
	if (only)
		return keyPushedOnly(keyarray, gkp);
	else {
		if (keyarray[gkp] > 0)
			return true;
		else
			return false;
	}
}
bool GameLive::keyPushed(float* keyarray, vector<GameKeyPress> vgkp, bool only) {
	if (only)
		return keyPushedOnly(keyarray, vgkp);
	else {
		for (unsigned int i = 0; i < vgkp.size(); i++)
			if (keyarray[vgkp[i]] == 0 || keyarray[vgkp[i]] < 0)
				return false;
		return true;
	}
}

bool GameLive::keyPushedOnly(float* keyarray, GameKeyPress gkp) {
	auto keys = keyarray;
	//pushed
	if (keys[gkp] <= _loopdevation)
		return false;
	//only
	for (auto i = GameKeyPress::buttonEmpty + 1; i < GameKeyPress::buttonEnd; ++i)
		if (i != gkp && keys[i] > _loopdevation)
			return false;
	return true;
}
bool GameLive::keyPushedOnly(float* keyarray, vector<GameKeyPress> vgkp) {
	auto keys = keyarray;
	//pushed
	for (unsigned int i = 0; i < vgkp.size(); i++)
		if (keys[vgkp[i]] <= _loopdevation)
			return false;
	//only
	int keyPressedNum = 0;
	for (auto i = GameKeyPress::buttonEmpty + 1; i < GameKeyPress::buttonEnd; ++i)
		if (keys[i] > _loopdevation)
			++keyPressedNum;
	return ((int)vgkp.size()) == keyPressedNum;
}

bool GameLive::keyJustPushed(float* keyarray, GameKeyPress gkp, bool only) {
	auto keys = keyarray;
	//pushed
	if (keys[gkp] < _loopfreq - _loopdevation || keys[gkp] > _loopfreq + _loopdevation) //考虑可以忽略的延迟时间?
		return false;
	//only
	if (only)
		for (auto i = GameKeyPress::buttonEmpty + 1; i < GameKeyPress::buttonEnd; ++i)
			if (i != gkp && (keys[i] >= _loopfreq - _loopdevation && keys[i] <= _loopfreq + _loopdevation))
				return false;
	return true;
}
bool GameLive::keyJustPushed(float* keyarray, vector<GameKeyPress> vgkp, bool only) {
	auto keys = keyarray;
	//pushed
	for (unsigned int i = 0; i < vgkp.size(); i++)
		if (keys[vgkp[i]] < _loopfreq - _loopdevation || keys[vgkp[i]] > _loopfreq + _loopdevation) //考虑可以忽略的延迟时间?
			return false;
	//only
	if (only) {
		int keyPressedNum = 0;
		for (auto i = GameKeyPress::buttonEmpty + 1; i < GameKeyPress::buttonEnd; ++i)
			if (keys[i] >= _loopfreq - _loopdevation && keys[i] <= _loopfreq + _loopdevation)
				++keyPressedNum;
		return ((int)vgkp.size()) == keyPressedNum;
	}
	else
		return true;
}
bool GameLive::_keyCyclePushed(float presstime, float cycleSec) {
	float time = std::fmod(presstime, cycleSec);
	if (time > _loopfreq - _loopdevation && time < _loopfreq + _loopdevation)
		return true;
	else
		return false;
}
bool GameLive::keyCyclePushed(float* keys, GameKeyPress gkp, float cycleSec, bool only) {
	//pushed
	if (!_keyCyclePushed(keys[gkp], cycleSec))
		return false;
	//only
	if (only)
		for (auto i = GameKeyPress::buttonEmpty + 1; i < GameKeyPress::buttonEnd; ++i)
			if (i != gkp && _keyCyclePushed(keys[i], cycleSec))
				return false;
	return true;
}
bool GameLive::keyCyclePushed(float* keys, vector<GameKeyPress> vgkp, float cycleSec, bool only) {
	//pushed
	for (const auto gkp : vgkp)
		if (!_keyCyclePushed(keys[gkp], cycleSec))
			return false;
	//auto
	if (only) {
		int keypressedcnt = 0;
		for (auto i = GameKeyPress::buttonEmpty + 1; i < GameKeyPress::buttonEnd; ++i)
			if (_keyCyclePushed(keys[i], cycleSec))
				keypressedcnt++;
		return ((int)vgkp.size()) == keypressedcnt;
	}
	else
		return true;
}


