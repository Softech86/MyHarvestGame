#include "GameLive.h"
#include "GameBase.h"
#include "GamePrincipal.h"
#include "cocos2d.h"
#include "GamePaint.h"
#include <cmath>
#include <list>

// <----->
void GameLive::enter() {
	api_UIStart(UICode::startPageCode);
}
// <----->

GameLiveObject::GameLiveObject(ObjPtr pobj) {
    if (pobj == nullptr) {
        _obj = ObjPtr(new GameObject(GameObject::origin));
    } else {
        _obj = ObjPtr(pobj->SHCP());
    }
}

GameLiveObject::GameLiveObject(ObjPtr pobj, const BlockPos& margin__)
: GameLiveObject::GameLiveObject(pobj) {
    this->margin() = margin__;
}

GameLiveObject::StickTo GameLiveObject::whereToStick(GameObject::BigType type) {
	switch (type)
	{
		break;
	case GameObject::background:
	case GameObject::ground:
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

const float GameLiveObject::layerOrderMultiplier = 0.03125f;
// 计算绘画的屏幕相对位置
BlockPos GameLiveObject::paintPos() {
    return this->MP() + PxPos(1, 0) * this->zValue();
}

float GameLiveObject::paintLayerOrder(int dotOrder) {
	if (dotOrder < 0)
		dotOrder = 0;
	return (float)(this->MP().y) + layerOrderMultiplier * dotOrder;
}

void GameLiveObject::setZOrder(float Zorder) {
	this->_order = Zorder;
	PAINT.objZOrder(this->paintCode(), Zorder);
	//cocos2d::log(("Z Order: " + std::to_string(Zorder)).c_str());
}

void GameLiveObject::setZOrder(float Zorder, float timeSec, float delaySec) {
	this->_order = Zorder;
	PAINT.objZOrder(this->paintCode(), this->_order, Zorder, timeSec, delaySec);
	//cocos2d::log(("Z Order: " + std::to_string(Zorder)).c_str());
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
		this->autoZOrder(dotOrder);
    }
	this->getObj()->afterPaint(result);
	this->paintCode() = result;
    return result;
}

void GameLiveObject::erase(LiveCode father) {
    PAINT.objRemove(this->paintCode(), father);
	this->paintCode() = nullptr;
}

void GameLiveObject::move(const BlockPos& vec, MoveType move, float timeSec, float delaySec) {
    BlockPos pos = this->paintPos();
	float now = PAINT.clock();
	float delay = 0;
	if (_movingUntil <= now) {
		_movingUntil = now;
	}
	delay = _movingUntil - now + delaySec;
	_movingUntil += delaySec + timeSec;
	PAINT.objMove(this->paintCode(), pos + vec, move, timeSec, delay);
}

// 这个目前的效果也就是按照最后动作执行完之后的pos来运动而已，就是跳过了中间的一点过程而已
void GameLiveObject::cleanMove(const BlockPos& vec, MoveType move, float timeSec, float delaySec) {
	BlockPos pos = this->paintPos();
	float now = PAINT.clock();
	_movingUntil = now + delaySec + timeSec;
	PAINT.objMove(this->paintCode(), pos + vec, move, timeSec, delaySec);
	// TODO
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

GameLiveUI::GameLiveUI(UIPtr ori) {
	if (ori == nullptr) {
		this->_ui = UIPtr(new GameUI(GameUI::origin));
	}
    else
        this->_ui = UIPtr(ori->SHCP());
}


void GameLiveObject::getRange(BlockPos& out_start, BlockPos& out_size) {
	if (this->rangeType == objectRelative) {
		BlockPos::Direction kidfacing = this->getFace();
		BlockPos start = this->MPC() + rangeCenter;
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
	RangeCache rc;
	rc.ptr = obj;
	rc.layer = layer;
	cache.push_back(rc);
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
					if ((*ld)[i] == itself)
						goto step_break_two;
				}
				for (int i = 0; i < (int)ld->size(); i++){
					LiveObjPtr ptr = (*ld)[i];
					if (ptr != this->liveScene) {
						if (useCache(cache, endlt, ptr, i, 5)) {
							auto mapfind = out_objects.find(ptr);
							if (mapfind == out_objects.end()){
								auto pa = std::pair<LiveObjPtr, int>(ptr, i);
								out_objects.insert(pa);
								cocos2d::log((std::to_string(ix) + ", " + std::to_string(iy) + ": " + ptr->getObj()->name()).c_str());
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
	if (this->liveKid == nullptr)
		return;
	BlockPos start, size;
	this->liveKid->getRange(start, size);
	map<LiveObjPtr, int> objects;
	rangeGetObjects(start, size, objects, this->liveKid);
	sortObjects(this->liveKid->MPC(), this->liveKid->getFace(), objects, out_result);
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
		if (insertPositionCompare(*i, obj) > 0)
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
        if (*i == nullptr) {
            i = ld.erase(i);
        } else if (*i == ptr) {
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
		if ((*ld)[i] == ptr)
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

void GameLiveScene::dictAdd(LiveCode code, LiveObjPtr obj) {
    this->dict.insert(std::pair<LiveCode, LiveObjPtr>(code, obj));
}

void GameLiveScene::dictRemove(LiveCode code) {
    auto lt = this->dict.find(code);
    if(lt != this->dict.end()) {
        this->dict.erase(lt);
    }
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
    return GameLiveScene::make(temp, margin, z, scale, alpha);
}

LiveObjPtr GameLiveScene::make(ObjPtr obj, const BlockPos& margin, int z, float scale, float alpha) {
	BlockPos mppd = BlockPos::zero;
    if(this->scene != nullptr)
		mppd = this->scene->padding();
	return make_(obj, margin, mppd, BlockPos::zero, z, scale, alpha);
}

LiveObjPtr GameLiveScene::make_(ObjPtr obj, const BlockPos& margin, const BlockPos& mappadding, BlockPos parentAdd, int z, float scale, float alpha) {
	LiveObjPtr pt(new GameLiveObject(obj, margin + parentAdd + (obj->type() == GameObject::BigType::background ? BlockPos::zero : mappadding)));
	pt->zValue() = z;
	pt->scale() = scale;
	pt->alpha() = alpha;
	pt->setStick(GameLiveObject::whereToStick(obj->type()));

	this->cacheAdd(pt);

	parentAdd += pt->margin() + (obj->type() == GameObject::BigType::background ? BlockPos::zero : mappadding);
	for (int index = 0; index < (int)obj->children().size(); index++) {
		LiveObjPtr livechild = make_(obj->children()[index].lock(), obj->childrenPos()[index], mappadding, parentAdd, z, scale, alpha);
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
	return parent;
}

void GameLiveScene::init(const BlockPos& mazeSize) {
    this->codeRoot = PAINT.nodeNew();
	// 这样的三句话也就确定了三个基本图层的顺序
    this->codeFlat = PAINT.objAddToObj(this->codeRoot, "", PxPos::zero);
    this->codeCube = PAINT.objAddToObj(this->codeRoot, "", PxPos::zero);
	this->codeCloud = PAINT.objAddToObj(this->codeRoot, "", PxPos::zero);
    
    this->mazeSize = mazeSize;
    this->blockMap = new LiveDot[mazeSize.x * mazeSize.y];

	this->defaultTranslator = BASE.getTranslator(basicObjectTranslator);
}

void GameLiveScene::setScene(BaseCode scene) {
	this->scene = BASE.getScene(scene);
	// scene.margin() always be zero
	this->liveScene = this->add(this->scene, BlockPos::zero);
	// viewPoint initialize to the left-under corner of the area you can see in this scene
	this->viewPoint = this->scene->padding();
}

LiveObjPtr GameLiveScene::add(ObjPtr obj, const BlockPos& margin) {
    LiveObjPtr live = make(obj, margin);
    add(live);
	return live;
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
    if(recursive) {
        mapRemove(live, true);
        for(auto childptr : live->outBind()) {
            remove(childptr.lock(), true);
        }
    } else {
        mapRemove(live, false);
    }
	live->erase(this->getParent(live));
    dictRemove(live->paintCode());
}

void GameLiveScene::movemove(LiveObjPtr ptr, const BlockPos& vec, MoveType move, float timeSec, bool recursive) {
	BlockPos oldpos = ptr->MP();
	ptr->move(vec, move, timeSec);

	int ind = mapMove(ptr, vec, false);
	ptr->autoZOrder(ind, timeSec, 0);
	BlockPos::Direction facing = vec;
	ptr->setFace(facing);

	if (recursive) {
		for (auto childptr : ptr->outBind())
			movemove(childptr.lock(), vec, move, timeSec, true);
	}

	if (this->getFocus() == ptr) {
		focusMoveViewPoint(ptr, oldpos, ptr->MP(), false, vec.distance() / timeSec);
	}
}

void GameLiveScene::replace(LiveObjPtr oldptr, ObjPtr newptr) {
	if (oldptr != nullptr && newptr != nullptr) {
		int indx = mapReplace(oldptr, newptr);
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
	}
	else if (newpos.x - winSze.x / 2 < scPad.x) {
		result.x = (int)(scPad.x);
	}
	else {
		result.x = (int)(newpos.x - winSze.x / 2);
	}
	if (newpos.y + winSze.y / 2 > scSze.y + scPad.y) {
		result.y = (int)(scSze.y + scPad.y - winSze.y);
	}
	else if (newpos.y - winSze.y / 2 < scPad.y) {
		result.y = (int)(scPad.y);
	}
	else {
		result.y = (int)(newpos.y - winSze.y / 2);
	}
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
	float time1 = PxPos::time(dist1, speed * SMALL_BLOCK_PX);
	float sqrSum = PxPos::distance(PxPos::zero, kidMove);
	float timeAll = PxPos::time(kidMove, speed * SMALL_BLOCK_PX);
	if (flash)
		setViewPoint(result);
	else if (result == cache) {
		moveViewPoint(result, 0, timeAll);
		return;
	}
	else {
		if (moveAll.x == 0 || moveAll.y == 0) {
			float time3;
			if (kidMove == PxPos::zero)
				time3 = 0;
			else if ((int)std::abs(moveAll.x) > 0) {
				time3 = std::abs(moveAll.x) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.x);
			}
			else {
				time3 = std::abs(moveAll.y) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.y);
			}
			moveViewPoint(result, time3, time1);

			if (timeAll - time3 - time1 > 0)
				moveViewPoint(result, 0, timeAll - time3 - time1);
		}
		else {
			PxPos onelineRela = oldRela + dist1;
			PxPos dist2 = PxPos::zero;
			GameLiveScene::LineReturn flag2 = DistanceToTheOtherLine(onelineRela, kidMove, dist2);
			PxPos move2 = distProcess(dist2, flag2, moveAll);
			float time2;
			if (kidMove == PxPos::zero)
				time2 = 0;
			else if ((int)std::abs(move2.x) > 0) {
				time2 = std::abs(move2.x) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.x);
			}
			else {
				time2 = std::abs(move2.y) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.y);
			}

			moveViewPoint(cache + move2, time2, time1);
			if (flag2 == NEVER) {
				if (timeAll - time2 - time1 > 0)
					moveViewPoint(result, 0, timeAll - time2 - time1);
				return;
			}
			else {
				float time25 = PxPos::time(dist2, speed * SMALL_BLOCK_PX);
				if (time25 > time2) {
					moveViewPoint(cache + move2, 0, time25 - time2);
				}
				PxPos move3 = moveBreak(moveAll, move2, kidMove);
				PxPos dist3 = move3;
				float time3 = PxPos::time(dist3, speed * SMALL_BLOCK_PX);
				moveViewPoint(cache + move2 + move3, time3, 0);

				PxPos move4 = (PxPos)moveAll - move2 - move3;
				float time4;
				if (kidMove == PxPos::zero)
					time4 = 0;
				else if ((int)std::abs(move4.x) > 0) {
					time4 = std::abs(move4.x) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.x);
				}
				else {
					time4 = std::abs(move4.y) / std::abs(speed * SMALL_BLOCK_PX / sqrSum * kidMove.y);
				}
				PxPos dist4 = kidMove - dist1 - dist2 - dist3;
				moveViewPoint(result, time4, 0);
				float time45 = PxPos::time(dist4, speed * SMALL_BLOCK_PX);
				if (time45 > time4)
					moveViewPoint(result, 0, time45 - time4);

			/*	cocos2d::log("move:");
				string str = std::to_string(move2.x) + " " + std::to_string(move2.y);
				cocos2d::log(str.c_str());
				str = std::to_string(move3.x) + " " + std::to_string(move3.y);
				cocos2d::log(str.c_str());
				str = std::to_string(move4.x) + " " + std::to_string(move4.y);
				cocos2d::log(str.c_str());
				cocos2d::log("dist:");
				str = std::to_string(dist1.x) + " " + std::to_string(dist1.y);
				cocos2d::log(str.c_str());
				str = std::to_string(dist2.x) + " " + std::to_string(dist2.y);
				cocos2d::log(str.c_str());
				str = std::to_string(dist3.x) + " " + std::to_string(dist3.y);
				cocos2d::log(str.c_str());
				cocos2d::log("time:");
				str = std::to_string(time1) + " " + std::to_string(time2) + " " + std::to_string(time3) + " " + std::to_string(time4) + " ";
				cocos2d::log(str.c_str());
*/
			}
		}
	}
}

void GameLiveScene::setViewPoint(const PxPos& point) {
	PAINT.objMove(this->rootCode(), -point, MoveType::linear, 0);
	this->viewPoint = point;
}

// TODO breakpause 打断之后的操作
void GameLiveScene::setFocus(const LiveObjPtr ptr, bool breakpause) {
	this->focusOn = ptr;
	focusMoveViewPoint(ptr, PxPos::zero, ptr->MP(), true, 1);
}

// rollback是一个对于dalyaSec为负数的确认提示
void GameLiveScene::moveViewPoint(const PxPos& point, float timeSec, float delaySec, bool rollback) {
	if (!rollback && delaySec < 0)
		delaySec = 0;
	float now = PAINT.clock();
	if (this->viewMovingUntil <= now) {
		this->viewMovingUntil = now;
	}
	PAINT.objMove(this->rootCode(), -point, MoveType::linear, timeSec, this->viewMovingUntil - now  + delaySec);
	this->viewMovingUntil += delaySec + timeSec;
	this->viewPoint = point;
}

void GameLiveScene::kidSet(ObjPtr child, const BlockPos& margin) {
	if(child == nullptr)
		return;
	if(kidPtr() == nullptr) {
		LiveObjPtr kind = GameLiveScene::make(child, margin);
		this->liveKid = kind;
		add(kind);
	}
}

void GameLiveScene::kidMove(const BlockPos& vec, MoveType type, float time, bool recursive) {
	if(kidPtr() == nullptr)
		return;
	else {
		BlockPos temp = this->liveKid->MP();
		movemove(this->kidPtr(), vec, type, time, recursive);
	}
}

void GameLiveScene::kidWalk(const BlockPos& vec) {
	if (vec != BlockPos::zero) {
		float time = BlockPos::time(vec, kidWalkSpeed);
		kidMove(vec, MoveType::linear, time, true);
	}
}

void GameLiveScene::kidRun(const BlockPos& vec) {
	if (vec != BlockPos::zero) {
		float time = BlockPos::time(vec, kidWalkSpeed * kidRunComparedToWalk);
		kidMove(vec, MoveType::linear, time, true);
	}
}

void GameLiveScene::kidRemove(bool recursive) {
	if(kidPtr() == nullptr)
		return;
	else{
		remove(kidPtr(), recursive);
		this->liveKid = nullptr;
	}
}

void GameLiveScene::kidReplace(ObjPtr newkid) {
	if(kidPtr() == nullptr)
		return;
	else {
		replace(this->kidPtr(), newkid);
	}
}

void GameLiveScene::kidAddObject(ObjPtr obj, const BlockPos& marginRelative) {
	if (kidPtr()) {
		BlockPos kidpos = kidPtr()->MP();
		LiveObjPtr live = add(obj, marginRelative + kidpos);
		addBind(kidPtr(), live);
	}
}

void GameLiveScene::switchFromSurroundingsToKid(LiveObjPtr obj, const BlockPos& margin, bool recursive) {
	remove(obj, recursive);
	obj->margin() = margin;
	add(obj);
	addBind(this->kidPtr(), obj);
}

void GameLiveScene::switchFromKidToSurroundings(LiveObjPtr obj, const BlockPos& margin, bool recursive) {
	remove(obj, recursive);
	obj->margin() = margin;
	add(obj);
}

void GameLiveScene::allDim(bool black) {
	// TODO
}

void GameLiveScene::allClear() {
	PAINT.nodeRemoveAllChildren(this->flatCode());
	PAINT.nodeRemoveAllChildren(this->cubeCode());
	PAINT.nodeRemoveAllChildren(this->cloudCode());
}

BlockPos GameLiveScene::getWindowRelativePosition(const BlockPos& pos) {
    return pos - this->viewPoint;
}

BlockPos GameLiveScene::getObjectRelativePosition(const BlockPos& pos, LiveObjPtr ptr) {
    return pos - ptr->MP();
}

Walkable GameLiveScene::detect(LiveObjPtr ptr, const LiveDot& ld, const BlockPos& current, LiveObjPtr& out_jumpObj) {
    bool noPass = false;
    LiveObjPtr jumpLayer = nullptr;
    LiveObjPtr slideLayer = nullptr;
    for (auto i = ld.begin(); i != ld.end(); i++) {
        // variable i cannot be null because it's an iterator, and it's not the end
        // so *i is ok
        if (*i == nullptr)
            continue;
        else {
            LiveObjPtr live = *i;
            WalkType wt = live->walktype();
            if (wt == WalkType::allWalk)
                continue;
            else if (wt == WalkType::noneWalk) {
                noPass = true;
                break;
            } else if (wt == WalkType::alphaWalk) {
                BlockPos rela = getObjectRelativePosition(current, *i);
                GameAlpha &alpha = live->walkBMP();
                Walkable able = alpha.getWalk(rela);
                if (able == Walkable::nullWalk)
                    continue;
                else if (able == Walkable::noWalk) {
                    noPass = true;
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
    if (noPass) {
        result = Walkable::noWalk;
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

Walkable GameLiveScene::detect(LiveObjPtr newptr, const BlockPos::Direction& dir, LiveObjPtr& out_jumpObj) {
    if (newptr == nullptr)
        return Walkable::nullWalk; // if it returns this, there seems to be a bug, so we need to print a warning.
    BlockPos start;
    BlockType dx, dy;

	BlockPos out_size, out_start;
	BlockPos::directionAreaSplit(newptr->MP(), newptr->size(), dir, detectSplit, out_start, out_size);
	dx = out_size.x;
	dy = out_size.y;

    bool twobreaks = false;
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
                auto dotres = detect(newptr, *ld, current, out_jumpObj);
                if (dotres == Walkable::nullWalk)
                    continue;
                else if (dotres == Walkable::noWalk) {
                    noPass = true;
                    twobreaks = true;
                    break;
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
        if (twobreaks)
            break;
    }

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

void GameLive::keySet() {
    auto el = cocos2d::EventListenerKeyboard::create();
    el->onKeyPressed = [](cocos2d::EventKeyboard::KeyCode kc, cocos2d::Event * event) {
        switch (kc) {
            case cocos2d::EventKeyboard::KeyCode::KEY_W:
                LIVE.press()[GameKeyPress::buttonUp]= true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_A:
                LIVE.press()[GameKeyPress::buttonLeft] = true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_S:
                LIVE.press()[GameKeyPress::buttonDown] = true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_D:
                LIVE.press()[GameKeyPress::buttonRight] = true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_J:
                LIVE.press()[GameKeyPress::buttonA] = true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_K:
                LIVE.press()[GameKeyPress::buttonB] = true;
				break;
			case cocos2d::EventKeyboard::KeyCode::KEY_L:
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
            case cocos2d::EventKeyboard::KeyCode::KEY_W:
                LIVE.press()[GameKeyPress::buttonUp] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_A:
                LIVE.press()[GameKeyPress::buttonLeft] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_S:
                LIVE.press()[GameKeyPress::buttonDown] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_D:
                LIVE.press()[GameKeyPress::buttonRight] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_J:
                LIVE.press()[GameKeyPress::buttonA] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_K:
                LIVE.press()[GameKeyPress::buttonB] = false;
				break;
			case cocos2d::EventKeyboard::KeyCode::KEY_L:
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

GameLiveScene::detectMoveReturn GameLiveScene::detectMoveOneObject(LiveObjPtr obj, const BlockPos& vec, MoveType move, float timeSec) {
	return GameLiveScene::detectMoveReturn::canMove;
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

void GameLive::api_UIStart(BaseCode uicode) {
    UIPtr uip = BASE.getUI(uicode);
    api_UIStart(uip);
}

void GameLive::api_UIStart(UIPtr uip) {
    if (uip == nullptr)
        return;
    LiveUIPtr glu(new GameLiveUI(uip));
    if (uip->type() == GameUI::up) {
        this->_UIUp.push_back(glu);
    } else if (uip->type() == GameUI::down) {
        this->_UIDown.push_back(glu);
    }
    glu->id() = glu->UI()->start(); // 所以就这样子直接调用了？不知道。
	PAINT.nodeDisplay(glu->id());
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
		(*lt)->UI()->stop();
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
		(*lt)->UI()->stop();
		PAINT.objRemove((*lt)->id(), PAINT.mainsc);
	}
	if (type == GameUI::empty)
		return;
	else if (type == GameUI::up)
		this->_UIUp.erase(lt);
	else if (type == GameUI::down)
		this->_UIDown.erase(lt);
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

void GameLive::api_sceneInit(BaseCode sceneCode, BlockPos mazeSize) {
	this->_scene = new GameLiveScene();
	this->_scene->init(mazeSize);
	this->_scene->setScene(sceneCode);
}

void GameLive::api_sceneDisplay() {
	PAINT.nodeDisplay(this->_scene->rootCode());
}

void GameLive::api_sceneCalculate() {
	// TODO npc position calculate, scene object calculate, and more
}

void GameLive::api_sceneICD(BaseCode sceneCode, const BlockPos& mazeSize, const BlockPos& windowSize) {
	api_sceneInit(sceneCode, mazeSize);
	api_sceneCalculate();
	api_sceneDisplay();
	api_setSceneSize(windowSize);
}

void GameLive::api_kidSet(BaseCode kidCode, const BlockPos& pos, bool focus) {
	ObjPtr pt = BASE.getStuff(kidCode);
	api_kidSet(pt, pos, focus);
}

void GameLive::api_kidSet(ObjPtr ptr, const BlockPos& pos, bool focus) {
	if (this->_scene != nullptr) {
		this->_scene->kidSet(ptr, pos);
		if (focus)
			this->_scene->setFocus(this->_scene->kidPtr());
	}
}

void GameLive::api_kidWalk(const BlockPos& vec) {
	if (this->_scene != nullptr) {
		this->_scene->kidWalk(vec);
	}
}

void GameLive::api_kidRun(const BlockPos& vec) {
	if (this->_scene != nullptr) {
		this->_scene->kidRun(vec);
	}
}

void GameLive::api_addObject(BaseCode code, const BlockPos& margin) {
	if (this->_scene != nullptr) {
		this->_scene->add(BASE.getStuff(code), margin);
	}
}

void GameLive::api_replaceObject(LiveObjPtr obj, ObjPtr ptr) {
	if (this->_scene != nullptr) {
		this->_scene->replace(obj, ptr);
	}
}

void GameLive::api_kidWalkStep(BlockPos::Direction dir) {
	if (this->_scene != nullptr) {
		api_kidWalk(this->_scene->getStepDist(dir));
	}
}

void GameLive::api_changePicture(LiveObjPtr obj, const string& newcsb) {
	if (this->_scene) {
		this->_scene->changePicture(obj, newcsb);
	}
}

void GameLive::api_delayTime(std::function<void(float)> func, float delaySec, const string& key) {
	cocos2d::Director::getInstance()->getScheduler()->schedule(func, &LIVE, 1, 1, delaySec, false, key);
}
void GameLive::api_undelay(const string &key) {
	cocos2d::Director::getInstance()->getScheduler()->unschedule(key, &LIVE);
}

void GameLive::api_autoAddActionLock(LiveObjPtr obj, float lockAdd) {
	 float now = PAINT.clock();
	 float loc = obj->getActionLock();
	 obj->setActionLock(loc > now ? loc : now + lockAdd);
 }

void GameLive::judge() {
step_one:
	if (this->_scene)
		this->_scene->commandCache = GameBase::DEFAULT_COMMAND;
	for (int i = _UIUp.size() - 1; i >= 0; i--) {
		if (_UIUp[i] == nullptr)
			continue;
		else {
			EventPtr eve = nullptr;
			LiveUIPtr ptt = _UIUp[i];
			_UIJudgeNow = ptt;
			JudgeReturn jud = ptt->UI()->action(ptt->id(), this->_keys);
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
		if (!(this->_scene->commandCache) && this->_scene->defaultTranslator)
			api_setCommandCache(this->_scene->defaultTranslator->translate(this->keys()));
		this->_objectJudgeNow = nullptr;

		vector<LiveObjPtr> objects;
		this->_scene->kidRangeObjects(objects);
		for (auto ind = 0; ind < (int)objects.size(); ind++) {
			this->_objectJudgeNow = objects[ind];
			ObjPtr obj = this->_objectJudgeNow->getObj();
			if (obj == nullptr)
				continue;
			else {
				GameCommand cmd;
				if (obj->isCustomTranslate())
					cmd = obj->translate(this->keys());
				else
					cmd = api_getCommandCache();
				EventPtr eve;
				JudgeReturn jud= obj->link(cmd, eve);
				if (eve != nullptr)
					api_eventStart(eve, nullptr);
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
					if (ind > 1)
						ind -= 2;
				}
				else if (jud == judgeObjectLayer) {
					goto step_two;
				}
				else if (jud == judgeResetLayer) {
					goto step_two;
				}
				else if (jud == judgeResetAll) {
					goto step_one;
				}
				else {
				}
			}
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
			JudgeReturn jud = ptt->UI()->action(ptt->id(), this->_keys);
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

bool GameLive::keyPushed(float* keyarray, GameKeyPress gkp) {
	if (keyarray[gkp] > 0)
		return true;
	else
		return false;
}
bool GameLive::keyPushed(float* keyarray, vector<GameKeyPress> vgkp) {
	for (unsigned int i = 0; i < vgkp.size(); i++)
		if (keyarray[vgkp[i]] == 0 || keyarray[vgkp[i]] < 0)
			return false;
	return true;
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
bool GameLive::keyJustPushedOnly(float* keyarray, GameKeyPress gkp) {
	auto keys = keyarray;
	//pushed
	if (keys[gkp] < _loopfreq - _loopdevation || keys[gkp] > _loopfreq + _loopdevation) //考虑可以忽略的延迟时间?
		return false;
	//only
	for (auto i = GameKeyPress::buttonEmpty + 1; i < GameKeyPress::buttonEnd; ++i)
		if (i != gkp && (keys[i] >= _loopfreq - _loopdevation && keys[i] <= _loopfreq + _loopdevation))
			return false;
	return true;
}
bool GameLive::keyJustPushedOnly(float* keyarray, vector<GameKeyPress> vgkp) {
	auto keys = keyarray;
	//pushed
	for (unsigned int i = 0; i < vgkp.size(); i++)
		if (keys[vgkp[i]] < _loopfreq - _loopdevation || keys[vgkp[i]] > _loopfreq + _loopdevation) //考虑可以忽略的延迟时间?
			return false;
	//only
	int keyPressedNum = 0;
	for (auto i = GameKeyPress::buttonEmpty + 1; i < GameKeyPress::buttonEnd; ++i)
		if (keys[i] >= _loopfreq - _loopdevation && keys[i] <= _loopfreq + _loopdevation)
			++keyPressedNum;
	return ((int)vgkp.size()) == keyPressedNum;
}
bool GameLive::_keyCyclePushed(float presstime, float cycleSec) {
	float time = std::fmod(presstime, cycleSec);
	if (time > _loopfreq - _loopdevation && time < _loopfreq + _loopdevation)
		return true;
	else
		return false;
}
bool GameLive::keyCyclePushedOnly(float* keys, GameKeyPress gkp, float cycleSec) {
	//pushed
	if (!_keyCyclePushed(keys[gkp], cycleSec))
		return false;
	//only
	for (auto i = GameKeyPress::buttonEmpty + 1; i < GameKeyPress::buttonEnd; ++i)
		if (i != gkp && _keyCyclePushed(keys[i], cycleSec))
			return false;
	return true;
}
bool GameLive::keyCyclePushedOnly(float* keys, vector<GameKeyPress> vgkp, float cycleSec) {
	//pushed
	for (auto gkp : vgkp)
		if (!_keyCyclePushed(keys[gkp], cycleSec))
			return false;
	//auto
	int keypressedcnt = 0;
	for (auto i = GameKeyPress::buttonEmpty + 1; i < GameKeyPress::buttonEnd; ++i)
		if (_keyCyclePushed(keys[i], cycleSec))
			keypressedcnt++;
	return ((int)vgkp.size()) == keypressedcnt;
}

