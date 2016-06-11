#include "GamePrincipal.h"
#include "GameBase.h"
#include "GameLive.h"
#include "GamePaint.h"
#include "GameUI.h"
#include "GameNPC.h"
#include <fstream>
#include <iostream>

const PxPos			PxPos::zero = PxPos(0, 0);
const BlockPos		BlockPos::zero = BlockPos(0, 0);
const GameObject	GameObject::origin;

const float			GameBase::KID_MOVE_SPEED_IN_BIG_BLOCKS = 6.0f;
const BlockType		GameBase::KID_STEP = 2;
const float			GameBase::KID_RUN_COMPARED_TO_WALK = 2.0f;
const float			GameBase::USE_TOOL_TIME = 0.3f;
const float			GameBase::SWITCH_TOOL_TIME = 0.2f;
const float			GameBase::PICK_STUFF_TIME = 0.4f;
const float			GameBase::DROP_STUFF_TIME = 0.4f;
const int			GameBase::MINUTES_IN_HOUR = 60;
const int			GameBase::HOUR_IN_DAY = 24;
const int			GameBase::DAYS_IN_SEASON = 30;
const BlockPos		GameBase::SCENE_DISPLAY_SIZE = BlockPos(PxPos(960, 640));
const string		GameBase::EMPTY_STRING = "";

const int			GameBase::DETECT_SPLIT = 2;
const GameCommand	GameBase::DEFAULT_COMMAND = GameCommand::emptyCmd;


const string talk1_name = "Lei Mu";
const string talk1_csb = "";
const string talk1_text = "地方刚回家";
const string talk2_name = "Lei Mu";
const string talk2_csb = "";
const string talk2_text = "Zai Diu Ni Yi Ge Lei Mu";

const string choose1_des = "FUCK THE FUCKING PROJECT!!!!!!!!";
const string choose1_choice[] = {"1", "2", "3", "4"};
const int choose1_count = 4;

