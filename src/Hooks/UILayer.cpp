#include "UILayer.hpp"
#include "PlayLayer.hpp"

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/UILayer.hpp>
#include <Geode/ui/Layout.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>

bool ModUILayer::init(GJBaseGameLayer* baseGameLayer) {
	if (!UILayer::init(baseGameLayer))
		return false;

	ModPlayLayer* playLayer;
	if (PlayLayer* gamePlayLayer = typeinfo_cast<PlayLayer*>(baseGameLayer)) {
		playLayer = static_cast<ModPlayLayer*>(gamePlayLayer);
	} else
		return true;

	// TODO Implement touch controls
	m_fields->m_switcherMenu = SwitcherMenu::create(playLayer);

	m_fields->m_switcherMenu->setVisible(false);

	addChild(m_fields->m_switcherMenu);

	createCheckpointCreateButton(
		m_checkpointMenu->getChildByID("add-checkpoint-button"), playLayer
	);
	createCheckpointRemoveButton(
		m_checkpointMenu->getChildByID("remove-checkpoint-button"), playLayer
	);

	return true;
}

void ModUILayer::updateSwitcher() {
	if (m_fields->m_switcherMenu == nullptr)
		return;

	ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());
	m_fields->m_switcherMenu->setVisible(
		playLayer->m_isPracticeMode &&
		(playLayer->m_fields->m_persistentCheckpointArray->count() > 0 ||
		 playLayer->m_fields->m_activeSaveLayer > 0)
	);

	m_fields->m_switcherMenu->m_checkpointLabel->setString(
		fmt::format(
			"{}/{}", playLayer->m_fields->m_activeCheckpoint,
			playLayer->m_fields->m_persistentCheckpointArray->count()
		)
			.c_str()
	);
	m_fields->m_switcherMenu->m_layerLabel->setString(
		fmt::format(
			"Layer {}/{}", playLayer->m_fields->m_activeSaveLayer + 1,
			playLayer->m_fields->m_saveLayerCount
		)
			.c_str()
	);

	const char* checkpointFrameName;

	if (playLayer->m_fields->m_activeCheckpoint > 0) {
		checkpointFrameName = "activeCheckpoint.png"_spr;
		m_fields->m_switcherMenu->m_checkpointSprite->setOpacity(255);
	} else {
		checkpointFrameName = "inactiveCheckpoint.png"_spr;
		m_fields->m_switcherMenu->m_checkpointSprite->setOpacity(192);
	}

	m_fields->m_switcherMenu->m_checkpointSprite->setDisplayFrame(
		CCSpriteFrameCache::get()->spriteFrameByName(checkpointFrameName)
	);
}

void createCheckpointCreateButton(CCNode* sibling, ModPlayLayer* playLayer) {
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

	sibling->getParent()->addChild(button);
}

void createCheckpointRemoveButton(CCNode* sibling, ModPlayLayer* playLayer) {
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

	sibling->getParent()->addChild(button);
}

// Code adapted from the Rewind mod https://github.com/undefined06855/Rewind
void createButtonBindsLabel(
	CCNode* parent, const keybinds::ActionID& action, bool right
) {
	CCNode* bindContainer = cocos2d::CCNode::create();
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
}
