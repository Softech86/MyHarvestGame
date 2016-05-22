#include "GameLive.h"
#include "GameBase.h"
#include "GamePrincipal.h"
#include "cocos2d.h"
#include "GamePaint.h"
#include <cmath>

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

// 计算绘画的屏幕相对位置
BlockPos GameLiveObject::paintPos() {
    return this->MP() + PxPos(1, 0) * this->zValue();
}

LiveCode GameLiveObject::paint(LiveCode father) {
    BlockPos pos = this->paintPos();
    LiveCode result;
	if (this->getObj()->isCustomPaint()) {
		result = this->getObj()->customPaint(father, pos);
    }
    else {
        result = PAINT.objAddToObj(father, this->picture(), pos, scale(), alpha());
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

LiveCode GameLiveObject::repaint(LiveCode father) {
	erase(father);
	return paint(father);
}

GameLiveUI::GameLiveUI(UIPtr ori) {
	if (ori == nullptr) {
		this->_ui = UIPtr(new GameUI(GameUI::origin));
	}
    else
        this->_ui = UIPtr(ori->SHCP());
}

void GameLiveScene::blockAdd(const LiveObjPtr ptr) {
    if (ptr == nullptr)
        return;
    BlockPos drawpos = ptr->MP();
    BlockPos scenesize = this->scene->size();
    for (int i = 0; i < ptr->size().x; i++) {
        for (int j = 0; j < ptr->size().y; j++) {
            auto ld = get(this->blockMap, scenesize, drawpos + BlockPos(i, j));
            if (ld == nullptr)
                continue;
            else
                ld->push_back(ptr);
        }
    }
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
    BlockPos scenesize = this->scene->size();
    for (int i = 0; i < ptr->size().x; i++) {
        for (int j = 0; j < ptr->size().y; j++) {
            LiveDot *ld = get(this->blockMap, scenesize, drawpos + BlockPos(i, j));
            if (ld == nullptr)
                continue;
            else
                dotRemoveLayer(*ld, ptr);
        }
    }
}

// 这里直接就修改live对象了
void GameLiveScene::blockMove(LiveObjPtr oldptr, const BlockPos& vec) {
    if (oldptr == nullptr)
        return;
    // 现在这东西很暴力，之后一定需要优化
    // 但是如果你想保留物品在移动之后的物品位置始终是最上面的话你是在这里不可以优化的啊
    // 嗯就是这样。
    blockRemove(oldptr);
    oldptr->margin() += vec;
    blockAdd(oldptr);
}

// 这里直接就修改live对象了
void GameLiveScene::blockReplace(LiveObjPtr oldptr, ObjPtr newobj) {
    if (newobj == nullptr)
        return;
    else {
        blockRemove(oldptr);
		oldptr->setObj(newobj);
        blockAdd(oldptr);
    }
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

void GameLiveScene::mapAdd(LiveObjPtr ptr, bool recursive) {
    if (ptr == nullptr)
        return;
    else {
        blockAdd(ptr);
        if (recursive) {
            for (LiveObjWeak &child : ptr->outBind()) {
                // 这里不用校验null了，调用的时候会处理的
                mapAdd(child.lock(), true);
            }
        }
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
void GameLiveScene::mapMove(LiveObjPtr ptr, const BlockPos& vec, bool recursive) {
    if (ptr == nullptr)
        return;
    else {
        blockMove(ptr, vec);
        if (recursive) {
            for (auto &child : ptr->outBind()) {
                mapMove(child.lock(), vec, true);
            }
        }
    }
}

// 这里直接就修改live对象了
void GameLiveScene::mapReplace(LiveObjPtr oldptr, ObjPtr newptr) {
    blockReplace(oldptr, newptr);
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

LiveObjPtr GameLiveScene::make(BaseCode ptr, bool scene, GameLiveObject::StickTo stick, const BlockPos& margin, int z, float scale, float alpha) {
    ObjPtr temp;
    if(scene)
        temp = BASE.getScene(ptr);
    else
        temp = BASE.getStuff(ptr);
    return GameLiveScene::make(temp, stick, margin, z, scale, alpha);
}

LiveObjPtr GameLiveScene::make(ObjPtr obj, GameLiveObject::StickTo stick, const BlockPos& margin, int z, float scale, float alpha) {
	BlockPos mppd = BlockPos::zero;
    if(this->scene != nullptr)
		mppd = this->scene->padding();
	return make_(obj, stick, margin, mppd, BlockPos::zero, z, scale, alpha);
}

LiveObjPtr GameLiveScene::make_(ObjPtr obj, GameLiveObject::StickTo stick, const BlockPos& margin, const BlockPos& mappadding, BlockPos parentAdd, int z, float scale, float alpha) {
	LiveObjPtr pt(new GameLiveObject(obj, margin + mappadding + parentAdd));
    pt->zValue() = z;
    pt->scale() = scale;
    pt->alpha() = alpha;
    pt->setStick(stick);

	this->cacheAdd(pt);

    if(obj->type() == GameObject::BigType::combStuff) {
		parentAdd += pt->MP();
        for(int index = 0; index < (int)obj->children().size(); index++) {
            LiveObjPtr livechild = make_(obj->children()[index].lock(), stick, obj->childrenPos()[index], mappadding, parentAdd, z, scale, alpha);
            addBind(pt, livechild);
        }
    }
    return pt;
}

LiveCode GameLiveScene::getParent(LiveObjPtr obj) {
    LiveCode parent = nullptr;
	if (obj->getStick() == GameLiveObject::StickTo::surroundings)
		parent = this->codeSurrounding;
	else if (obj->getStick() == GameLiveObject::StickTo::kid)
		parent = this->codeKid;
	return parent;
}

void GameLiveScene::init(const BlockPos& mazeSize) {
    this->codeRoot = PAINT.nodeNew();
    this->codeSurrounding = PAINT.objAddToObj(this->codeRoot, "", PxPos::zero);
    this->codeKid = PAINT.objAddToObj(this->codeRoot, "", PxPos::zero);
    
    this->mazeSize = mazeSize;
    this->blockMap = new LiveDot[mazeSize.x * mazeSize.y];
}

void GameLiveScene::setScene(BaseCode scene) {
	this->scene = BASE.getScene(scene);
	// scene.margin() always be zero
	this->add(this->scene, GameLiveObject::StickTo::surroundings, BlockPos::zero);
	// viewPoint initialize to the left-under corner of the area you can see in this scene
	this->viewPoint = this->scene->padding();
}

LiveObjPtr GameLiveScene::add(ObjPtr obj, GameLiveObject::StickTo stick, const BlockPos& margin) {
    LiveObjPtr live = make(obj, stick, margin);
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
    mapAdd(live, false);
    LiveCode parent = this->getParent(live);
	if(parent == nullptr)
		return;

	LiveCode code = live->paint(parent);
	if(code == nullptr)
		return;
	
    dictAdd(code, live);
	
	this->cacheRemove(live);

    for(auto childptr : live->outBind()) {
        add(childptr.lock());
    }

	if (this->getFocus() == live) {
		focusMoveViewPoint(live, PxPos::zero, live->MP(), true);
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
    if(recursive) {
        mapMove(ptr, vec, false);
        for(auto childptr : ptr->outBind())
            movemove(childptr.lock(), vec, move, timeSec, true);
    } else {
        mapMove(ptr, vec, false);
	}

	if (this->getFocus() == ptr) {
		focusMoveViewPoint(ptr, oldpos, ptr->MP(), false);
	}
}

void GameLiveScene::replace(LiveObjPtr oldptr, ObjPtr newptr) {
    mapReplace(oldptr, newptr);
	oldptr->repaint(this->getParent(oldptr));
    // I guess there should be no problem
}

GameLiveScene::LineReturn GameLiveScene::DistanceToCentralLine(const PxPos& windowRelative, const PxPos& direction, PxPos& outResult) {
	if (direction == PxPos::zero)
		return NEVER;
	PxPos window = this->windowSize;
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
	PxPos window = this->windowSize;
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
	PxPos winSze = this->windowSize;
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

void GameLiveScene::focusMoveViewPoint(LiveObjPtr obj, const PxPos& oldpos, const PxPos& newpos, bool flash) {
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
	float time1 = PxPos::time(dist1, kidMoveSpeed * SMALL_BLOCK_PX);
	float sqrSum = PxPos::distance(PxPos::zero, kidMove);
	float timeAll = PxPos::time(kidMove, kidMoveSpeed * SMALL_BLOCK_PX);
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
				time3 = std::abs(moveAll.x) / std::abs(kidMoveSpeed * SMALL_BLOCK_PX / sqrSum * kidMove.x);
			}
			else {
				time3 = std::abs(moveAll.y) / std::abs(kidMoveSpeed * SMALL_BLOCK_PX / sqrSum * kidMove.y);
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
				time2 = std::abs(move2.x) / std::abs(kidMoveSpeed * SMALL_BLOCK_PX / sqrSum * kidMove.x);
			}
			else {
				time2 = std::abs(move2.y) / std::abs(kidMoveSpeed * SMALL_BLOCK_PX / sqrSum * kidMove.y);
			}

			moveViewPoint(cache + move2, time2, time1);
			if (flag2 == NEVER) {
				if (timeAll - time2 - time1 > 0)
					moveViewPoint(result, 0, timeAll - time2 - time1);
				return;
			}
			else {
				float time25 = PxPos::time(dist2, kidMoveSpeed * SMALL_BLOCK_PX);
				if (time25 > time2) {
					moveViewPoint(cache + move2, 0, time25 - time2);
				}
				PxPos move3 = moveBreak(moveAll, move2, kidMove);
				PxPos dist3 = move3;
				float time3 = PxPos::time(dist3, kidMoveSpeed * SMALL_BLOCK_PX);
				moveViewPoint(cache + move2 + move3, time3, 0);

				PxPos move4 = (PxPos)moveAll - move2 - move3;
				float time4;
				if (kidMove == PxPos::zero)
					time4 = 0;
				else if ((int)std::abs(move4.x) > 0) {
					time4 = std::abs(move4.x) / std::abs(kidMoveSpeed * SMALL_BLOCK_PX / sqrSum * kidMove.x);
				}
				else {
					time4 = std::abs(move4.y) / std::abs(kidMoveSpeed * SMALL_BLOCK_PX / sqrSum * kidMove.y);
				}
				PxPos dist4 = kidMove - dist1 - dist2 - dist3;
				moveViewPoint(result, time4, 0);
				float time45 = PxPos::time(dist4, kidMoveSpeed * SMALL_BLOCK_PX);
				if (time45 > time4)
					moveViewPoint(result, 0, time45 - time4);

				cocos2d::log("move:");
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
	focusMoveViewPoint(ptr, PxPos::zero, ptr->MP(), true);
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
		LiveObjPtr kind = GameLiveScene::make(child, GameLiveObject::StickTo::kid, margin);
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
		float time = BlockPos::time(vec, kidMoveSpeed);
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

void GameLiveScene::switchFromSurroundingsToKid(LiveObjPtr obj, const BlockPos& margin, bool recursive) {
	remove(obj, recursive);
	obj->setStick(GameLiveObject::StickTo::kid);
	obj->margin() = margin;
	add(obj);
}

void GameLiveScene::switchFromKidToSurroundings(LiveObjPtr obj, const BlockPos& margin, bool recursive) {
	remove(obj, recursive);
	obj->setStick(GameLiveObject::StickTo::surroundings);
	obj->margin() = margin;
	add(obj);
}

void GameLiveScene::allDim(bool black) {
	// TODO
}

void GameLiveScene::allClear() {
	PAINT.nodeRemoveAllChildren(this->surroundingCode());
	PAINT.nodeRemoveAllChildren(this->kidCode());
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
    if (dir == BlockPos::Direction::empty) {
        return Walkable::nullWalk;
    } else if (dir == BlockPos::Direction::one) {
        dx = newptr->size().x / detectSplit;
        dy = newptr->size().y / detectSplit;
        start = BlockPos::zero;
    } else if (dir == BlockPos::Direction::two) {
        dx = newptr->size().x;
        dy = newptr->size().y / detectSplit;
        start = BlockPos::zero;
    } else if (dir == BlockPos::Direction::three) {
        dx = newptr->size().x / detectSplit;
        dy = newptr->size().y / detectSplit;
        start = newptr->size() - BlockPos(dx, 0);
    } else if (dir == BlockPos::Direction::four) {
        dx = newptr->size().x / detectSplit;
        dy = newptr->size().y;
        start = BlockPos::zero;
    } else if (dir == BlockPos::Direction::five) {
        dx = newptr->size().x;
        dy = newptr->size().y;
        start = BlockPos::zero;
    } else if (dir == BlockPos::Direction::six) {
        dx = newptr->size().x / detectSplit;
        dy = newptr->size().y;
        start = newptr->size() - BlockPos(dx, 0);
    } else if (dir == BlockPos::Direction::seven) {
        dx = newptr->size().x / detectSplit;
        dy = newptr->size().y / detectSplit;
        start = BlockPos(0, newptr->size().y - dy);
    } else if (dir == BlockPos::Direction::eight) {
        dx = newptr->size().x;
        dy = newptr->size().y / detectSplit;
        start = BlockPos(0, newptr->size().y - dy);
    } else if (dir == BlockPos::Direction::nine) {
        dx = newptr->size().x;
        dy = newptr->size().y / detectSplit;
        start = newptr->size() - BlockPos(dx, dy);
    } else {
        return Walkable::nullWalk;
    }


    BlockPos drawpos = newptr->MP() + start;
    BlockPos scenesize = this->scene->size();

    bool twobreaks = false;
    bool noPass = false;
    LiveObjPtr jumpLayer = nullptr;
    LiveObjPtr slideLayer = nullptr;
    for (BlockType i = 0; i < dx; i++) {
        for (BlockType j = 0; j < dy; j++) {
            auto current = drawpos + BlockPos(i, j);
            LiveDot *ld = get(this->blockMap, scenesize, current);
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


void GameLive::keyAddTime() {
	for (int i = 0; i < KEY_COUNT; i++) {
		if (_press[i]) {
			_keys[i] += _loopfreq;
		}
		else {
			_keys[i] = 0;
		}
	}
}

const string GameLive::KEY_LOOP_NAME = "keyboardLoop";

void GameLive::keyLoop() {
    auto judgeSch = [](float dt) {
		if (LIVE._close)
            cocos2d::Director::getInstance()->getScheduler()->unschedule(KEY_LOOP_NAME, &LIVE);
		else {
			LIVE.keyAddTime();
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

bool GameLive::api_setWindowSize(const BlockPos& size) {
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

void GameLive::api_UIStop(LiveCode id) {
	GameUI::UIType type = GameUI::UIType::empty;
	auto lt = _UIPtrQuery(id, type);
	if (type == GameUI::empty)
		return;
	else if (type == GameUI::up)
		this->_UIUp.erase(lt);
	else if(type == GameUI::down)
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
	api_setWindowSize(windowSize);
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

void GameLive::api_kidWalkStep(BlockPos::Direction dir) {
	if (this->_scene != nullptr) {
		api_kidWalk(this->_scene->getStepDist(dir));
	}
}

void GameLive::judge() {
step_one:
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
            } else if (jud == judgeNextObject) {
                continue;
            } else if (jud == judgeNextLayer) {
                break;
            } else if (jud == judgePreviousObject) {
                i += 2;
                if (i >= (int) _UIUp.size())
                    i -= 2; // 超出的话视作NextObject处理
            } else if (jud == judgeResetLayer || jud == judgeResetAll) {
                goto step_one;
            } else {
            }
        }
	}
	_UIJudgeNow = nullptr;

    //TODO

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
            } else if (jud == judgeResetLayer) {
                goto step_three;
            } else if (jud == judgeResetAll) {
                goto step_one;
            } else {
            }
        }
	}
	_UIJudgeNow = nullptr;
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
	for (auto gkp = vgkp.begin(); gkp < vgkp.end(); ++gkp)
		if (keys[*gkp] <= _loopdevation)
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
	for (auto gkp = vgkp.begin(); gkp < vgkp.end(); ++gkp)
		if (keys[*gkp] < _loopfreq - _loopdevation || keys[*gkp] > _loopfreq + _loopdevation) //考虑可以忽略的延迟时间?
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