void GameBase::init() {
	cocos2d::FileUtils::getInstance()->addSearchPath("res/Tools");
	cocos2d::FileUtils::getInstance()->addSearchPath("res/Humans");
	cocos2d::FileUtils::getInstance()->addSearchPath("res/Plants");
	cocos2d::FileUtils::getInstance()->addSearchPath("res/Stuff");
	cocos2d::FileUtils::getInstance()->addSearchPath("res/Ground");
	cocos2d::FileUtils::getInstance()->addSearchPath("res/Background");
	cocos2d::FileUtils::getInstance()->addSearchPath("res/UI");
	cocos2d::FileUtils::getInstance()->addSearchPath("fonts");

    // Translator Create
    GameTranslator::create<BasicMenuTranslator>(basicMenuTranslator, &transData);
	GameTranslator::create<BasicMoveTranslator>(basicMoveTranslator, &transData);
	GameTranslator::create<BasicObjectTranslator>(basicObjectTranslator, &transData);
	GameTranslator::create<ToolTranslator>(toolTranslator, &transData);
	GameTranslator::create<TalkTranslator>(talkTranslator, &transData);
	GameTranslator::create<HandTranslator>(handTranslator, &transData);

    // Linker Create
	GameLinker::create<SoilLinker>(soilLinkerCode, &linkerData);
	GameLinker::create<BedLinker>(bedLinkerCode, &linkerData);
	GameLinker::create<DefaultLinker>(defaultLinkerCode, &linkerData);
	GameLinker::create<PotatoLinker>(potatoLinkerCode, &linkerData);
	GameLinker::create<SeedSellerLinker>(seedSellerLinkerCode, &linkerData);

    // Object Create
    GameObject::create(GameObject::ground, farmPicCode, "farmBackground", "", BlockPos(PxPos(960, 640)), WalkType::allWalk, &stuffData, "FarmBackground.csb")
		->setPickable(false)->setDropable(false);

	ObjPtr soilComb = GameObject::create(GameObject::combStatue, soilCombCode, "soilComb", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData);
	soilComb->setPickable(false)->setDropable(false);
	GameObject::create(GameObject::ground, soilOriginCode, "soilOrigin", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "SoilOrigin.csb", soilComb);
	GameObject::create(GameObject::ground, soilHoedCode, "soilHoed", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "SoilHoed.csb", soilComb);
	GameObject::create(GameObject::ground, soilWateredCode, "soilWatered", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "SoilWatered.csb", soilComb);
	soilComb->setChildrenLinker(BASE.getLinker(soilLinkerCode));

	// bed
	ObjPtr bedComb = GameObject::create(GameObject::combStuff, bedCode, "bed", "", BigBlockPos(2, 4), WalkType::allWalk, &stuffData);
	bedComb->setPickable(false)->setDropable(false);
	GameObject::create(GameObject::ground, bedDownCode, "bedDown", "", BigBlockPos(2, 4), WalkType::noneWalk, &stuffData, "BedDown.csb", bedComb)
		->setCenter(BlockPos(BIG_TO_SMALL / 2, BIG_TO_SMALL * 2 - 2));
	bedComb->childrenPos().push_back(BlockPos::zero);
	GameObject::create(GameObject::furniture, bedUpCode, "bedUp", "", BigBlockPos(2, 4), WalkType::noneWalk, &stuffData, "BedUp.csb", bedComb)
		->setCenter(BlockPos(BIG_TO_SMALL / 2, BIG_TO_SMALL * 2 - 2));
	bedComb->childrenPos().push_back(BlockPos::zero);
	bedComb->setChildrenLinker(BASE.getLinker(bedLinkerCode));
	
	// tools
	GameObject::create(GameObject::BigType::stuff, toolHoe, "Tool Hoe", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "Hoe.csb")
		->setPickable(false)->setDropable(false);
	GameObject::create(GameObject::BigType::stuff, toolWaterCan, "Tool WaterCan", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "WaterCan.csb")
		->setPickable(false)->setDropable(false);
	GameObject::create(GameObject::BigType::stuff, toolPotatoSeed, "Tool PotatoSeed", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "PotatoSeed.csb")
		->setPickable(false)->setDropable(false)->setQuality(1);

	// plant stuff
	ObjPtr potatoComb = GameObject::create(GameObject::combStatue, stuffPotatoStart, "", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData);
	potatoComb->setPickable(false)->setDropable(false);
	GameObject::create(GameObject::BigType::seed, stuffPotatoWithered, "Withered Potato", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "WitherPotato.csb", potatoComb);
	GameObject::create(GameObject::BigType::seed, stuffPotatoSeed, "Potato Seed", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "Seed.csb", potatoComb);
	GameObject::create(GameObject::BigType::plant, stuffPotatoLittle, "Little Potato", "", BigBlockPos(1, 1), WalkType::noneWalk, &stuffData, "LittlePlant.csb", potatoComb);
	GameObject::create(GameObject::BigType::plant, stuffPotatoBig, "Big Potato", "", BigBlockPos(1, 1), WalkType::noneWalk, &stuffData, "BigPotato.csb", potatoComb);
	GameObject::create(GameObject::BigType::plant, stuffPotatoHarvest, "Harvest Potato", "", BigBlockPos(1, 1), WalkType::noneWalk, &stuffData, "HarvestPotato.csb", potatoComb)
		->setLinker(BASE.getLinker(potatoLinkerCode));

	GameObject::create(GameObject::BigType::seed, houseCode, "House", "", BlockPos(PxPos(320, 290)), WalkType::noneWalk, &stuffData, "House.csb")
		->setPickable(false)->setDropable(false)->setAlphaTXT("houseCanWalk.png.txt");
	GameObject::create(GameObject::BigType::stuff, potatoCode, "Potato", "", BigBlockPos(1, 1), WalkType::allWalk, &stuffData, "Potato.csb");



	// human stuff
	string kidfacing[] = { "KidFace.csb",
		"KidFace.csb",
		"KidFace.csb",
		"KidRight.csb",
		"KidLeft.csb",
		"KidFace.csb",
		"KidRight.csb",
		"KidLeft.csb",
		"KidBack.csb",
		"KidBack.csb"
	};
	GameObject::create(GameObject::BigType::kid, kidNormalStuffCode, "kid", "", BigBlockPos(1, 1), WalkType::noneWalk, &stuffData, "KidFace.csb")
		->setPickable(false)->setDropable(false)->setCenter(BlockPos(2, 2))->setMixedSize((BlockPos)BigBlockPos(1, 2) + BlockPos(0, 0))->setFacingPicture(kidfacing);

	GameObject::create(GameObject::BigType::npc, seedSellerStuffCode, "seed seller", "", BigBlockPos(1, 1), WalkType::noneWalk, &stuffData, "NPC1.csb")
		->setPickable(false)->setDropable(false)->setMixedSize((BlockPos)BigBlockPos(1, 2) - BlockPos(0, 1))->setLinker(getLinker(seedSellerLinkerCode));

    // Scene Create
	ObjPtr farmsc = GameObject::create(GameObject::BigType::background, farmSceneCode, "farmScene", "", BlockPos(415, 415), WalkType::allWalk, &sceneData, "FarmScene.csb", nullptr, BlockPos::zero, BlockPos::zero);
	for (int i = 1; i < 11; i++)
		for (int j = 1; j < 11; j++) {
			farmsc->children().push_back(getStuff(soilOriginCode));
			farmsc->childrenPos().push_back(BlockPos(146, 234) + BigBlockPos(i, j));
		}
	farmsc->setAlphaTXT("BigBackgroundWalk.txt");
	farmsc->children().push_back(getStuff(houseCode));
	farmsc->childrenPos().push_back(BlockPos(PxPos(2220, 2442)));

	ObjPtr housesc = GameObject::create(GameObject::BigType::background, houseSceneCode, "House Inside Scene", "", BlockPos(90, 60), WalkType::allWalk, &sceneData, "HouseInside.csb", nullptr, BlockPos::zero, BlockPos::zero);
	housesc->setAlphaTXT("houseInsideCanWalk.png.txt");
	housesc->children().push_back(getStuff(bedCode));
	housesc->childrenPos().push_back(BigBlockPos(20, 9));
	housesc->children().push_back(getStuff(seedSellerStuffCode));
	housesc->childrenPos().push_back(BigBlockPos(10, 4));

	// Jump Create
	getStuff(houseCode)->setJumpTXT("houseCanWalk.png.txt", houseSceneCode, BlockPos(PxPos(450, 20)));
	getScene(houseSceneCode)->setJumpTXT("houseInsideCanWalk.png.txt", farmSceneCode, BlockPos(PxPos(2290, 2342)));

	// Plant Create
	GamePlant::create(PlantCode::plantPotato, "", potatoComb, vector<int>{0, 0, 1, 1, 1}, vector<SeasonType> {haru}, &plantData);


	// Human Create
	GameHuman::create(kidHumanCode, "kidSelf", 100, &humanData);
	GameHuman::create(seedSellerHumanCode, "Seed Seller", 100, &humanData);

    // UI Create
    GameUI::create<StartPageUI>(startPageCode, "startPage", basicMenuTranslator,  &UIData);
	GameUI::create<KidMoveUI>(kidMoveUICode, "kidMoveUI", basicMoveTranslator, &UIData);
	GameUI::create<ToolUI>(toolUICode, "toolUI", toolTranslator, &UIData);
	GameUI::create<TalkUI>(talkUICode, "Talk UI", talkTranslator, &UIData);
	GameUI::create<HandUI>(handUICode, "HandUI", handTranslator, &UIData);
	GameUI::create<ChooseUI>(chooseUICode, "Choose UI", basicMenuTranslator, &UIData);

	// Event Create
	GameEvent::create<StartGameEvent>(startGameEventCode, &eventData);
	GameEvent::create<DayPassEvent>(dayPassEventCode, &eventData);

	// Story Create
	GameStory* story1 = GameStory::create(seedSellerStoryCode, &storyData);
	GameTalk* talk1 = new GameTalk(talk1_name, talk1_csb, talk1_text);
	const GameStory** effect1 = new const GameStory*[] { story1, nullptr, nullptr, nullptr};
	GameChoose* choose1 = new GameChoose(talk1_name, choose1_des, choose1_choice, choose1_count, effect1);
	GameTalk* talk2 = new GameTalk(talk2_name, talk2_csb, talk2_text);
	story1->add(talk1);
	story1->add(choose1);
	story1->add(talk2);

	// internal
	setDefaultLinker();
}

