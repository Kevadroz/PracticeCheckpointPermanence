#include "CheckpointManager.hpp"
#include "../Hooks/PlayLayer.hpp"
#include "Geode/ui/Layout.hpp"

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

CheckpointManager* CheckpointManager::create() {
	auto ret = new CheckpointManager;
	if (ret->init()) {
		ret->autorelease();
		return ret;
	}

	delete ret;
	return nullptr;
}

bool CheckpointManager::init() {
	if (!Popup::init(260.f, 280.f, "GJ_square02.png"))
		return false;

	ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());
	bool hasCheckpoints =
		playLayer->m_fields->m_persistentCheckpointArray->count() > 0;

	m_noElasticity = true;

	setTitle("Persistent Checkpoints");

	CCSprite* optionsSpr =
		CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
	CCMenuItemSpriteExtra* optionsButton =
		CCMenuItemExt::createSpriteExtra(optionsSpr, [](CCNode* sender) {
			geode::openSettingsPopup(Mod::get(), true);
		});
	optionsButton->m_baseScale = .6;
	optionsButton->setScale(.6);

	CCSprite* deleteSprite =
		CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
	m_deleteButton = CCMenuItemExt::createSpriteExtra(
		deleteSprite, [this, playLayer](CCMenuItemSpriteExtra* deleteButton) {
			if (playLayer->m_fields->m_persistentCheckpointArray->count() > 0 ||
				 playLayer->m_fields->m_loadError != LoadError::None)
				geode::createQuickPopup(
					"Delete All",
					"Delete all saved checkpoints for this layer?\n"
					"This action cannot be undone.",
					"Cancel", "Delete", [this, playLayer](auto, bool confirmed) {
						if (confirmed) {
							playLayer->removeCurrentSaveLayer();
							updateUIElements(true);
						}
					}
				);
			else
				FLAlertLayer::create(
					"No checkpoints", "Can't delete what doesn't exist!", "Ok"
				)
					->show();
		}
	);
	m_deleteButton->m_baseScale = .6;
	m_deleteButton->setScale(.6);

	if (!hasCheckpoints) {
		m_deleteButton->setColor(ccc3(90, 90, 90));
		m_deleteButton->setOpacity(205);
	}

	m_saveLayerLabel = CCLabelBMFont::create("", "bigFont.fnt");
	m_saveLayerLabel->setScale(.6);

	CCSprite* previousLayerSpr =
		CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	CCSprite* nextLayerSpr =
		CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	nextLayerSpr->setFlipX(true);

	m_previousLayerBtn = CCMenuItemExt::createSpriteExtra(
		previousLayerSpr, [this, playLayer](CCMenuItemSpriteExtra* sender) {
			playLayer->previousSaveLayer();
			updateUIElements(true);
		}
	);
	m_nextLayerBtn = CCMenuItemExt::createSpriteExtra(
		nextLayerSpr, [this, playLayer](CCMenuItemSpriteExtra* sender) {
			playLayer->nextSaveLayer();
			updateUIElements(true);
		}
	);
	m_previousLayerBtn->m_baseScale = .6;
	m_nextLayerBtn->m_baseScale = .6;
	m_previousLayerBtn->setScale(.6);
	m_nextLayerBtn->setScale(.6);

	CCSprite* moveLayerBackSpr =
		CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
	CCSprite* moveLayerForwardSpr =
		CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
	moveLayerBackSpr->setFlipX(true);

	m_moveLayerBackBtn = CCMenuItemExt::createSpriteExtra(
		moveLayerBackSpr, [this, playLayer](CCMenuItemSpriteExtra* sender) {
			playLayer->updateSaveLayerCount();
			unsigned int saveLayer = playLayer->m_fields->m_activeSaveLayer;
			if (saveLayer > 0 &&
				 saveLayer < playLayer->m_fields->m_saveLayerCount) {
				playLayer->swapSaveLayers(saveLayer, saveLayer - 1);
				playLayer->m_fields->m_activeSaveLayer--;

				updateUIElements(true);
			}
		}
	);
	m_moveLayerForwardBtn = CCMenuItemExt::createSpriteExtra(
		moveLayerForwardSpr, [this, playLayer](CCMenuItemSpriteExtra* sender) {
			playLayer->updateSaveLayerCount();
			unsigned int saveLayer = playLayer->m_fields->m_activeSaveLayer;
			if (saveLayer + 1 < playLayer->m_fields->m_saveLayerCount) {
				playLayer->swapSaveLayers(saveLayer, saveLayer + 1);
				playLayer->m_fields->m_activeSaveLayer++;

				updateUIElements(true);
			}
		}
	);
	m_moveLayerBackBtn->m_baseScale = .4;
	m_moveLayerForwardBtn->m_baseScale = .4;
	m_moveLayerBackBtn->setScale(.4);
	m_moveLayerForwardBtn->setScale(.4);

	m_listContainer = CCLayerColor::create();
	m_listContainer->setContentSize(ccp(230, 190));
	m_listContainer->setAnchorPoint(ccp(.5, 1));
	m_listContainer->setColor(ccc3(74, 97, 225));
	m_listContainer->setOpacity(255);
	m_listContainer->setZOrder(5);

	m_buttonMenu->addChildAtPosition(
		optionsButton, geode::Anchor::BottomLeft, ccp(3, 3)
	);
	m_buttonMenu->addChildAtPosition(
		m_deleteButton, geode::Anchor::BottomRight, ccp(-3, 3)
	);
	m_buttonMenu->addChildAtPosition(
		m_previousLayerBtn, geode::Anchor::Top, ccp(0, -45)
	);
	m_buttonMenu->addChildAtPosition(
		m_nextLayerBtn, geode::Anchor::Top, ccp(0, -45)
	);
	m_buttonMenu->addChildAtPosition(
		m_moveLayerBackBtn, geode::Anchor::TopLeft, ccp(15, -45)
	);
	m_buttonMenu->addChildAtPosition(
		m_moveLayerForwardBtn, geode::Anchor::TopRight, ccp(-15, -45)
	);
	m_mainLayer->addChildAtPosition(
		m_saveLayerLabel, geode::Anchor::Top, ccp(0, -45)
	);
	m_mainLayer->addChildAtPosition(
		m_listContainer, geode::Anchor::Top, ccp(0, -65)
	);

	m_emptyListLabel = CCLabelBMFont::create("", "bigFont.fnt", 215);
	m_emptyListLabel->setScale(.7);
	m_emptyListLabel->setOpacity(150);
	m_emptyListLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	m_emptyListLabel->setVisible(!hasCheckpoints);

	ButtonSprite* forceLoadButtonSprite = ButtonSprite::create("Force Load");
	m_forceLoadButton = CCMenuItemExt::createSpriteExtra(
		forceLoadButtonSprite,
		[this, playLayer](CCMenuItemSpriteExtra* forceButton) {
			geode::createQuickPopup(
				"Force Load",
				"This will force load and resave this layer.\n"
				"When switching to a checkpoint the game will likely crash or the "
				"level will break.\n"
				"If things go very wrong you can try opening the checkpoint "
				"manager outside of practice mode to remove all checkpoints.\n"
				"Are you sure about this?",
				"No", "YOLO", [this, playLayer](auto, bool confirmed) {
					if (confirmed) {
						playLayer->deserializeCheckpoints(true);
						playLayer->serializeCheckpoints();

						updateUIElements();
						playLayer->updateModUI();
					}
				}
			);
		}
	);
	m_forceLoadButton->setVisible(
		playLayer->m_fields->m_loadError != LoadError::None
	);
	m_forceLoadButton->m_baseScale = 0.7;
	m_forceLoadButton->setScale(0.7);

	updateUIElements();

	ListBorders* borders = ListBorders::create();
	borders->setContentSize(m_listView->getContentSize() + ccp(7, 7));
	borders->setZOrder(15);
	borders->setSpriteFrames(
		"GJ_commentTop2_001.png", "GJ_commentSide2_001.png"
	);

	m_listContainer->addChildAtPosition(borders, geode::Anchor::Center);
	m_listContainer->addChildAtPosition(m_emptyListLabel, geode::Anchor::Center);
	m_buttonMenu->addChildAtPosition(m_forceLoadButton, geode::Anchor::Bottom);

	return true;
}

