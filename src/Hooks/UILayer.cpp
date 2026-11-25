#include "UILayer.hpp"
#include "PlayLayer.hpp"

$execute {
	Mod* mod = Mod::get();

	listenForSettingChanges<bool>("switcher-enabled", [](bool value) {
		ModUILayer* uiLayer = static_cast<ModUILayer*>(UILayer::get());
		if (uiLayer != nullptr)
			uiLayer->updateSwitcher();
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

	for (std::string setting :
		  {"switcher-button-active-opacity", "switcher-button-inactive-opacity",
			"switcher-icon-active-opacity", "switcher-icon-inactive-opacity",
			"switcher-label-error", "switcher-label-layers",
			"switcher-label-checkpoints", "switcher-label-name"}) {
		SettingChangedEvent(mod, setting)
			.listen(+[](std::shared_ptr<SettingV3> setting) {
				ModUILayer* uiLayer = static_cast<ModUILayer*>(UILayer::get());
				if (uiLayer != nullptr)
					uiLayer->m_fields->m_switcherMenu->updateLabels();
			})
			.leak();
	}

	listenForSettingChanges<std::string>(
		"practice-buttons-position", [](std::string value) {
			ModUILayer* uiLayer = static_cast<ModUILayer*>(UILayer::get());
			if (uiLayer == nullptr)
				return;

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

			std::tuple<Anchor, CCPoint, CCPoint> leftPosition =
				getCheckpointButtonLabelPosition(false);
			std::tuple<Anchor, CCPoint, CCPoint> rightPosition =
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

	listenForSettingChanges<float>(
		"practice-buttons-distance", [](float value) {
			ModUILayer* uiLayer = static_cast<ModUILayer*>(UILayer::get());
			if (uiLayer == nullptr)
				return;

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
	m_fields->m_switcherMenu->setPosition(SwitcherMenu::getSwitcherPosition());
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

	PersistentCheckpoint* checkpoint = nullptr;
	if (playLayer->m_fields->m_activeCheckpoint > 0)
		checkpoint = static_cast<PersistentCheckpoint*>(
			playLayer->m_fields->m_persistentCheckpointArray->objectAtIndex(
				playLayer->m_fields->m_activeCheckpoint - 1
			)
		);

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
	else if (SaveParser::isErrorFallbackCapable(loadError))
		m_fields->m_switcherMenu->setColor(ccc3(224, 166, 111));
	else
		m_fields->m_switcherMenu->setColor(ccc3(224, 111, 111));

	std::string nameString = "";
	if (checkpoint != nullptr) {
		if (checkpoint->m_checkpointName.empty())
			nameString = checkpoint->getDefaultLabel(playLayer->m_isPlatformer);
		else
			nameString = checkpoint->m_checkpointName;
	}
	std::string ghostCheckpointString =
		playLayer->m_fields->m_ghostActiveCheckpoint == 0
			? ""
			: fmt::format(" ({})", playLayer->m_fields->m_ghostActiveCheckpoint);
	std::string checkpointString = fmt::format(
		"{}{}/{}", playLayer->m_fields->m_activeCheckpoint, ghostCheckpointString,
		playLayer->m_fields->m_persistentCheckpointArray->count()
	);
	std::string layerString = fmt::format(
		"Lay {}/{}", playLayer->m_fields->m_activeSaveLayer + 1,
		playLayer->m_fields->m_saveLayerCount
	);
	const char* errorString = "";
	if (checkpoint != nullptr && checkpoint->m_isDegraded)
		errorString = "DEGRADED";
	switch (loadError) {
	case LoadError::Crash:
		errorString = "BAD DATA";
		break;
	case LoadError::GameVersionMismatch:
		errorString = "GMD VERS";
		break;
	case LoadError::OutdatedData:
		errorString = "OUTDATED";
		break;
	case LoadError::NewData:
		errorString = "NEW VERS";
		break;
	case LoadError::OtherPlatform:
		errorString = "PLATFORM";
		break;
	case LoadError::LevelVersionMismatch:
		errorString = "LVL VERS";
		break;
	case LoadError::BadFile:
		errorString = "BAD FILE";
		break;
	case LoadError::ForcedFallback:
		errorString = "FALLBACK";
		break;
	default:
		break;
	}

	m_fields->m_switcherMenu->m_nameLabel->setVisible(checkpoint != nullptr);
	m_fields->m_switcherMenu->m_nameLabel->setString(nameString.c_str());
	m_fields->m_switcherMenu->m_checkpointLabel->setString(
		checkpointString.c_str()
	);
	m_fields->m_switcherMenu->m_layerLabel->setString(layerString.c_str());
	m_fields->m_switcherMenu->m_errorLabel->setVisible(
		playLayer->m_fields->m_loadError != LoadError::None ||
		playLayer->isDegradedCheckpoint()
	);
	m_fields->m_switcherMenu->m_errorLabel->setString(errorString);

	CCSpriteFrame* checkpointSpriteBase;
	CCSpriteFrame* checkpointSpriteColor = nullptr;
	ccColor3B checkpointColor;
	GLubyte checkpointSpriteOpacity;

	if (playLayer->m_fields->m_activeCheckpoint > 0) {
		checkpointSpriteBase = checkpoint->m_physicalObject->displayFrame();
		checkpointSpriteColor = checkpoint->m_physicalObjectColor->displayFrame();
		checkpointColor = checkpoint->m_physicalObjectColor->getColor();
		checkpointSpriteOpacity = 255;
	} else {
		checkpointSpriteBase = CCSpriteFrameCache::get()->spriteFrameByName(
			playLayer->m_fields->m_loadError == LoadError::None
				? "inactiveCheckpoint_base.png"_spr
				: "inactiveDegradedCheckpoint_base.png"_spr
		);
		checkpointSpriteColor = CCSpriteFrameCache::get()->spriteFrameByName(
			playLayer->m_fields->m_loadError == LoadError::None
				? "inactiveCheckpoint_default.png"_spr
				: "inactiveDegradedCheckpoint_default.png"_spr
		);
		checkpointColor = ccWHITE;
		checkpointSpriteOpacity = 192;
	}

	m_fields->m_switcherMenu->m_checkpointSpriteBase->setDisplayFrame(
		checkpointSpriteBase
	);
	m_fields->m_switcherMenu->m_checkpointSpriteColor->setDisplayFrame(
		checkpointSpriteColor
	);
	m_fields->m_switcherMenu->m_checkpointSpriteColor->setColor(checkpointColor);

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
				m_fields->m_switcherMenu->m_checkpointSpriteBase, iconActiveOpacity,
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
	m_fields->m_switcherMenu->m_checkpointSpriteBase->stopActionByTag(55);
	m_fields->m_switcherMenu->m_labelMenu->setOpacity(
		255 * mod->getSettingValue<double>("switcher-label-active-opacity")
	);
	m_fields->m_switcherMenu->m_buttonMenu->setOpacity(
		255 * mod->getSettingValue<double>("switcher-button-active-opacity")
	);
	m_fields->m_switcherMenu->m_checkpointSpriteBase->setOpacity(
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
		createButtonBindsLabel(button, "keybind-create-checkpoint", false);
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
	CCNodeRGBA* bindContainer = CCNodeRGBA::create();
	bindContainer->setScale(.65f);
	bool first = true;
	for (auto bind : Mod::get()->getSettingValue<std::vector<Keybind>>(action)) {
		if (!first) {
			bindContainer->addChild(Label::create("/", "bigFont.fnt"));
		}
		first = false;
		bindContainer->addChild(bind.createNode());
	}
	bindContainer->setID("binds");
	bindContainer->setContentSize({95, 40.f});
	CCPoint anchorPoint, positionOffset;
	bindContainer->setLayout(RowLayout::create());

	std::tuple<Anchor, CCPoint, CCPoint> position =
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

	std::string side =
		Mod::get()->getSettingValue<std::string>("practice-buttons-position");
	float distance =
		Mod::get()->getSettingValue<float>("practice-buttons-distance");

	if (side == "Above")
		offset = ccp(0, sibling->getContentHeight() + 10 + distance);
	else if (side == "Below")
		offset = ccp(0, -sibling->getContentHeight() - 10 - distance);
	else
		offset = ccp(
			(-(sibling->getContentWidth() + button->getContentWidth()) / 2 - 10 -
			 distance) *
				(invertX ? -1 : 1),
			0
		);

	button->setPosition(sibling->getPosition() + offset);
}

std::tuple<Anchor, CCPoint, CCPoint>
getCheckpointButtonLabelPosition(bool right) {
	std::string settingValue =
		Mod::get()->getSettingValue<std::string>("practice-buttons-position");

	if (settingValue == "Horizontal") {
		if (right)
			return std::make_tuple(
				Anchor::BottomLeft, ccp(-4.f, -1.f), ccp(0.f, 0.5f)
			);
		else
			return std::make_tuple(
				Anchor::BottomRight, ccp(4.f, -1.f), ccp(1.f, 0.5f)
			);
	}

	return std::make_tuple(
		Anchor::Bottom, ccp(0.f, -1.f), ccp(0.5f, 0.5f)
	);
}