BlockPos BlockPos::dirToBlock(Direction dir) {
    if (dir == Direction::one)
        return BlockPos(-1, -1);
    else if (dir == Direction::two)
        return BlockPos(0, -1);
    else if (dir == Direction::three)
        return BlockPos(1, -1);
    else if (dir == Direction::four)
        return BlockPos(-1, 0);
    else if (dir == Direction::five)
        return BlockPos::zero;
    else if (dir == Direction::six)
        return BlockPos(1, 0);
    else if (dir == Direction::seven)
        return BlockPos(-1, 1);
    else if (dir == Direction::eight)
        return BlockPos(0, 1);
    else if (dir == Direction::nine)
        return BlockPos(1, 1);
    else
        return BlockPos::zero;
}

BlockPos::Direction BlockPos::cmdToDir(GameCommand cmd) {
	switch (cmd)
	{
	case walkOne:
	case runOne:
		return one;
		break;
	case walkTwo:
	case runTwo:
	case selectDown:
		return two;
		break;
	case walkThree:
	case runThree:
		return three;
		break;
	case walkFour:
	case runFour:
	case selectLeft:
		return four;
		break;
	case walkSix:
	case runSix:
	case selectRight:
		return six;
		break;
	case walkSeven:
	case runSeven:
		return seven;
		break;
	case walkEight:
	case runEight:
	case selectUp:
		return eight;
		break;
	case walkNine:
	case runNine:
		return nine;
		break;
	default:
		return empty;
		break;
	}
}