void CheckpointManager::createList(bool resetPosition) {
	ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());
	if (playLayer == nullptr)
		return;

	bool recreated = m_listView != nullptr;
	bool carryPosition =
		!resetPosition && recreated &&
		m_listView->getChildByIndex(0)->getChildByIndex(0)->getChildrenCount() >
			0;
	float contentPosition;

	if (recreated) {
		m_cellsArray->removeAllObjects();
		m_listView->removeFromParent();

		if (carryPosition)
			contentPosition =
				m_listView->getChildByIndex(0)->getChildByIndex(0)->getPositionY();
	}

	CCArray* checkpointArray = playLayer->m_fields->m_persistentCheckpointArray;
	for (PersistentCheckpoint* checkpoint :
		  CCArrayExt<PersistentCheckpoint*>(checkpointArray)) {
		unsigned int index = checkpointArray->indexOfObject(checkpoint);

		std::function<void(CCMenuItemSpriteExtra*)> moveUpCallback = nullptr;
		std::function<void(CCMenuItemSpriteExtra*)> moveDownCallback = nullptr;

		if (index != 0)
			moveUpCallback = [this, index,
									playLayer](CCMenuItemSpriteExtra* sender) {
				playLayer->swapPersistentCheckpoints(index, index - 1);
				createList();
			};
		if (index != checkpointArray->count() - 1)
			moveDownCallback = [this, index,
									  playLayer](CCMenuItemSpriteExtra* sender) {
				playLayer->swapPersistentCheckpoints(index, index + 1);
				createList();
			};

		m_cellsArray->addObject(createCheckpointCell(
			checkpoint, moveUpCallback, moveDownCallback,
			[this, checkpoint, playLayer](CCMenuItemSpriteExtra* sender) {
				unsigned int index =
					playLayer->m_fields->m_persistentCheckpointArray->indexOfObject(
						checkpoint
					) +
					1;

				if (playLayer->m_fields->m_activeCheckpoint == index)
					index = 0;

				playLayer->switchCurrentCheckpoint(index);
				createList();
			},
			[this, checkpoint, playLayer](CCMenuItemSpriteExtra* sender) {
				bool updateLabel =
					playLayer->m_fields->m_persistentCheckpointArray->count() == 1;
				playLayer->removePersistentCheckpoint(checkpoint);

				if (updateLabel)
					updateUIElements();
				else
					createList();
			}
		));
	}

	m_listView = ListView::create(
		m_cellsArray, 40, m_listContainer->getContentWidth(),
		m_listContainer->getContentHeight()
	);
	m_listView->setPrimaryCellColor(ccc3(76, 105, 250));
	m_listView->setSecondaryCellColor(ccc3(68, 91, 210));
	m_listView->setZOrder(10);

	CCNode* listContent = m_listView->getChildByIndex(0)->getChildByIndex(0);
	if (carryPosition && checkpointArray->count() >= 5) {
		listContent->setPositionY(contentPosition);
		if (listContent->getPositionY() > 0)
			listContent->setPositionY(0);
	}

	m_listContainer->addChildAtPosition(m_listView, geode::Anchor::BottomLeft);
}

