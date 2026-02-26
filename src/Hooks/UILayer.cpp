#include "UILayer.hpp"
#include "PlayLayer.hpp"

$execute {
	Mod* mod = Mod::get();

	geode::listenForSettingChanges<bool>("switcher-enabled", [](bool value) {
		static_cast<ModUILayer*>(UILayer::get())->updateSwitcher();
	});

	for (std::string setting :
		  {"switcher-label-active-opacity", "switcher-label-inactive-opacity",
			"switcher-button-active-opacity", "switcher-button-inactive-opacity",
			"switcher-icon-active-opacity", "switcher-icon-inactive-opacity"}) {
		SettingChangedEvent(mod, setting)
			.listen(+[](std::shared_ptr<SettingV3> setting) {
				ModUILayer* uiLayer = static_cast<ModUILayer*>(UILayer::get());
				if (uiLayer != nullptr)
					uiLayer->resetSwitcherOpacity();
			})
			.leak();
	}

	geode::listenForSettingChanges<std::string>(
		"practice-buttons-position", [](std::string value) {
			ModUILayer* uiLayer = static_cast<ModUILayer*>(UILayer::get());
			setCheckpointButtonPosition(
				uiLayer->m_fields->m_createCheckpointButton,
				uiLayer->m_checkpointMenu->getChildByID("add-checkpoint-button"),
				false
			);
			setCheckpointButtonPosition(
				uiLayer->m_fields->m_removeCheckpointButton,
				uiLayer->m_checkpointMenu->getChildByID("remove-checkpoint-button"),
				true
			);

			std::tuple<geode::Anchor, CCPoint, CCPoint> leftPosition =
				getCheckpointButtonLabelPosition(false);
			std::tuple<geode::Anchor, CCPoint, CCPoint> rightPosition =
				getCheckpointButtonLabelPosition(true);

			CCNode* leftBind =
				uiLayer->m_fields->m_createCheckpointButton->getChildByID("binds");
			CCNode* rightBind =
				uiLayer->m_fields->m_removeCheckpointButton->getChildByID("binds");

			leftBind->setAnchorPoint(std::get<2>(leftPosition));
			leftBind->updateAnchoredPosition(
				std::get<0>(leftPosition), std::get<1>(leftPosition)
			);
			rightBind->setAnchorPoint(std::get<2>(rightPosition));
			rightBind->updateAnchoredPosition(
				std::get<0>(rightPosition), std::get<1>(rightPosition)
			);
		}
	);
}

bool ModUILayer::init(GJBaseGameLayer* baseGameLayer) {
	if (!UILayer::init(baseGameLayer))
		return false;

	ModPlayLayer* playLayer;
	if (PlayLayer* gamePlayLayer = typeinfo_cast<PlayLayer*>(baseGameLayer)) {
		playLayer = static_cast<ModPlayLayer*>(gamePlayLayer);
	} else
		return true;

	m_fields->m_switcherMenu = SwitcherMenu::create(playLayer);
	m_fields->m_switcherMenu->setPosition(getSwitcherPosition());
	m_fields->m_switcherMenu->setVisible(false);
	m_fields->m_switcherMenu->setEnabled(false);

	m_fields->m_switcherMenu->m_buttonMenu->setTouchEnabled(
#if defined(GEODE_IS_MOBILE)
		true
#else
		GameManager::get()->getGameVariable("0024")
#endif
	);

	addChild(m_fields->m_switcherMenu);

	m_fields->m_createCheckpointButton = createCheckpointCreateButton(
		m_checkpointMenu->getChildByID("add-checkpoint-button"), playLayer
	);
	m_fields->m_removeCheckpointButton = createCheckpointRemoveButton(
		m_checkpointMenu->getChildByID("remove-checkpoint-button"), playLayer
	);

	resetSwitcherOpacity();

	return true;
}