void BlockPos::move(Direction dir) {
    *this += dirToBlock(dir);
}

void BlockPos::moveBack(Direction dir) {
    *this -= dirToBlock(dir);
}

BlockPos::Direction BlockPos::degreeToDir(float degree) {
	while (degree < 0)
		degree += 360;
	while (degree > 360)
		degree -= 360;
	if (degree > 337.5 || degree <= 22.5)
		return Direction::six;
	if (degree > 22.5 && degree <= 67.5)
		return Direction::nine;
	if (degree > 67.5 && degree <= 112.5)
		return Direction::eight;
	if (degree > 112.5 && degree <= 157.5)
		return Direction::seven;
	if (degree > 157.5 && degree <= 202.5)
		return Direction::four;
	if (degree > 202.5 && degree <= 247.5)
		return Direction::one;
	if (degree > 247.5 && degree <= 292.5)
		return Direction::two;
	if (degree > 292.5 && degree <= 337.5)
		return Direction::three;
	else
		return Direction::six;
}

BlockPos::operator BlockPos::Direction() const {
	if (*this == BlockPos::zero)
		return Direction::five;
	float angle = std::atan2(this->y, this->x) * 180 / std::_Pi;
	return degreeToDir(angle);
}

void BlockPos::directionAreaSplit(const BlockPos& start, const BlockPos& size, const Direction dir, const int split, BlockPos& out_start, BlockPos& out_size) {
	BlockPos area = size;
	BlockPos startRelative = BlockPos::zero;
	if (dir == BlockPos::Direction::empty) {
	}
	else if (dir == BlockPos::Direction::one) {
		area = BlockPos(area.x / split, area.y / split);
	}
	else if (dir == BlockPos::Direction::two) {
		area = BlockPos(area.x, area.y / split);
	}
	else if (dir == BlockPos::Direction::three) {
		area = BlockPos(area.x / split, area.y / split);
		startRelative = BlockPos(size.x - area.x, 0);
	}
	else if (dir == BlockPos::Direction::four) {
		area = BlockPos(area.x / split, area.y);
	}
	else if (dir == BlockPos::Direction::five) {
	}
	else if (dir == BlockPos::Direction::six) {
		area = BlockPos(area.x / split, area.y);
		startRelative = BlockPos(size.x - area.x, 0);
	}
	else if (dir == BlockPos::Direction::seven) {
		area = BlockPos(area.x / split, area.y / split);
		startRelative = BlockPos(0, size.y - area.y);
	}
	else if (dir == BlockPos::Direction::eight) {
		area = BlockPos(area.x, area.y / split);
		startRelative = BlockPos(0, size.y - area.y);
	}
	else if (dir == BlockPos::Direction::nine) {
		area = BlockPos(area.x / split, area.y / split);
		startRelative = BlockPos(size.x - area.x, size.y - area.y);
	}
	else {
	}
	out_start = start + startRelative;
	out_size = area;
}