void CheckpointManager::updateUIElements(bool resetListPosition) {
	ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());
	if (playLayer == nullptr)
		return;

	unsigned int saveLayer = playLayer->m_fields->m_activeSaveLayer;

	m_saveLayerLabel->setString(
		fmt::format(
			"Layer: {}/{}", saveLayer + 1, playLayer->m_fields->m_saveLayerCount
		)
			.c_str()
	);

	createList(resetListPosition);

	if (playLayer->m_fields->m_persistentCheckpointArray->count() == 0) {
		const char* text;
		switch (playLayer->m_fields->m_loadError) {
		case None:
			if (saveLayer == 0)
				text = "No checkpoints saved in this level.";
			else
				text = "No checkpoints saved in the current layer.";
			break;
		case Crash:
			text = "Error while loading saved checkpoints.";
			break;
		case OutdatedData:
			text = "The version of the mod the checkpoints were saved in is no "
					 "longer supported.";
			break;
		case NewData:
			text = "The checkpoints were saved with a newer version of the mod.";
			break;
		case OtherPlatform:
			text = "The checkpoints were saved in another platform or device.";
			break;
		case LevelVersionMismatch:
			text = "The level version has changed, the checkpoints cannot "
					 "be loaded.";
			break;
		}

		bool hasLoadError = playLayer->m_fields->m_loadError != LoadError::None;

		m_emptyListLabel->setString(text);
		m_emptyListLabel->setVisible(true);
		m_emptyListLabel->setColor(hasLoadError ? ccc3(224, 111, 111) : ccWHITE);

		if (hasLoadError) {
			m_deleteButton->setColor(ccc3(255, 255, 255));
			m_deleteButton->setOpacity(255);
		} else {
			m_deleteButton->setColor(ccc3(90, 90, 90));
			m_deleteButton->setOpacity(200);
		}
	} else {
		m_emptyListLabel->setVisible(false);

		m_deleteButton->setColor(ccc3(255, 255, 255));
		m_deleteButton->setOpacity(255);
	}

	m_forceLoadButton->setVisible(
		playLayer->m_fields->m_loadError != LoadError::None
	);

	float layerSwitchOffset =
		m_saveLayerLabel->getScaledContentWidth() / 2.f + 15.f;
	if (AnchorLayoutOptions* options = typeinfo_cast<AnchorLayoutOptions*>(
			 m_previousLayerBtn->getLayoutOptions()
		 ))
		options->setOffset(ccp(-layerSwitchOffset, options->getOffset().y));
	if (AnchorLayoutOptions* options = typeinfo_cast<AnchorLayoutOptions*>(
			 m_nextLayerBtn->getLayoutOptions()
		 ))
		options->setOffset(ccp(layerSwitchOffset, options->getOffset().y));

	if (playLayer->m_fields->m_saveLayerCount > 0) {
		m_previousLayerBtn->m_animationEnabled = true;
		m_previousLayerBtn->setColor(ccc3(255, 255, 255));
		m_previousLayerBtn->setOpacity(255);
		m_nextLayerBtn->m_animationEnabled = true;
		m_nextLayerBtn->setColor(ccc3(255, 255, 255));
		m_nextLayerBtn->setOpacity(255);
	} else {
		m_previousLayerBtn->m_animationEnabled = false;
		m_previousLayerBtn->setColor(ccc3(90, 90, 90));
		m_previousLayerBtn->setOpacity(200);
		m_nextLayerBtn->m_animationEnabled = false;
		m_nextLayerBtn->setColor(ccc3(90, 90, 90));
		m_nextLayerBtn->setOpacity(200);
	}
	if (saveLayer > 0 && saveLayer < playLayer->m_fields->m_saveLayerCount) {
		m_moveLayerBackBtn->m_animationEnabled = true;
		m_moveLayerBackBtn->setColor(ccc3(255, 255, 255));
		m_moveLayerBackBtn->setOpacity(255);
	} else {
		m_moveLayerBackBtn->m_animationEnabled = false;
		m_moveLayerBackBtn->setColor(ccc3(90, 90, 90));
		m_moveLayerBackBtn->setOpacity(200);
	}
	if (saveLayer + 1 < playLayer->m_fields->m_saveLayerCount) {
		m_moveLayerForwardBtn->m_animationEnabled = true;
		m_moveLayerForwardBtn->setColor(ccc3(255, 255, 255));
		m_moveLayerForwardBtn->setOpacity(255);
	} else {
		m_moveLayerForwardBtn->m_animationEnabled = false;
		m_moveLayerForwardBtn->setColor(ccc3(90, 90, 90));
		m_moveLayerForwardBtn->setOpacity(200);
	}

	m_buttonMenu->updateLayout(false);
}