void ModUILayer::updateSwitcher() {
	if (m_fields->m_switcherMenu == nullptr || PlayLayer::get() == nullptr)
		return;

	Mod* mod = Mod::get();

	ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());
	LoadError loadError = playLayer->m_fields->m_loadError;

	bool switcherActive =
		Mod::get()->getSettingValue<bool>("switcher-enabled") &&
		playLayer->isPersistentSystemActive() &&
		// Save file exists
		(playLayer->m_fields->m_persistentCheckpointArray->count() > 0 ||
		 playLayer->m_fields->m_activeSaveLayer > 0 ||
		 loadError != LoadError::None);
	m_fields->m_switcherMenu->setVisible(switcherActive);
	m_fields->m_switcherMenu->setEnabled(switcherActive);

	if (loadError == LoadError::None)
		m_fields->m_switcherMenu->setColor(ccWHITE);
	else
		m_fields->m_switcherMenu->setColor(ccc3(224, 111, 111));

	std::string checkpointString;
	switch (loadError) {
	case None: {
		std::string ghostCheckpointString =
			playLayer->m_fields->m_ghostActiveCheckpoint == 0
				? ""
				: fmt::format(
					  " ({})", playLayer->m_fields->m_ghostActiveCheckpoint
				  );
		checkpointString = fmt::format(
			"{}{}/{}", playLayer->m_fields->m_activeCheckpoint,
			ghostCheckpointString,
			playLayer->m_fields->m_persistentCheckpointArray->count()
		);
		break;
	}
	case Crash:
		checkpointString = "BAD DATA";
		break;
	case OutdatedData:
		checkpointString = "OUTDATED";
		break;
	case NewData:
		checkpointString = "NEW VERS";
		break;
	case OtherPlatform:
		checkpointString = "PLATFORM";
		break;
	case LevelVersionMismatch:
		checkpointString = "LVL VERS";
		break;
	case BadFile:
		checkpointString = "BAD FILE";
		break;
	}

	m_fields->m_switcherMenu->m_checkpointLabel->setString(
		checkpointString.c_str()
	);
	m_fields->m_switcherMenu->m_layerLabel->setString(
		fmt::format(
			"Layer {}/{}", playLayer->m_fields->m_activeSaveLayer + 1,
			playLayer->m_fields->m_saveLayerCount
		)
			.c_str()
	);

	const char* checkpointFrameName;
	GLubyte checkpointSpriteOpacity;

	if (playLayer->m_fields->m_activeCheckpoint > 0) {
		checkpointFrameName = "activeCheckpoint.png"_spr;
		checkpointSpriteOpacity = 255;
	} else {
		checkpointFrameName = "inactiveCheckpoint.png"_spr;
		checkpointSpriteOpacity = 192;
	}

	m_fields->m_switcherMenu->m_checkpointSprite->setDisplayFrame(
		CCSpriteFrameCache::get()->spriteFrameByName(checkpointFrameName)
	);

	double labelActiveOpacity =
		255 * mod->getSettingValue<double>("switcher-label-active-opacity");
	double labelInactiveOpacity =
		255 * mod->getSettingValue<double>("switcher-label-inactive-opacity");
	double buttonActiveOpacity =
		255 * mod->getSettingValue<double>("switcher-button-active-opacity");
	double buttonInactiveOpacity =
		255 * mod->getSettingValue<double>("switcher-button-inactive-opacity");
	double iconActiveOpacity =
		checkpointSpriteOpacity *
		mod->getSettingValue<double>("switcher-icon-active-opacity");
	double iconInactiveOpacity =
		checkpointSpriteOpacity *
		mod->getSettingValue<double>("switcher-icon-inactive-opacity");

	if (!playLayer->isModUIVisible()) {
		labelInactiveOpacity = 0;
		buttonInactiveOpacity = 0;
		iconInactiveOpacity = 0;
	}

	std::vector<std::tuple<CCNode*, GLubyte, GLubyte>> nodes;
	if (labelActiveOpacity != labelInactiveOpacity)
		nodes.push_back(
			std::make_tuple(
				m_fields->m_switcherMenu->m_labelMenu, labelActiveOpacity,
				labelInactiveOpacity
			)
		);
	if (buttonActiveOpacity != buttonInactiveOpacity)
		nodes.push_back(
			std::make_tuple(
				m_fields->m_switcherMenu->m_buttonMenu, buttonActiveOpacity,
				buttonInactiveOpacity
			)
		);
	if (iconActiveOpacity != iconInactiveOpacity)
		nodes.push_back(
			std::make_tuple(
				m_fields->m_switcherMenu->m_checkpointSprite, iconActiveOpacity,
				iconInactiveOpacity
			)
		);

	for (std::tuple<CCNode*, GLubyte, GLubyte> tuple : nodes) {
		CCNode* node = std::get<0>(tuple);
		GLubyte activeOpacity = std::get<1>(tuple);
		GLubyte inactiveOpacity = std::get<2>(tuple);

		node->stopActionByTag(55);

		CCSequence* sequence = CCSequence::create(
			CCEaseInOut::create(CCFadeTo::create(0.15f, activeOpacity), 2.f),
			CCDelayTime::create(1.75f),
			CCEaseInOut::create(CCFadeTo::create(0.8f, inactiveOpacity), 2.f),
			nullptr
		);
		sequence->setTag(55);

		node->runAction(sequence);
	}
}

void ModUILayer::resetSwitcherOpacity() {
	Mod* mod = Mod::get();

	m_fields->m_switcherMenu->m_labelMenu->stopActionByTag(55);
	m_fields->m_switcherMenu->m_buttonMenu->stopActionByTag(55);
	m_fields->m_switcherMenu->m_checkpointSprite->stopActionByTag(55);
	m_fields->m_switcherMenu->m_labelMenu->setOpacity(
		255 * mod->getSettingValue<double>("switcher-label-active-opacity")
	);
	m_fields->m_switcherMenu->m_buttonMenu->setOpacity(
		255 * mod->getSettingValue<double>("switcher-button-active-opacity")
	);
	m_fields->m_switcherMenu->m_checkpointSprite->setOpacity(
		255 * mod->getSettingValue<double>("switcher-icon-active-opacity")
	);

	updateSwitcher();
}