//bool GameAlpha::init(const BlockPos& _size, const string& BMPfile) {
//	Byte* colors;
//	BlockPos colorSize;
//	if (readBMP(BMPfile, colors, colorSize)) {
//		this->size = _size;
//		return anaBMP(this->moveMap, _size, colors, colorSize);
//	}
//	return false;
//}

inline void blockconvert(const int& bigx, const int& bigy, const float& stepx, const float& stepy, int& outx, int& outy)
{
	outx = (float)bigx / stepx;
	outy = (float)bigy / stepy;
}

bool GameAlpha::initTXT(const BlockPos& _size, const string& TXTfile) {
	std::ifstream file(TXTfile, std::ios::binary);
	if (!file) {
		cocos2d::log("[ERROR] Cannot open the TXT file %s.", TXTfile.c_str());
		return false;
	}
	this->size = _size;
	char tempchar;
	int capa = _size.x * _size.y;
	this->moveMap = new Walkable[capa];
	for (int iy = _size.y - 1; iy > -1; iy--) {
		for (int ix = 0; ix < _size.x; ix++) {
			file.read(&tempchar, sizeof(char));
			auto blank = get2(this->moveMap, _size.x, _size.y, ix, iy);
			if (blank == nullptr)
				continue;
			while (tempchar == '\n' || tempchar == '\r')
				file.read(&tempchar, sizeof(char));
			switch (tempchar)
			{
			case '1':
				*blank = Walkable::jump;
				break;
			case '0':
				*blank = Walkable::noWalk;
				break;
			case '7':
				*blank = Walkable::canWalk;
				break;
			default:
				break;
			}
			std::cout << tempchar;
		}
		std::cout << std::endl;
	}
	return true;
}

bool GameAlpha::readBMP(const string& BMPfile, Byte *&colors, BlockPos &out_colorSize) {
	Byte *buffer1, *buffer2, *buffer3;
	std::ifstream file(BMPfile, std::ios::binary);
	if (!file) {
		cocos2d::log("[ERROR] Cannot open the BMP file %s.", BMPfile.c_str());
		return false;
	}
	buffer1 = new Byte[sizeof(BitmapFileHeader)];
	buffer2 = new Byte[sizeof(BitmapInfoHeader)];
	file.read((char*)buffer1, sizeof(BitmapFileHeader));
	file.read((char*)buffer2, sizeof(BitmapInfoHeader));
	BitmapFileHeader *header = (BitmapFileHeader*)buffer1;
	BitmapInfoHeader *info = (BitmapInfoHeader*)buffer2;
	if (header->bfType != 0x4D42) {
		cocos2d::log("[ERROR] That's not a BMP file.");
		return false;
	}
	buffer3 = new Byte[info->biSizeImage];
	file.seekg(header->bfOffBits);
	file.read((char*)buffer3, info->biSizeImage);
	colors = buffer3;
	out_colorSize = BlockPos(info->biWidth, info->biHeight);
	delete buffer1;
	delete buffer2;
	return true;
}

