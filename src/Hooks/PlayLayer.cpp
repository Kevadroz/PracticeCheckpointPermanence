#include "PlayLayer.hpp"
#include "UILayer.hpp"

$execute {
	SettingChangedEvent(Mod::get(), "progressbar-checkpoint-opacity")
		.listen(+[](std::shared_ptr<SettingV3> setting) {
			ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());
			if (playLayer != nullptr)
				playLayer->m_fields->m_pbCheckpointContainer->setOpacity(
					255 * typeinfo_pointer_cast<FloatSettingV3>(setting)->getValue()
				);
		})
		.leak();

	SettingChangedEvent(Mod::get(), "switch-in-out-normal-mode")
		.listen(+[](std::shared_ptr<SettingV3> setting) {
			ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());
			if (playLayer != nullptr && !playLayer->m_isPracticeMode) {
				if (playLayer->m_fields->m_persistentCheckpointArray == nullptr)
					return;

				playLayer->m_fields->m_activeSaveLayer = 0;
				playLayer->unloadPersistentCheckpoints();
				if (typeinfo_pointer_cast<BoolSettingV3>(setting)->getValue()) {
					playLayer->updateSaveLayerCount();
					playLayer->deserializeCheckpoints();
				}
				playLayer->updateModUI();
			}
		})
		.leak();

	SettingChangedEvent(Mod::get(), "show-checkpoints-in-normal-mode")
		.listen(+[](std::shared_ptr<SettingV3> setting) {
			ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());
			if (playLayer != nullptr && !playLayer->m_isPracticeMode &&
				 Mod::get()->getSettingValue<bool>("switch-in-out-normal-mode"))
				playLayer->updateModUI();
		})
		.leak();
}

bool ModPlayLayer::init(
	GJGameLevel* level, bool useReplay, bool dontCreateObjects
) {
	if (!PlayLayer::init(level, useReplay, dontCreateObjects))
		return false;

	if (m_fields->m_persistentCheckpointArray != nullptr &&
		 isPersistentSystemActive() &&
		 !m_fields->m_hasAttemptedToLoadCheckpoints) {
		m_fields->m_hasAttemptedToLoadCheckpoints = true;

		updateSaveLayerCount();
		deserializeCheckpoints();

		updateModUI();
	}

	return true;
}

void ModPlayLayer::setupHasCompleted() {
	PlayLayer::setupHasCompleted();

	if (m_fields->m_persistentCheckpointArray == nullptr) {
		m_fields->m_persistentCheckpointArray = CCArray::create();

		m_fields->m_persistentCheckpointBatchNode =
			// @geode-ignore(unknown-resource)
			CCSpriteBatchNode::create("MainSheet.png"_spr);
		m_fields->m_persistentCheckpointBatchNode->setZOrder(219);
		m_objectLayer->addChild(m_fields->m_persistentCheckpointBatchNode);

		registerKeybindListeners();
	} else
		return;

	if (m_progressBar == nullptr)
		return;

	m_fields->m_pbCheckpointContainer = CCNodeRGBA::create();
	m_fields->m_pbCheckpointContainer->setPosition(
		ccp(0.f, m_progressBar->getContentHeight() / 2.f)
	);
	m_fields->m_pbCheckpointContainer->setCascadeOpacityEnabled(true);
	m_fields->m_pbCheckpointContainer->setOpacity(
		255 *
		Mod::get()->getSettingValue<double>("progressbar-checkpoint-opacity")
	);
	m_fields->m_pbCheckpointContainer->setID("checkpoint_container"_spr);
	m_progressBar->addChild(m_fields->m_pbCheckpointContainer);

	if (isPersistentSystemActive() &&
		 !m_fields->m_hasAttemptedToLoadCheckpoints) {
		m_fields->m_hasAttemptedToLoadCheckpoints = true;

		updateSaveLayerCount();
		deserializeCheckpoints();
	}

	updateModUI();
}

void ModPlayLayer::destructor() {
	unloadPersistentCheckpoints();
	PlayLayer::~PlayLayer();
}

// Copied from PlatformerSaves
void ModPlayLayer::processCreateObjectsFromSetup() {
	// if (!m_fields->m_startedLoadingObjects) {
	// 	m_fields->m_startedLoadingObjects = true;
	// 	*reinterpret_cast<int*>(geode::base::get() + UNIQUE_ID_OFFSET) = 12;
	// 	reinterpret_cast<persistenceAPI::PAPlayLayer*>(this)
	// 		->m_fields->m_uniqueIDBase =
	// 		*reinterpret_cast<int*>(geode::base::get() + UNIQUE_ID_OFFSET);
	// }
	PlayLayer::processCreateObjectsFromSetup();
}

