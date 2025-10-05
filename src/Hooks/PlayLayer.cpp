#include "PlayLayer.hpp"
#include "UILayer.hpp"

bool ModPlayLayer::init(
	GJGameLevel* level, bool useReplay, bool dontCreateObjects
) {
	if (!PlayLayer::init(level, useReplay, dontCreateObjects))
		return false;

	m_fields->m_persistentCheckpointArray = CCArray::create();
	CC_SAFE_RETAIN(m_fields->m_persistentCheckpointArray);

	m_fields->m_persistentCheckpointBatchNode =
		// @geode-ignore(unknown-resource)
		CCSpriteBatchNode::create("MainSheet.png"_spr);
	CC_SAFE_RETAIN(m_fields->m_persistentCheckpointBatchNode);
	m_fields->m_persistentCheckpointBatchNode->setZOrder(219);
	m_objectLayer->addChild(m_fields->m_persistentCheckpointBatchNode);

	registerKeybindListeners();

	if (m_isPracticeMode) {
		updateSaveLayerCount();
		deserializeCheckpoints();
	}

	return true;
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

	// union {
	// 	float f;
	// 	uint32_t u;
	// } percent = {.f = getCurrentPercent()};
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
	} else {
		unloadPersistentCheckpoints();
		updateUISwitcher();
	}
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

void ModPlayLayer::updateUISwitcher() {
	static_cast<ModUILayer*>(m_uiLayer)->updateSwitcher();
}