inline int colorminus(int ra, int ga, int ba, int rb, int gb, int bb) {
	return std::abs(ra - rb) + std::abs(ga - gb) + std::abs(ba - bb);
}

bool GameAlpha::anaBMP(Walkable* moveMap, const BlockPos& size, Byte* colors, const BlockPos& colorSize) {
	if (colors == nullptr)
		return false;
	moveMap = new Walkable[size.x * size.y]{Walkable::nullWalk};
	int *red = new int[size.x * size.y]{0};
	int *green = new int[size.x * size.y]{0};
	int *blue = new int[size.x * size.y]{0};
	int *cnt = new int[size.x * size.y]{0};
	float stepx = (float)colorSize.x / (float)size.x;
	float stepy = (float)colorSize.y / (float)size.y;

	for (int ix = 0; ix < colorSize.x; ix++) {
		for (int iy = 0; iy < colorSize.y; iy++) {
			Byte *colordot = get3(colors, colorSize.x, colorSize.y, ix, iy, 3);
			if (colordot == nullptr) {
				delete colors;
				return false;
			}
			int smallx, smally;
			blockconvert(ix, iy, stepx, stepy, smallx, smally);
			int *rd = get2(red, size.x, size.y, smallx, smally),
				*gd = get2(green, size.x, size.y, smallx, smally),
				*bd = get2(blue, size.x, size.y, smallx, smally),
				*cd = get2(cnt, size.x, size.y, smallx, smally);
			if (rd && gd && bd && cd){
				(*cd)++;
				(*rd) += colordot[2];
				(*bd) += colordot[1];
				(*gd) += colordot[0];
			}
		}
	}

	for (int smallx = 0; smallx < size.x; smallx++) {
		for (int smally = 0; smally < size.y; smally++) {
			Walkable* dot = get2(moveMap, size.x, size.y, smallx, smally);
			int *rd = get2(red, size.x, size.y, smallx, smally),
				*gd = get2(green, size.x, size.y, smallx, smally),
				*bd = get2(blue, size.x, size.y, smallx, smally),
				*cd = get2(cnt, size.x, size.y, smallx, smally);
			if (dot && rd && gd && bd && cd) {
				int redAve = (*rd) / (*cd);
				int greenAve = (*gd) / (*cd);
				int blueAve = (*bd) / (*cd);
				int distK = colorminus(redAve, greenAve, blueAve, 0, 0, 0),
					distW = colorminus(redAve, greenAve, blueAve, 255, 255, 255),
					distR = colorminus(redAve, greenAve, blueAve, 255, 0, 0),
					distG = colorminus(redAve, greenAve, blueAve, 0, 255, 0);
				int dist[] = { distK, distW, distR, distG };
				std::sort(dist, dist + 4);
				if (dist[0] == distK)
					*dot = Walkable::noWalk;
				else if (dist[0] == distW)
					*dot = Walkable::canWalk;
				else if (dist[0] == distR)
					*dot = Walkable::jump;
				else if (dist[0] == distG)
					*dot = Walkable::slide;
				else
					*dot = Walkable::nullWalk;
			}
		}
	}
	delete red;
	delete green;
	delete blue;
	delete cnt;
	delete colors;
	return true;
}

GameCommand GameObject::translate(float* arrOfKeys) {
	if (isCustomTranslate()) {
		return this->_translator->translate(arrOfKeys);
	}
	else
		return GameCommand::emptyCmd;
}