void ModPlayLayer::resetLevel() {
	PersistentCheckpoint* checkpoint = nullptr;
	if (m_fields->m_loadError == LoadError::None) {
		if (isPersistentSystemActive() && m_isPracticeMode) {
			unsigned int loadIndex = 0;
			if (m_fields->m_ghostActiveCheckpoint > 0) {
				loadIndex = m_fields->m_ghostActiveCheckpoint;
				m_fields->m_ghostCheckpointUsedTime = m_attemptTime;
			} else if (m_checkpointArray->count() == 0)
				loadIndex = m_fields->m_activeCheckpoint;

			if (loadIndex != 0) {
				checkpoint = reinterpret_cast<PersistentCheckpoint*>(
					m_fields->m_persistentCheckpointArray->objectAtIndex(
						loadIndex - 1
					)
				);
				m_checkpointArray->addObject(checkpoint->m_checkpoint);
			}
		}
	}

	PlayLayer::resetLevel();

	if (checkpoint != nullptr)
		m_checkpointArray->removeObject(checkpoint->m_checkpoint);
}

void ModPlayLayer::loadFromCheckpoint(CheckpointObject* checkpoint) {
	PersistentCheckpoint* persistentCheckpoint = nullptr;
	for (PersistentCheckpoint* perCheckpoint : CCArrayExt<PersistentCheckpoint*>(
			  m_fields->m_persistentCheckpointArray
		  ))
		if (perCheckpoint->m_checkpoint == checkpoint) {
			persistentCheckpoint = perCheckpoint;
			break;
		}

	if (persistentCheckpoint != nullptr) {
		m_timePlayed = persistentCheckpoint->m_time;

		m_effectManager->m_persistentItemCountMap = {};
		m_effectManager->m_persistentTimerItemSet = {};
	}

	PlayLayer::loadFromCheckpoint(checkpoint);

	if (persistentCheckpoint != nullptr) {
		m_effectManager->m_persistentItemCountMap =
			persistentCheckpoint->m_persistentItemCountMap;
		m_effectManager->m_persistentTimerItemSet =
			persistentCheckpoint->m_persistentTimerItemSet;
	}
}

// Can't call switchCurrentCheckpoint here because it would cause infinite
// recursion in integrated normal mode.
void ModPlayLayer::togglePracticeMode(bool enabled) {
	PlayLayer::togglePracticeMode(enabled);

	if (m_fields->m_persistentCheckpointArray == nullptr)
		return;

	if (Mod::get()->getSettingValue<bool>("switch-in-out-normal-mode")) {
		if (!enabled && m_fields->m_activeCheckpoint > 0) {
			PersistentCheckpoint* checkpoint =
				reinterpret_cast<PersistentCheckpoint*>(
					m_fields->m_persistentCheckpointArray->objectAtIndex(
						m_fields->m_activeCheckpoint - 1
					)
				);
			if (checkpoint != nullptr) {
				checkpoint->toggleActive(false);
				m_currentCheckpoint = nullptr;
				setStartPosObject(nullptr);
				m_fields->m_activeCheckpoint = 0;
			}
			resetLevelFromStart();
		}
		updateModUI();

		return;
	}

	if (enabled) {
		updateSaveLayerCount();
		deserializeCheckpoints();
	} else
		unloadPersistentCheckpoints();

	updateModUI();
}

void ModPlayLayer::storeCheckpoint(CheckpointObject* p0) {
	PlayLayer::storeCheckpoint(p0);

	if (m_fields->m_ghostActiveCheckpoint > 0) {
		switchGhostCheckpoint(0);
		static_cast<ModUILayer*>(m_uiLayer)->updateSwitcher();
	}
}

