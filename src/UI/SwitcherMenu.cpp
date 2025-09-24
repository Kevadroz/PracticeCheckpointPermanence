#include "SwitcherMenu.hpp"

#if defined(GEODE_IS_DESKTOP)
#define SWITCHER_SCALE .75f
#else
#define SWITCHER_SCALE 1.5f
#endif

SwitcherMenu* SwitcherMenu::create(ModPlayLayer* playLayer) {
	CCDirector* director = CCDirector::sharedDirector();
	CCSize winSize = director->getWinSize();

	SwitcherMenu* menu = new SwitcherMenu();
	menu->setContentSize(CCSizeMake(80, 80));
	menu->setScale(SWITCHER_SCALE);
	menu->setPosition(getSwitcherPosition(director));
	menu->setZOrder(15);
	menu->setID("switcher_menu"_spr);

	CCSprite* nextSpr =
		CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	CCSprite* previousSpr =
		CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	nextSpr->setScale(.6f);
	previousSpr->setScale(.6f);
	nextSpr->setFlipX(true);

	AxisLayoutOptions* nextLayoutOptions = AxisLayoutOptions::create();
	nextLayoutOptions->setPrevGap(5.f);
	AxisLayoutOptions* previousLayoutOptions = AxisLayoutOptions::create();
	previousLayoutOptions->setNextGap(5.f);

	CCNode* nextBtn;
	CCNode* previousBtn;

	if (playLayer != nullptr) {
		nextBtn = CCMenuItemExt::createSpriteExtra(
			nextSpr, [playLayer](CCObject* sender) { playLayer->nextCheckpoint(); }
		);

		previousBtn = CCMenuItemExt::createSpriteExtra(
			previousSpr,
			[playLayer](CCObject* sender) { playLayer->previousCheckpoint(); }
		);
	} else {
		nextBtn = nextSpr;
		previousBtn = previousSpr;
	}
	nextBtn->setID("next");
	nextBtn->setLayoutOptions(nextLayoutOptions);

	previousBtn->setID("previous");
	previousBtn->setLayoutOptions(previousLayoutOptions);

	const char* checkpointLabelString;
	const char* layerLabelString;
	if (playLayer != nullptr) {
		menu->m_checkpointSprite =
			CCSprite::createWithSpriteFrameName("inactiveCheckpoint.png"_spr);
		menu->m_checkpointSprite->setOpacity(192);

		checkpointLabelString = "0/0";
		layerLabelString = "Layer 1/0";
	} else {
		menu->m_checkpointSprite =
			CCSprite::createWithSpriteFrameName("activeCheckpoint.png"_spr);

		checkpointLabelString = "2/5";
		layerLabelString = "Layer 2/3";
	}
	menu->m_checkpointSprite->setID("checkpoint");

	menu->m_checkpointLabel =
		CCLabelBMFont::create(checkpointLabelString, "bigFont.fnt");
	menu->m_checkpointLabel->setID("checkpoint-label");
	menu->m_checkpointLabel->setScale(.6f);

	menu->m_layerLabel = CCLabelBMFont::create(layerLabelString, "bigFont.fnt");
	menu->m_layerLabel->setID("layer-label");
	menu->m_layerLabel->setScale(.6f);

	menu->addChildAtPosition(menu->m_checkpointSprite, geode::Anchor::Center);
	menu->addChildAtPosition(previousBtn, geode::Anchor::Center, ccp(-25, 0));
	menu->addChildAtPosition(nextBtn, geode::Anchor::Center, ccp(25, 0));
	menu->addChildAtPosition(
		menu->m_checkpointLabel, geode::Anchor::Center, ccp(0, -28)
	);
	menu->addChildAtPosition(
		menu->m_layerLabel, geode::Anchor::Center, ccp(0, 31)
	);

	menu->autorelease();
	return menu;
}

SwitcherMenu* SwitcherMenu::createWithTouch(
	ModPlayLayer* playLayer,
	std::function<bool(CCTouch*, CCEvent*)> touchBeganCallback,
	std::function<void(CCTouch*, CCEvent*)> touchMovedCallback,
	std::function<void(CCTouch*, CCEvent*)> touchEndedCallback,
	std::function<void(CCTouch*, CCEvent*)> touchCancelledCallback
) {
	SwitcherMenu* menu = create(playLayer);
	menu->setTouchEnabled(true);

	menu->m_touchBeganCallback = touchBeganCallback;
	menu->m_touchMovedCallback = touchMovedCallback;
	menu->m_touchEndedCallback = touchEndedCallback;
	menu->m_touchCancelledCallback = touchCancelledCallback;

	return menu;
}

bool SwitcherMenu::ccTouchBegan(CCTouch* touch, CCEvent* event) {
	if (m_touchBeganCallback != nullptr)
		return m_touchBeganCallback(touch, event);
	else
		return false;
}

void SwitcherMenu::ccTouchMoved(CCTouch* touch, CCEvent* event) {
	if (m_touchMovedCallback != nullptr)
		m_touchMovedCallback(touch, event);
}

void SwitcherMenu::ccTouchEnded(CCTouch* touch, CCEvent* event) {
	if (m_touchEndedCallback != nullptr)
		m_touchEndedCallback(touch, event);
}

void SwitcherMenu::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
	if (m_touchCancelledCallback != nullptr)
		m_touchCancelledCallback(touch, event);
}

CCPoint getSwitcherPosition(CCDirector* director) {
	Mod* mod = Mod::get();
	return ccp(
		mod->getSavedValue<double>(
			"switcherMenuPositionX", director->getScreenLeft() + 45.f
		),
		mod->getSavedValue<double>(
			"switcherMenuPositionY", director->getScreenTop() - 50.f
		)
	);
}