JudgeReturn GameObject::link(GameCommand gcmd, EventPtr& out_event, GameObjectJudge& judge, float& timeSec, float& delaySec, LockType& locktype) {
    if (this->linker() != nullptr) {
        LinkerReturn sec = this->linker()->link(gcmd, judge);
        out_event = sec.eve;
		timeSec = sec.lockTime;
		delaySec = sec.delayTime;
		locktype = sec.lock;
        return sec.judge;
    } else {
        out_event = nullptr;
        return this->_defaultJudge;
    }
}

#if defined(LINUX) || defined(__APPLE__) || defined(APPLE)
LiveCode GameObject::customPaint(__attribute__((unused)) LiveCode father, __attribute__((unused)) const BlockPos& pos) {
#endif
#ifdef WIN32
LiveCode GameObject::customPaint(LiveCode father, const BlockPos& pos, int dotOrder) {
	UNREFERENCED_PARAMETER(father);
	UNREFERENCED_PARAMETER(pos);
#endif
    return nullptr;
}

#if defined(LINUX) || defined(__APPLE__) || defined(APPLE)
void GameObject::afterPaint(__attribute__((unused)) LiveCode obj) {
#endif
#ifdef WIN32
void GameObject::afterPaint(LiveCode obj) {
	UNREFERENCED_PARAMETER(obj);
#endif
}

bool GameObject::onFaceChange(LiveObjPtr obj, BlockPos::Direction oldface, BlockPos::Direction newface) {
	if (this->_noFacingDifference)
		return true;
	else {
		if (obj == nullptr)
			return true;
		LIVE.api_objectChangePicture(obj, this->getFacingPicture()[(int)newface]);
		return true;
	}
}

bool GameObject::getJump(const BlockPos& relative, const BlockPos& size, BaseCode& out_sceneCode, BlockPos& out_kidpos, bool getSlide) {
	Walkable target = Walkable::jump;
	if (getSlide)
		target = Walkable::slide;
	for (auto &layer : this->_jumpInfo) {
		if (layer == nullptr)
			continue;
		for (int ix = 0; ix < size.x; ix++)
			for (int iy = 0; iy < size.y; iy++) {
				Walkable walk = layer->jump.getWalk(relative + BlockPos(ix, iy));
				if (walk == target) {
					out_sceneCode = layer->sceneCode;
					out_kidpos = layer->kidPos;
					return true;
				}
			}
	}
	return false;
}

const string& GamePlant::getCSB(int stage) {
	auto temp = statueObj.lock();
	if (temp == nullptr)
		return BASE.EMPTY_STRING;
	if (stage >= 0 && stage < (int)(temp->children().size())) {
		auto tt = (temp->children()[stage]).lock();
		if (tt == nullptr)
			return BASE.EMPTY_STRING;
		else
			return tt->picture();
	}
	else
		return BASE.EMPTY_STRING;
}


ObjPtr GameBase::getStuff(BaseCode code) {
    if (code >= 0 && code < (int) stuffData.size())
        return stuffData[code];
    else
        return nullptr;
}

const string& GameBase::getStuffCSB(BaseCode code) {
	ObjPtr ptr = getStuff(code);
	if (ptr)
		return ptr->picture();
	else
		return EMPTY_STRING;
}

ObjPtr GameBase::getScene(BaseCode code) {
    if (code >= 0 && code < (int) sceneData.size())
        return sceneData[code];
    else
        return nullptr;
}

TransPtr GameBase::getTranslator(BaseCode code) {
    if (code >= 0 && code < (int) transData.size())
        return transData[code];
    else
        return nullptr;
}

UIPtr GameBase::getUI(BaseCode code) {
    if (code >= 0 && code < (int) UIData.size())
        return UIData[code];
    else
        return nullptr;
}

EventPtr GameBase::getEvent(BaseCode code) {
	if (code >= 0 && code < (int)eventData.size())
		return eventData[code];
	else
		return nullptr;
}
 
LinkerPtr GameBase::getLinker(BaseCode code) {
	if (code >= 0 && code < (int)linkerData.size())
		return linkerData[code];
	else
		return nullptr;
}

PlantPtr GameBase::getPlant(BaseCode code) {
	if (code >= 0 && code < (int)plantData.size())
		return plantData[code];
	else
		return nullptr;
}

HumanPtr GameBase::getHuman(BaseCode code) {
	if (code >= 0 && code < (int)humanData.size())
		return humanData[code];
	else
		return nullptr;
}

GameStory* GameBase::getStory(BaseCode code) {
	if (code >= 0 && code < (int)storyData.size())
		return storyData[code];
	else
		return nullptr;
}

void GameBase::setDefaultLinker() {
	auto linker = BASE.getLinker(defaultLinkerCode);
	for (auto &obj : stuffData) {
		if (obj != nullptr && obj->linker() == nullptr) {
			obj->linker() = linker;
		}
	}
}

bool GameEvent::start(LiveObjPtr obj) {
    return true;
}

TransPtr GameTranslator::create(BaseCode code, std::function<GameCommand(float*)> &method, vector<TransPtr> *container) {
    TransPtr pt(new GameTranslator(method));
    T_push(container, pt, code);
    return pt;
}

LinkerPtr GameLinker::create(BaseCode code, std::function<LinkerReturn(GameCommand)> &method, vector<LinkerPtr> *container) {
    LinkerPtr pt(new GameLinker(method));
    T_push(container, pt, code);
    return pt;
}

LinkerReturn GameLinker::link(GameCommand gcmd, GameObjectJudge& judge) {
    return convert(gcmd);
}

GameCommand GameTranslator::translate(float* arrOfKeys) {
	if (arrOfKeys != nullptr)
		return convert(arrOfKeys);
	else
		return emptyCmd;
}

int GameBase::cmdWalkOrRun(GameCommand cmd) {
	if (cmd > walkStart && cmd < walkEnd)
		return WALK;
	else if (cmd > runStart && cmd < runEnd)
		return RUN;
	else
		return OTHERCMD;
}
//
//GameCommand GameBase::toolToCmd(BaseCode tool) {
//	if (tool > toolStart && tool < toolEnd) {
//		switch (tool) {
//		case toolHoe:
//			return useHoe;
//		case toolWaterCan:
//			return useWaterCan;
//		case toolPotatoSeed:
//			return usePotatoSeed;
//			//TODO 未完待续
//		default:
//			return emptyCmd;
//		}
//	}
//	else
//		return emptyCmd;
//}

PlantCode GameBase::stuffToPlant(BaseCode plantStuff) {
	if (plantStuff > plantStuffStart && plantStuff < plantStuffEnd) {
		if (plantStuff >= stuffPotatoStart && plantStuff <= stuffPotatoHarvest)
			return plantPotato;
		else
			return plantStart;
	}
	else
		return plantStart;
}

void GameStoryElement::stop(GameStoryLoader& loader) {
	if (loader.getActiveNode() != nullptr) {
		LIVE.api_UIStop(loader.getActiveNode());
	}
}

LiveCode GameTalk::action(GameStoryLoader& loader) {
	std::shared_ptr<TalkUI> uip = std::dynamic_pointer_cast<TalkUI>(BASE.getUI(talkUICode));
	uip->parentLoader = &loader;
	uip->talkContent = this;
	return LIVE.api_UIStart(uip);
}

LiveCode GameStoryEvent::action(GameStoryLoader& loader) {
	LIVE.api_eventStart(eventCode, nullptr); //这就是一个坑
	return nullptr;
}

LiveCode GameChoose::action(GameStoryLoader& loader) {
	std::shared_ptr<ChooseUI> uip = std::dynamic_pointer_cast<ChooseUI>(BASE.getUI(chooseUICode));
	uip->content = this;
	uip->parentLoader = &loader;
	return LIVE.api_UIStart(uip);
}

