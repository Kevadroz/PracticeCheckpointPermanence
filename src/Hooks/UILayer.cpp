#include "UILayer.hpp"
#include "PlayLayer.hpp"

#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>
#endif

$execute {
	Mod* mod = Mod::get();

	for (std::string setting :
		  {"switcher-label-active-opacity", "switcher-label-inactive-opacity",
			"switcher-button-active-opacity", "switcher-button-inactive-opacity",
			"switcher-icon-active-opacity", "switcher-icon-inactive-opacity"}) {
		new EventListener<SettingChangedFilterV3>(
			+[](std::shared_ptr<SettingV3> setting) {
				ModUILayer* uiLayer = static_cast<ModUILayer*>(UILayer::get());
				if (uiLayer != nullptr)
					uiLayer->resetSwitcherOpacity();
			},
			SettingChangedFilterV3(mod, setting)
		);
	}
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
	m_fields->m_switcherMenu->setVisible(false);

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

	return true;
}

void ModUILayer::updateSwitcher() {
	if (m_fields->m_switcherMenu == nullptr || PlayLayer::get() == nullptr)
		return;

	Mod* mod = Mod::get();

	ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());
	LoadError loadError = playLayer->m_fields->m_loadError;

	m_fields->m_switcherMenu->setVisible(
		playLayer->m_isPracticeMode &&
		(playLayer->m_fields->m_persistentCheckpointArray->count() > 0 ||
		 playLayer->m_fields->m_activeSaveLayer > 0 ||
		 loadError != LoadError::None)
	);

	if (loadError == LoadError::None)
		m_fields->m_switcherMenu->setColor(ccWHITE);
	else
		m_fields->m_switcherMenu->setColor(ccc3(224, 111, 111));

	std::string checkpointString;
	switch (loadError) {
	case None:
		checkpointString = fmt::format(
			"{}/{}", playLayer->m_fields->m_activeCheckpoint,
			playLayer->m_fields->m_persistentCheckpointArray->count()
		);
		break;
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

	button->setPosition(
		sibling->getPosition() + ccp(-sibling->getContentWidth() / 2 - 10, 0)
	);
	button->setAnchorPoint(ccp(1, .5));
	button->setID("markPersistentCheckpoint"_spr);

	if (playLayer != nullptr)
		createButtonBindsLabel(button, "create_checkpoint"_spr, false);

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

	button->setPosition(
		sibling->getPosition() + ccp(sibling->getContentWidth() / 2 + 10, 0)
	);
	button->setAnchorPoint(ccp(0, .5));
	button->setID("removePersistentCheckpoint"_spr);

	if (playLayer != nullptr)
		createButtonBindsLabel(button, "remove_checkpoint"_spr, true);

	button->setCascadeOpacityEnabled(true);
	button->setOpacity(GameManager::get()->m_practiceOpacity * 255);

	sibling->getParent()->addChild(button);

	return button;
}

// Code adapted from the Rewind mod https://github.com/undefined06855/Rewind
void createButtonBindsLabel(
	CCNode* parent, const std::string& action, bool right
) {
#ifndef GEODE_IS_IOS
	CCNodeRGBA* bindContainer = cocos2d::CCNodeRGBA::create();
	bindContainer->setScale(.65f);
	bool first = true;
	for (auto& bind : keybinds::BindManager::get()->getBindsFor(action)) {
		if (!first) {
			bindContainer->addChild(
				cocos2d::CCLabelBMFont::create("/", "bigFont.fnt")
			);
		}
		first = false;
		bindContainer->addChild(bind->createLabel());
	}
	bindContainer->setID("binds");
	bindContainer->setContentSize({95, 40.f});
	bindContainer->setLayout(geode::RowLayout::create());
	if (right) {
		bindContainer->setAnchorPoint({0.f, .5f});
		parent->addChildAtPosition(
			bindContainer, geode::Anchor::BottomLeft, {-4.f, -1.f}
		);
	} else {
		bindContainer->setAnchorPoint({1.f, .5f});
		parent->addChildAtPosition(
			bindContainer, geode::Anchor::BottomRight, {4.f, -1.f}
		);
	}
	bindContainer->setCascadeOpacityEnabled(true);
#endif
}
