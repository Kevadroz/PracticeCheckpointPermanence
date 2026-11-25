#include "SwitcherMenu.hpp"

SwitcherMenu* SwitcherMenu::create(ModPlayLayer* playLayer) {
	CCSize winSize = CCDirector::sharedDirector()->getWinSize();

	CCSize size = CCSizeMake(110, 110);

	SwitcherMenu* menu = new SwitcherMenu();
	menu->setContentSize(size);
	menu->setCascadeColorEnabled(true);
	menu->setScale(SwitcherMenu::getSwitcherScale());
	menu->setZOrder(15);
	menu->setID("switcher_menu"_spr);

	menu->m_buttonMenu = CCMenu::create();
	menu->m_labelMenu = CCMenu::create();
	menu->m_buttonMenu->setContentSize(size);
	menu->m_labelMenu->setContentSize(size);
	menu->m_buttonMenu->setID("buttons");
	menu->m_labelMenu->setID("labels");
	menu->m_labelMenu->setTouchEnabled(false);
	menu->m_buttonMenu->setTouchEnabled(false);

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

	if (playLayer != nullptr) {
		menu->m_nextBtn = CCMenuItemExt::createSpriteExtra(
			nextSpr, [playLayer](CCObject* sender) { playLayer->nextCheckpoint(); }
		);

		menu->m_previousBtn = CCMenuItemExt::createSpriteExtra(
			previousSpr,
			[playLayer](CCObject* sender) { playLayer->previousCheckpoint(); }
		);
	} else {
		menu->m_nextBtn = nextSpr;
		menu->m_previousBtn = previousSpr;
	}
	menu->m_nextBtn->setID("next");
	menu->m_nextBtn->setLayoutOptions(nextLayoutOptions);

	menu->m_previousBtn->setID("previous");
	menu->m_previousBtn->setLayoutOptions(previousLayoutOptions);

	const char* nameLabelString;
	const char* checkpointLabelString;
	const char* layerLabelString;
	const char* errorLabelString;
	if (playLayer != nullptr) {
		menu->m_checkpointSpriteBase =
			CCSprite::createWithSpriteFrameName("inactiveCheckpoint_base.png"_spr);
		menu->m_checkpointSpriteBase->setOpacity(192);

		menu->m_checkpointSpriteColor = CCSprite::createWithSpriteFrameName(
			"inactiveCheckpoint_default.png"_spr
		);

		nameLabelString = "0%";
		checkpointLabelString = "0/0";
		layerLabelString = "Lay 1/0";
	} else {
		menu->m_checkpointSpriteBase =
			CCSprite::createWithSpriteFrameName("activeCheckpoint.png"_spr);
		menu->m_checkpointSpriteColor = CCSprite::createWithSpriteFrameName(
			"activeCheckpoint_default.png"_spr
		);

		nameLabelString = "First Ship Hard Part";
		checkpointLabelString = "2 (4)/5";
		layerLabelString = "Lay 12/26";
	}
	menu->m_checkpointSpriteBase->setID("checkpoint");
	menu->m_checkpointSpriteColor->setID("checkpoint-color");
	menu->m_checkpointSpriteBase->setCascadeOpacityEnabled(true);
	menu->m_checkpointSpriteColor->ignoreAnchorPointForPosition(true);

	menu->m_nameLabel =
		CCLabelBMFont::create(nameLabelString, "bigFont.fnt", 125.0f);
	menu->m_nameLabel->setID("name-label");
	menu->m_nameLabel->setLineBreakWithoutSpace(true);
	menu->m_nameLabel->setScale(.6f);
	menu->m_nameLabel->setAnchorPoint(ccp(0.5f, 1.0f));
	menu->m_nameLabel->setAlignment(kCCTextAlignmentCenter);

	menu->m_checkpointLabel =
		CCLabelBMFont::create(checkpointLabelString, "bigFont.fnt");
	menu->m_checkpointLabel->setID("checkpoint-label");
	menu->m_checkpointLabel->setScale(.6f);

	menu->m_layerLabel = CCLabelBMFont::create(layerLabelString, "bigFont.fnt");
	menu->m_layerLabel->setID("layer-label");
	menu->m_layerLabel->setScale(.6f);

	menu->m_errorLabel = CCLabelBMFont::create("NO ERROR", "bigFont.fnt");
	menu->m_errorLabel->setID("error-label");
	menu->m_errorLabel->setScale(.6f);

	menu->m_checkpointSpriteBase->addChild(menu->m_checkpointSpriteColor);

	menu->addChildAtPosition(menu->m_buttonMenu, Anchor::Center);
	menu->addChildAtPosition(menu->m_labelMenu, Anchor::Center);
	menu->addChildAtPosition(
		menu->m_checkpointSpriteBase, Anchor::Center
	);
	menu->m_buttonMenu->addChildAtPosition(
		menu->m_previousBtn, Anchor::Center, ccp(-25, 0)
	);
	menu->m_buttonMenu->addChildAtPosition(
		menu->m_nextBtn, Anchor::Center, ccp(25, 0)
	);
	menu->m_labelMenu->addChildAtPosition(
		menu->m_nameLabel, Anchor::Center
	);
	menu->m_labelMenu->addChildAtPosition(
		menu->m_checkpointLabel, Anchor::Center
	);
	menu->m_labelMenu->addChildAtPosition(
		menu->m_layerLabel, Anchor::Center
	);
	menu->m_labelMenu->addChildAtPosition(
		menu->m_errorLabel, Anchor::Center
	);

	menu->updateLabels();

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

CCPoint SwitcherMenu::getSwitcherPosition() {
	CCDirector* director = CCDirector::sharedDirector();
	Mod* mod = Mod::get();
	return ccp(
		mod->getSavedValue<double>(
			"switcherMenuPositionX", director->getScreenLeft() + SWITCHER_OFFSET_X
		),
		mod->getSavedValue<double>(
			"switcherMenuPositionY", director->getScreenTop() - SWITCHER_OFFSET_Y
		)
	);
}

float SwitcherMenu::getSwitcherScale() {
	return Mod::get()->getSavedValue<double>(
		"switcherMenuScale", SWITCHER_SCALE
	);
}

void SwitcherMenu::updateLabels() {
	bool error = Mod::get()->getSettingValue<bool>("switcher-label-error");
	bool layers = Mod::get()->getSettingValue<bool>("switcher-label-layers");
	bool checkpoints =
		Mod::get()->getSettingValue<bool>("switcher-label-checkpoints");
	bool name = Mod::get()->getSettingValue<bool>("switcher-label-name");

	m_errorLabel->setVisible(error);
	m_layerLabel->setVisible(layers);
	m_checkpointLabel->setVisible(checkpoints);
	m_nameLabel->setVisible(name);

	float errorPos = 48;
	float layerPos = 31;
	float checkPos = -28;
	float namePos = -35;

	if (
		Mod::get()->getSettingValue<float>("switcher-icon-active-opacity") +
			Mod::get()->getSettingValue<float>("switcher-icon-inactive-opacity") <=
		0
	) {
		float offset = 10;
		if (
			Mod::get()->getSettingValue<float>("switcher-button-active-opacity") +
				Mod::get()->getSettingValue<float>(
					"switcher-button-inactive-opacity"
				) <=
			0
		)
			offset = 21;

		errorPos -= offset;
		layerPos -= offset;
		checkPos += offset;
		namePos += offset;
	}

	if (!layers)
		errorPos -= 17;
	if (!checkpoints)
		namePos += 17;

	m_errorLabel->updateAnchoredPosition(Anchor::Center, ccp(0, errorPos));
	m_layerLabel->updateAnchoredPosition(Anchor::Center, ccp(0, layerPos));
	m_checkpointLabel->updateAnchoredPosition(Anchor::Center, ccp(0, checkPos));
	m_nameLabel->updateAnchoredPosition(Anchor::Center, ccp(0, namePos));
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
