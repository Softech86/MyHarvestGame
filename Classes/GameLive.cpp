#include "GameLive.h"
#include "GameBase.h"
#include "GamePrincipal.h"
#include "cocos2d.h"
#include "cocostudio\CocoStudio.h"
#include "GamePaint.h"

GameLiveObject::GameLiveObject(ObjPtr pobj) {
    if (pobj == nullptr) {
        _obj = GameObject::origin;
    } else {
        _obj = *pobj;
    }
}

GameLiveObject::GameLiveObject(ObjPtr pobj, const BlockPos& margin__)
: GameLiveObject::GameLiveObject(pobj) {
    this->margin() = margin__;
}

BlockPos GameLiveObject::paintPos() {
    return this->margin() + this->padding() + PxPos(1, 0) * this->zValue();
}

LiveCode GameLiveObject::paint(LiveCode father) {
    BlockPos pos = this->paintPos();
    LiveCode result;
    if(this->obj().isCustomPaint()) {
        result = this->obj().customPaint(father, pos);
    }
    else {
        result = GamePrincipal::getPaint().objAddToObj(father, this->picture(), pos);
    }
    this->obj().afterPaint(result);
	this->paintCode() = result;
    return result;
}

void GameLiveObject::erase() {
    // TODO
}

LiveCode GameLiveObject::repaint() {
    // TODO
    return nullptr;
}

GameLiveUI::GameLiveUI(UIPtr ori) {
    if (ori == nullptr)
        this->_ui = GameUI::origin;
    else
        this->_ui = *ori;
}

void GameLiveScene::blockAdd(const LiveObjPtr ptr) {
    if (ptr == nullptr)
        return;
    BlockPos drawpos = ptr->margin() + ptr->padding();
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
    BlockPos drawpos = ptr->margin() + ptr->padding();
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
        oldptr->obj() = *newobj;
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
        temp = GamePrincipal::getBase().getScene(ptr);
    else
        temp = GamePrincipal::getBase().getStuff(ptr);
    return GameLiveScene::make(temp, stick, margin, z, scale, alpha);
}

LiveObjPtr GameLiveScene::make(ObjPtr obj, GameLiveObject::StickTo stick, const BlockPos& margin, int z, float scale, float alpha) {
    LiveObjPtr pt(new GameLiveObject(obj, margin));
    pt->zValue() = z;
    pt->scale() = scale;
    pt->alpha() = alpha;
    pt->setStick(stick);

	this->cacheAdd(pt);

    // don't forget to create bind
    if(obj->type() == GameObject::BigType::combStuff) {
        for(int index = 0; index < (int)obj->children().size(); index++) {
            LiveObjPtr livechild = make(obj->children()[index].lock(), stick, obj->childrenPos()[index]);
            addBind(pt, livechild);
        }
    }
    return pt;
}

void GameLiveScene::init(const BlockPos& mazeSize) {
    auto pai = GamePrincipal::getPaint();
    
    this->root = pai.nodeNew();
    this->surrounding = pai.objAddToObj(this->root, "", PxPos::zero);
    this->kid = pai.objAddToObj(this->root, "", PxPos::zero);
    
    this->mazeSize = mazeSize;
    this->blockMap = new LiveDot[mazeSize.x * mazeSize.y];
}

void GameLiveScene::setScene(BaseCode scene) {
	this->scene = GamePrincipal::getBase().getScene(scene);
	this->add(this->scene, GameLiveObject::StickTo::surroundings, BlockPos::zero);
}

void GameLiveScene::add(ObjPtr obj, GameLiveObject::StickTo stick, const BlockPos& margin) {
    LiveObjPtr live = make(obj, stick, margin);
    add(live);
}

void GameLiveScene::cacheAdd(LiveObjPtr live) {
	this->toDebug.push_back(live);
}

void GameLiveScene::cacheRemove(LiveObjPtr live) {
	for (auto lt = this->toDebug.begin(); lt != this->toDebug.end(); lt++) {
		if (*lt == live) {
			this->toDebug.erase(lt);
			return;
		}
	}
}

void GameLiveScene::add(LiveObjPtr live) {
    mapAdd(live, false);
    LiveCode parent;

	if (live->getStick() == GameLiveObject::StickTo::surroundings)
		parent = this->surrounding;
	else if (live->getStick() == GameLiveObject::StickTo::kid)
		parent = this->kid;
	else
		return;

	LiveCode code = live->paint(parent);
    dictAdd(code, live);
	
	this->cacheRemove(live);

    for(auto childptr : live->outBind()) {
        add(childptr.lock());
    }
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
    GamePrincipal::getPaint().objRemove(live->paintCode());
    dictRemove(live->paintCode());
}