CCNodeRGBA*
createCheckpointCreateButton(CCNode* sibling, ModPlayLayer* playLayer) {
	CCNodeRGBA* button;

	CCSprite* sprite =
		CCSprite::createWithSpriteFrameName("markCheckpoint.png"_spr);
	if (playLayer != nullptr) {
		CCMenuItemSpriteExtra* trueButton = CCMenuItemExt::createSpriteExtra(
			sprite, [playLayer](CCObject* sender) {
				playLayer->markPersistentCheckpoint();
			}
		);
		trueButton->m_scaleMultiplier = 1.1;

		button = trueButton;
	} else
		button = sprite;

	setCheckpointButtonPosition(button, sibling, false);
	button->setID("markPersistentCheckpoint"_spr);

#ifdef GEODE_IS_DESKTOP
	if (playLayer != nullptr)
		createButtonBindsLabel(button, "keybind-create_checkpoint", false);
#endif

	button->setCascadeOpacityEnabled(true);
	button->setOpacity(GameManager::get()->m_practiceOpacity * 255);

	sibling->getParent()->addChild(button);

	return button;
}

CCNodeRGBA*
createCheckpointRemoveButton(CCNode* sibling, ModPlayLayer* playLayer) {
	CCNodeRGBA* button;

	CCSprite* sprite =
		CCSprite::createWithSpriteFrameName("removeCheckpoint.png"_spr);
	if (playLayer != nullptr) {
		CCMenuItemSpriteExtra* trueButton = CCMenuItemExt::createSpriteExtra(
			sprite, [playLayer](CCObject* sender) {
				playLayer->removeCurrentPersistentCheckpoint();
			}
		);
		trueButton->m_scaleMultiplier = 1.1;

		button = trueButton;
	} else
		button = sprite;

	setCheckpointButtonPosition(button, sibling, true);
	button->setID("removePersistentCheckpoint"_spr);

#ifdef GEODE_IS_DESKTOP
	if (playLayer != nullptr)
		createButtonBindsLabel(button, "keybind-remove-checkpoint", true);
#endif

	button->setCascadeOpacityEnabled(true);
	button->setOpacity(GameManager::get()->m_practiceOpacity * 255);

	sibling->getParent()->addChild(button);

	return button;
}

// Code adapted from the Rewind mod https://github.com/undefined06855/Rewind
void createButtonBindsLabel(
	CCNode* parent, const std::string& action, bool right
) {
	CCNodeRGBA* bindContainer = cocos2d::CCNodeRGBA::create();
	bindContainer->setScale(.65f);
	bool first = true;
	for (auto bind : Mod::get()->getSettingValue<std::vector<Keybind>>(action)) {
		if (!first) {
			bindContainer->addChild(
				cocos2d::CCLabelBMFont::create("/", "bigFont.fnt")
			);
		}
		first = false;
		bindContainer->addChild(bind.createNode());
	}
	bindContainer->setID("binds");
	bindContainer->setContentSize({95, 40.f});
	CCPoint anchorPoint, positionOffset;
	bindContainer->setLayout(geode::RowLayout::create());

	std::tuple<geode::Anchor, CCPoint, CCPoint> position =
		getCheckpointButtonLabelPosition(right);
	bindContainer->setAnchorPoint(std::get<2>(position));
	parent->addChildAtPosition(
		bindContainer, std::get<0>(position), std::get<1>(position)
	);

	bindContainer->setCascadeOpacityEnabled(true);
}

void setCheckpointButtonPosition(
	CCNodeRGBA* button, CCNode* sibling, bool invertX
) {
	CCPoint offset;

	std::string settingValue =
		Mod::get()->getSettingValue<std::string>("practice-buttons-position");

	if (settingValue == "Above")
		offset = ccp(0, sibling->getContentHeight() + 10);
	else if (settingValue == "Below")
		offset = ccp(0, -sibling->getContentHeight() - 10);
	else
		offset = ccp(
			(-(sibling->getContentWidth() + button->getContentWidth()) / 2 - 10) *
				(invertX ? -1 : 1),
			0
		);

	button->setPosition(sibling->getPosition() + offset);
}

std::tuple<geode::Anchor, CCPoint, CCPoint>
getCheckpointButtonLabelPosition(bool right) {
	std::string settingValue =
		Mod::get()->getSettingValue<std::string>("practice-buttons-position");

	if (settingValue == "Horizontal") {
		if (right)
			return std::make_tuple(
				geode::Anchor::BottomLeft, ccp(-4.f, -1.f), ccp(0.f, 0.5f)
			);
		else
			return std::make_tuple(
				geode::Anchor::BottomRight, ccp(4.f, -1.f), ccp(1.f, 0.5f)
			);
	}

	return std::make_tuple(
		geode::Anchor::Bottom, ccp(0.f, -1.f), ccp(0.5f, 0.5f)
	);
}