void ModPlayLayer::registerKeybindListeners() {
	this->addEventListener(
		KeybindSettingPressedEventV3(Mod::get(), "keybind-create-checkpoint"),
		[this](Keybind const& keybind, bool down, bool repeat, double timestamp) {
			if (isPersistentSystemActive() && down) {
				if (!repeat)
					markPersistentCheckpoint();
				return true;
			}
			return false;
		}
	);

	this->addEventListener(
		KeybindSettingPressedEventV3(Mod::get(), "keybind-remove-checkpoint"),
		[this](Keybind const& keybind, bool down, bool repeat, double timestamp) {
			if (isPersistentSystemActive() && down) {
				if (!repeat) {
					if (m_fields->m_ghostActiveCheckpoint != 0)
						removeGhostPersistentCheckpoint();
					else if (m_fields->m_activeCheckpoint != 0)
						removeCurrentPersistentCheckpoint();
				}

				return true;
			}
			return false;
		}
	);

	this->addEventListener(
		KeybindSettingPressedEventV3(Mod::get(), "keybind-previous-checkpoint"),
		[this](Keybind const& keybind, bool down, bool repeat, double timestamp) {
			if (isPersistentSystemActive() && down) {
				previousCheckpoint();
				return true;
			}
			return false;
		}
	);

	this->addEventListener(
		KeybindSettingPressedEventV3(Mod::get(), "keybind-next-checkpoint"),
		[this](Keybind const& keybind, bool down, bool repeat, double timestamp) {
			if (isPersistentSystemActive() && down) {
				nextCheckpoint();
				return true;
			}
			return false;
		}
	);

	this->addEventListener(
		KeybindSettingPressedEventV3(Mod::get(), "keybind-previous-layer"),
		[this](Keybind const& keybind, bool down, bool repeat, double timestamp) {
			if (isPersistentSystemActive() && down) {
				previousSaveLayer();
				return true;
			}
			return false;
		}
	);

	this->addEventListener(
		KeybindSettingPressedEventV3(Mod::get(), "keybind-next-layer"),
		[this](Keybind const& keybind, bool down, bool repeat, double timestamp) {
			if (isPersistentSystemActive() && down) {
				nextSaveLayer();
				return true;
			}
			return false;
		}
	);
}

void ModPlayLayer::updateModUI() {
	static_cast<ModUILayer*>(m_uiLayer)->updateSwitcher();

	if (m_fields->m_pbCheckpointContainer == nullptr)
		return;

	if (m_isPlatformer)
		return;

	bool checkpointsVisible = isModUIVisible();

	CCNodeRGBA* container = m_fields->m_pbCheckpointContainer;
	container->setVisible(checkpointsVisible);
	container->removeAllChildren();

	m_fields->m_persistentCheckpointBatchNode->setVisible(checkpointsVisible);

	unsigned int currentCheckpoint = 0;
	float barWidth = m_progressBar->getContentWidth() - 4;
	for (PersistentCheckpoint* checkpoint : CCArrayExt<PersistentCheckpoint*>(
			  m_fields->m_persistentCheckpointArray
		  )) {
		GameObject* physicalObject =
			checkpoint->m_checkpoint->m_physicalCheckpointObject;
		CCSpriteFrame* frame = physicalObject->displayFrame();

		CCSprite* sprite = CCSprite::createWithSpriteFrame(frame);
		sprite->setScale(
			m_fields->m_activeCheckpoint == currentCheckpoint + 1 ? .5f : .4f
		);
		sprite->setOpacity(physicalObject->getOpacity());

		sprite->setPosition(
			ccp(barWidth * (checkpoint->m_percent / 100.f) + 2, 0)
		);
		if (m_fields->m_activeCheckpoint > 0 &&
			 checkpoint == m_fields->m_persistentCheckpointArray->objectAtIndex(
									m_fields->m_activeCheckpoint - 1
								))
			sprite->setZOrder(1);
		container->addChild(sprite);

		currentCheckpoint++;
	}

	// Update Cascade Opacity (Why is it protected?)
	container->setOpacity(container->getOpacity());
}

bool ModPlayLayer::isPersistentSystemActive() {
	return (Mod::get()->getSettingValue<bool>("switch-in-out-normal-mode") ||
			  m_isPracticeMode)
#ifndef PCP_DEBUG
			 && m_level->m_levelType != GJLevelType::Editor
#endif
		;
}

bool ModPlayLayer::isModUIVisible() {
#ifndef PCP_DEBUG
	if (m_level->m_levelType == GJLevelType::Editor)
		return false;
#endif

	if (m_isPracticeMode)
		return true;

	if (Mod::get()->getSettingValue<bool>("switch-in-out-normal-mode"))
		return Mod::get()->getSettingValue<bool>(
			"show-checkpoints-in-normal-mode"
		);

	return false;
}

bool ModPlayLayer::isInFallbackMode() {
	switch (m_fields->m_loadError) {
	case GameVersionMismatch:
	case LevelVersionMismatch:
	case Crash:
		return true;
	default:
		return false;
	}
}