void GameLiveScene::movemove(LiveObjPtr ptr, const BlockPos& vec, MoveType move, float timeSec, bool recursive) {
    BlockPos pos = ptr->paintPos();
    GamePrincipal::getPaint().objMove(ptr->paintCode(), pos, pos + vec, move, timeSec);
    if(recursive) {
        mapMove(ptr, vec, false);
        for(auto childptr : ptr->outBind())
            movemove(childptr.lock(), vec, move, timeSec, true);
    } else {
        mapMove(ptr, vec, false);
    }
}

void GameLiveScene::replace(LiveObjPtr oldptr, ObjPtr newptr) {
    mapReplace(oldptr, newptr);
    // I guess there should be no problem
}

BlockPos GameLiveScene::getWindowRelativePosition(const BlockPos& pos) {
    return pos - this->viewPoint;
}

BlockPos GameLiveScene::getObjectRelativePosition(const BlockPos& pos, LiveObjPtr ptr) {
    return pos - ptr->margin();
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


    BlockPos drawpos = newptr->margin() + newptr->padding() + start;
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
                GamePrincipal::getLive().keys()[GameKeyPress::buttonUp] = true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_A:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonLeft] = true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_S:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonDown] = true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_D:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonRight] = true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_Z:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonA] = true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_X:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonB] = true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_ENTER:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonStart] = true;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonSpace] = true;
                break;
            default:
                break;
        }
    };
    el->onKeyReleased = [](cocos2d::EventKeyboard::KeyCode kc, cocos2d::Event * event) {
        switch (kc) {
            case cocos2d::EventKeyboard::KeyCode::KEY_W:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonUp] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_A:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonLeft] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_S:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonDown] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_D:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonRight] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_Z:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonA] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_X:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonB] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_ENTER:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonStart] = false;
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
                GamePrincipal::getLive().keys()[GameKeyPress::buttonSpace] = false;
                break;
            default:
                break;
        }
    };
    cocos2d::Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(el, 1);
}

const string GameLive::KEY_LOOP_NAME = "keyboardLoop";

void GameLive::keyLoop() {
    auto judgeSch = [](GameLive* ptr, float dt) {
        if (ptr->_close)
            cocos2d::Director::getInstance()->getScheduler()->unschedule(KEY_LOOP_NAME, ptr);
        else
            ptr->judge();
    };
    GameLive* tmp = this;
    // TODO
    //cocos2d::Director::getInstance()->getScheduler()->schedule(std::bind(&judgeSch, tmp), tmp, _loopfreq, false, KEY_LOOP_NAME);
}

void GameLive::enter() {
    this->api_sceneInit(farmSceneCode, BlockPos(100, 100));
	this->api_sceneDisplay();
	GamePrincipal::getPaint().objMove(this->_scene->surroundingCode(), PxPos(0, 0), PxPos(100, 100), MoveType::linear, 5.0f);
	GamePrincipal::getPaint().objRotate(this->_scene->surroundingCode(), 0, 150, 7.0f);
	//GamePrincipal::getPaint().objAlpha(this->_scene->surroundingCode(), 0, 0, 0.0f);
	//GamePrincipal::getPaint().objRemove(this->_scene->surroundingCode());

}

void GameLive::init() {
    if (_keys == nullptr)
        _keys = new bool[KEY_COUNT];
    if (_keys == nullptr)
        return;
    this->keySet();
    this->enter();
    this->keyLoop();
}

void GameLive::api_UIStart(BaseCode uicode) {
    UIPtr uip = GamePrincipal::getBase().getUI(uicode);
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
    glu->id() = glu->UI().start(); // 所以就这样子直接调用了？不知道。
}

void GameLive::api_eventStart(EventPtr eve, LiveObjPtr obj) {
    if (eve == nullptr)
        return;
    else {
        eve->start(obj); // 所以就这样子直接调用了？不知道。
    }
}

void GameLive::api_sceneInit(BaseCode sceneCode, BlockPos mazeSize) {
	this->_scene = new GameLiveScene();
	this->_scene->init(mazeSize);
	this->_scene->setScene(sceneCode);
}

void GameLive::api_sceneDisplay() {
	GamePrincipal::getPaint().nodeDisplay(this->_scene->rootCode());
}

void GameLive::judge() {
step_one:
    for (int i = _UIUp.size() - 1; i >= 0; i--) {
        if (_UIUp[i] == nullptr)
            continue;
        else {
            EventPtr eve = nullptr;
            LiveUIPtr ptt = _UIUp[i];
            JudgeReturn jud = ptt->UI().action(this->_keys);
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

    //TODO

step_three:
    for (int i = _UIDown.size() - 1; i >= 0; i--) {
        if (_UIDown[i] == nullptr)
            continue;
        else {
            EventPtr eve = nullptr;
            LiveUIPtr ptt = _UIUp[i];
            JudgeReturn jud = ptt->UI().action(this->_keys);
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

}
