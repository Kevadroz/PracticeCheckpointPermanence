#include "PlayLayer.hpp"
#include "UILayer.hpp"

$execute {
	new EventListener<SettingChangedFilterV3>(
		+[](std::shared_ptr<SettingV3> setting) {
			ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());
			if (playLayer != nullptr)
				playLayer->m_fields->m_pbCheckpointContainer->setOpacity(
					255 * typeinfo_pointer_cast<FloatSettingV3>(setting)->getValue()
				);
		},
		SettingChangedFilterV3(Mod::get(), "progressbar-checkpoint-opacity")
	);
}

bool ModPlayLayer::init(
	GJGameLevel* level, bool useReplay, bool dontCreateObjects
) {
	if (!PlayLayer::init(level, useReplay, dontCreateObjects))
		return false;

	if (m_fields->m_persistentCheckpointArray != nullptr && m_isPracticeMode &&
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
		CC_SAFE_RETAIN(m_fields->m_persistentCheckpointArray);

		m_fields->m_persistentCheckpointBatchNode =
			// @geode-ignore(unknown-resource)
			CCSpriteBatchNode::create("MainSheet.png"_spr);
		CC_SAFE_RETAIN(m_fields->m_persistentCheckpointBatchNode);
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

	if (m_isPracticeMode && !m_fields->m_hasAttemptedToLoadCheckpoints) {
		m_fields->m_hasAttemptedToLoadCheckpoints = true;

		updateSaveLayerCount();
		deserializeCheckpoints();
	}

	updateModUI();
}

void ModPlayLayer::destructor() {
	PlayLayer::~PlayLayer();
	CC_SAFE_RELEASE(m_fields->m_persistentCheckpointArray);
	CC_SAFE_RELEASE(m_fields->m_persistentCheckpointBatchNode);
}

// Copied from PlatformerSaves
void ModPlayLayer::processCreateObjectsFromSetup() {
	if (!m_fields->m_startedLoadingObjects) {
		m_fields->m_startedLoadingObjects = true;
		*reinterpret_cast<int*>(geode::base::get() + UNIQUE_ID_OFFSET) = 12;
		reinterpret_cast<persistenceAPI::PAPlayLayer*>(this)
			->m_fields->m_uniqueIDBase =
			*reinterpret_cast<int*>(geode::base::get() + UNIQUE_ID_OFFSET);
	}
	PlayLayer::processCreateObjectsFromSetup();
}

void ModPlayLayer::resetLevel() {
	PersistentCheckpoint* checkpoint = nullptr;
	if (m_isPracticeMode && m_fields->m_activeCheckpoint != 0 &&
		 m_checkpointArray->count() == 0) {
		checkpoint = reinterpret_cast<PersistentCheckpoint*>(
			m_fields->m_persistentCheckpointArray->objectAtIndex(
				m_fields->m_activeCheckpoint - 1
			)
		);
		m_checkpointArray->addObject(checkpoint->m_checkpoint);
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
		m_attempts = persistentCheckpoint->m_attempts;

		m_effectManager->m_persistentItemCountMap =
			persistentCheckpoint->m_persistentItemCountMap;
		m_effectManager->m_persistentTimerItemSet =
			persistentCheckpoint->m_persistentTimerItemSet;
	}
}

void ModPlayLayer::togglePracticeMode(bool enabled) {
	PlayLayer::togglePracticeMode(enabled);

	if (m_fields->m_persistentCheckpointArray == nullptr)
		return;

	m_fields->m_activeSaveLayer = 0;

	if (enabled) {
		updateSaveLayerCount();
		deserializeCheckpoints();
	} else
		unloadPersistentCheckpoints();

	updateModUI();
}

void ModPlayLayer::registerKeybindListeners() {
#ifndef GEODE_IS_IOS
	this->template addEventListener<InvokeBindFilter>(
		[this](InvokeBindEvent* event) {
			if (m_isPracticeMode && event->isDown()) {
				markPersistentCheckpoint();
			}
			return ListenerResult::Propagate;
		},
		"create_checkpoint"_spr
	);

	this->template addEventListener<InvokeBindFilter>(
		[this](InvokeBindEvent* event) {
			if (m_isPracticeMode && event->isDown()) {
				if (m_fields->m_activeCheckpoint != 0) {
					removeCurrentPersistentCheckpoint();
				}
			}
			return ListenerResult::Propagate;
		},
		"remove_checkpoint"_spr
	);

	this->template addEventListener<InvokeBindFilter>(
		[this](InvokeBindEvent* event) {
			if (event->isDown()) {
				previousCheckpoint();
			}
			return ListenerResult::Propagate;
		},
		"previous_checkpoint"_spr
	);

	this->template addEventListener<InvokeBindFilter>(
		[this](InvokeBindEvent* event) {
			if (event->isDown()) {
				nextCheckpoint();
			}
			return ListenerResult::Propagate;
		},
		"next_checkpoint"_spr
	);

	this->template addEventListener<InvokeBindFilter>(
		[this](InvokeBindEvent* event) {
			if (event->isDown()) {
				previousSaveLayer();
			}
			return ListenerResult::Propagate;
		},
		"previous_layer"_spr
	);

	this->template addEventListener<InvokeBindFilter>(
		[this](InvokeBindEvent* event) {
			if (event->isDown()) {
				nextSaveLayer();
			}
			return ListenerResult::Propagate;
		},
		"next_layer"_spr
	);
#endif
}

void ModPlayLayer::updateModUI() {
	static_cast<ModUILayer*>(m_uiLayer)->updateSwitcher();

	if (m_fields->m_pbCheckpointContainer == nullptr)
		return;

	if (m_isPlatformer)
		return;

	CCNodeRGBA* container = m_fields->m_pbCheckpointContainer;
	container->setVisible(m_isPracticeMode);
	container->removeAllChildren();

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
		container->addChild(sprite);

		currentCheckpoint++;
	}

	// Update Cascade Opacity (Why is it protected?)
	container->setOpacity(container->getOpacity());
}