CCNode* createCheckpointCell(
	PersistentCheckpoint* checkpoint,
	std::function<void(CCMenuItemSpriteExtra*)> moveUpCallback,
	std::function<void(CCMenuItemSpriteExtra*)> moveDownCallback,
	std::function<void(CCMenuItemSpriteExtra*)> selectCallback,
	std::function<void(CCMenuItemSpriteExtra*)> removeCallback
) {
	PlayLayer* playLayer = PlayLayer::get();

	CCMenu* menu = CCMenu::create();
	menu->setContentSize(ccp(230, 40));

	CCSprite* moveUpSpr =
		CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
	CCSprite* moveDownSpr =
		CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
	moveUpSpr->setFlipX(true);

	CCNode* moveUpBtn;
	CCNode* moveDownBtn;
	if (moveUpCallback != nullptr) {
		CCMenuItemSpriteExtra* button =
			CCMenuItemExt::createSpriteExtra(moveUpSpr, moveUpCallback);
		button->m_baseScale = .3;
		moveUpBtn = button;
	} else {
		moveUpSpr->setColor(ccc3(90, 90, 90));
		moveUpSpr->setOpacity(200);
		moveUpBtn = moveUpSpr;
	}
	if (moveDownCallback != nullptr) {
		CCMenuItemSpriteExtra* button =
			CCMenuItemExt::createSpriteExtra(moveDownSpr, moveDownCallback);
		button->m_baseScale = .3;
		moveDownBtn = button;
	} else {
		moveDownSpr->setColor(ccc3(90, 90, 90));
		moveDownSpr->setOpacity(200);
		moveDownBtn = moveDownSpr;
	}
	moveUpBtn->setScale(.3);
	moveDownBtn->setScale(.3);
	moveUpBtn->setRotation(90);
	moveDownBtn->setRotation(90);

	std::string progressString;
	if (playLayer->m_level->isPlatformer()) {
		int time = checkpoint->m_time;

		progressString = fmt::format("{}s", time % 60);

		if (time >= 60) {
			progressString =
				fmt::format("{}m", (time % 3600) / 60) + progressString;

			if (time >= 3600)
				progressString = fmt::format("{}h", time / 3600) + progressString;
		}
	} else {
		int decimals =
			Mod::get()->getSettingValue<int64_t>("percentage-display-decimals");
		progressString =
			fmt::format("{:.{}f}%", (float)checkpoint->m_percent, decimals);
	}

	CCSprite* checkpointSprite = CCSprite::createWithSpriteFrame(
		checkpoint->m_checkpoint->m_physicalCheckpointObject->displayFrame()
	);
	CCMenuItemSpriteExtra* selectBtn =
		CCMenuItemExt::createSpriteExtra(checkpointSprite, selectCallback);

	CCLabelBMFont* label =
		CCLabelBMFont::create(progressString.c_str(), "goldFont.fnt");
	label->setAnchorPoint(ccp(0, .5));
	label->setScale(.65);

	CCSprite* removeSprite =
		CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
	CCMenuItemSpriteExtra* removeBtn =
		CCMenuItemExt::createSpriteExtra(removeSprite, removeCallback);
	removeBtn->m_baseScale = .75;
	removeBtn->setScale(.75);

	menu->addChildAtPosition(moveUpBtn, geode::Anchor::Left, ccp(15, 10));
	menu->addChildAtPosition(moveDownBtn, geode::Anchor::Left, ccp(15, -10));
	menu->addChildAtPosition(selectBtn, geode::Anchor::Left, ccp(35, 0));
	menu->addChildAtPosition(label, geode::Anchor::Left, ccp(50, 0));
	menu->addChildAtPosition(removeBtn, geode::Anchor::Right, ccp(-20, 0));

#if defined(PA_DEBUG) && defined(PA_DESCRIBE)
	CCSprite* describeSprite =
		CCSprite::createWithSpriteFrameName("GJ_pasteBtn_001.png");
	CCMenuItemSpriteExtra* describeBtn = CCMenuItemExt::createSpriteExtra(
		describeSprite,
		[checkpoint, selectBtn](CCMenuItemSpriteExtra* sender) {
			selectBtn->activate();
			checkpoint->describe();
		}
	);
	describeBtn->m_baseScale = .75;
	describeBtn->setScale(.75);
	menu->addChildAtPosition(describeBtn, geode::Anchor::Right, ccp(-55, 0));
#endif

	return menu;
}
