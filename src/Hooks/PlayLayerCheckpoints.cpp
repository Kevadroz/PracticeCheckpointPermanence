#include "PlayLayer.hpp"

void ModPlayLayer::nextCheckpoint() {
	unsigned int nextCheckpoint = m_fields->m_activeCheckpoint + 1;
	if (nextCheckpoint > m_fields->m_persistentCheckpointArray->count())
		nextCheckpoint = 0;
	switchCurrentCheckpoint(nextCheckpoint);
}

void ModPlayLayer::previousCheckpoint() {
	unsigned int nextCheckpoint = m_fields->m_activeCheckpoint - 1;
	if (m_fields->m_activeCheckpoint == 0)
		nextCheckpoint = m_fields->m_persistentCheckpointArray->count();
	switchCurrentCheckpoint(nextCheckpoint);
}

// This may need a rewrite.
void ModPlayLayer::switchCurrentCheckpoint(
	unsigned int nextIndex, bool ignoreLastCheckpoint, bool noVisualUpdates
) {
	if (!isPersistentSystemActive())
		return;

	unsigned int checkpointCount =
		m_fields->m_persistentCheckpointArray->count();

	bool lastExists = 0 < m_fields->m_activeCheckpoint &&
							m_fields->m_activeCheckpoint <= checkpointCount;
	bool nextExists = 0 < nextIndex && nextIndex <= checkpointCount;
	PersistentCheckpoint* lastCheckpoint;
	if (lastExists)
		lastCheckpoint = reinterpret_cast<PersistentCheckpoint*>(
			(m_fields->m_persistentCheckpointArray)
				->objectAtIndex(m_fields->m_activeCheckpoint - 1)
		);
	PersistentCheckpoint* nextCheckpoint;
	if (nextExists)
		nextCheckpoint = reinterpret_cast<PersistentCheckpoint*>(
			(m_fields->m_persistentCheckpointArray)->objectAtIndex(nextIndex - 1)
		);

	if (m_levelEndAnimationStarted) {
		CCNode* endLayer = getChildByID("EndLevelLayer");
		if (endLayer == nullptr)
			return;
		endLayer->removeFromParentAndCleanup(true);
		if (!GameManager::get()->getGameVariable("0024"))
			PlatformToolbox::hideCursor();
	}

	removeAllCheckpoints();

	if (m_fields->m_activeCheckpoint == nextIndex)
		return;

	if (!ignoreLastCheckpoint && lastExists)
		lastCheckpoint->toggleActive(false);

	if (!noVisualUpdates && nextExists) {
		nextCheckpoint->toggleActive(true);
	}

	m_fields->m_activeCheckpoint = nextIndex;
	switchGhostCheckpoint(0, false, false);

	if (lastExists && lastCheckpoint->m_isDegraded)
		setActiveStartPosObject(nullptr);

	if (nextExists && nextCheckpoint->m_isDegraded)
		setActiveStartPosObject(nextCheckpoint->m_fallbackData.startPos);

	if (
		Mod::get()->getSettingValue<bool>("switch-in-out-normal-mode") &&
		(nextIndex != 0) != m_isPracticeMode
	) {
		PlayLayer::togglePracticeMode(!m_isPracticeMode);
		if (!m_isPracticeMode)
			return;
	}

	if (
		!noVisualUpdates && Mod::get()->getSettingValue<bool>("reset-attempts")
	) {
		m_attempts = 0;
		m_clicks = 0;
		m_jumps = 0;
	} else
		m_attempts--;

	m_level->setAttempts(m_level->m_attempts - 1);

	if (!noVisualUpdates)
		updateModUI();

	PlayLayer::resetLevel();
	if (!noVisualUpdates && nextExists && nextCheckpoint->m_isDegraded)
		startMusic();
}

void ModPlayLayer::switchGhostCheckpoint(
	unsigned int nextCheckpoint, bool noVisualUpdates, bool updateUI
) {
	if (
		!noVisualUpdates && m_fields->m_ghostActiveCheckpoint > 0 &&
		(m_fields->m_ghostActiveCheckpoint != m_fields->m_activeCheckpoint)
	)
		static_cast<PersistentCheckpoint*>(
			m_fields->m_persistentCheckpointArray->objectAtIndex(
				m_fields->m_ghostActiveCheckpoint - 1
			)
		)
			->toggleActive(false, true);

	m_fields->m_ghostActiveCheckpoint = nextCheckpoint;

	if (!noVisualUpdates && nextCheckpoint > 0)
		static_cast<PersistentCheckpoint*>(m_fields->m_persistentCheckpointArray
														  ->objectAtIndex(nextCheckpoint - 1))
			->toggleActive(true, true);

	if (updateUI)
		updateModUI();
}

void ModPlayLayer::markPersistentCheckpoint() {
	if (m_playerDied || m_levelEndAnimationStarted || !m_isPracticeMode)
		return;

	if (isUnloadableError()) {
		updateModUI();
		return;
	} else if (isFallbackMode()) {
		m_fields->m_loadError = LoadError::None;
	}

	PersistentCheckpoint* checkpoint = PersistentCheckpoint::create();
	checkpoint->storeData(createCheckpoint(), this);

	if (Mod::get()->getSettingValue<bool>("color-placed-checkpoints")) {
		std::vector<float> points;
		for (unsigned int i = 0;
			  i < m_fields->m_persistentCheckpointArray->count(); ++i) {
			float color =
				reinterpret_cast<PersistentCheckpoint*>(
					m_fields->m_persistentCheckpointArray->objectAtIndex(i)
				)
					->m_color;
			if (!std::isnan(color))
				points.push_back(color);
		}

		if (points.size() > 0) {
			std::sort(points.begin(), points.end());
			points.push_back(points[0] + 360.f);

			float start, size = -1;
			for (unsigned int i = 0; i < points.size() - 1; ++i) {
				float cSize = points[i + 1] - points[i];
				if (cSize > size) {
					start = points[i];
					size = cSize;
				}
			}

			float min = start + size * .3f;
			float range = size * .4f;

			float color = min + rand() / (float)RAND_MAX * range;
			if (color > 360.f)
				color -= 360.f;

			checkpoint->m_color = color;
		} else {
			checkpoint->m_color = rand() / (float)RAND_MAX * 360.f;
		}
		checkpoint->refreshStyle(false);
	}

	unsigned int newCheckpointIndex =
		storePersistentCheckpoint(checkpoint, true) + 1;
	serializeCheckpoints();

	if (m_isPracticeMode) {
		switchGhostCheckpoint(newCheckpointIndex);
		if (
			Mod::get()->getSettingValue<double>(
				"ghost-auto-checkpoint-inactive-time"
			) > 0.0
		)
			m_fields->m_ghostCheckpointUsedTime = m_attemptTime;
	}

	if (m_fields->m_persistentCheckpointArray->count() == 1)
		updateSaveLayerCount();

	updateModUI();
}

unsigned int ModPlayLayer::storePersistentCheckpoint(
	PersistentCheckpoint* checkpoint, bool reorder
) {
	CCArray* array = m_fields->m_persistentCheckpointArray;

	unsigned int index = 0;
	if (reorder && array->count() > 0)
		for (PersistentCheckpoint* arrayCheckpoint :
			  CCArrayExt<PersistentCheckpoint*>(array)) {
			if (
				m_isPlatformer ? arrayCheckpoint->m_time > checkpoint->m_time
									: arrayCheckpoint->m_percent > checkpoint->m_percent
			)
				break;
			index++;
		}

	m_fields->m_persistentCheckpointBatchNode->addChild(
		checkpoint->m_physicalObject
	);
	if (reorder && index < array->count())
		array->insertObject(checkpoint, index);
	else
		array->addObject(checkpoint);

	return index;
}

void ModPlayLayer::removePersistentCheckpoint(
	PersistentCheckpoint* checkpoint
) {
	if (isUnloadableError()) {
		updateModUI();
		return;
	}

	assert(m_fields->m_persistentCheckpointArray->containsObject(checkpoint));

	unsigned int removeIndex =
		m_fields->m_persistentCheckpointArray->indexOfObject(checkpoint);

	bool updateActiveCheckpoint =
		m_fields->m_activeCheckpoint > 0 &
		removeIndex <= m_fields->m_activeCheckpoint - 1;
	bool switchCheckpoint =
		m_fields->m_activeCheckpoint > 0 && updateActiveCheckpoint;

	checkpoint->m_physicalObject->removeFromParent();
	m_fields->m_persistentCheckpointArray->removeObjectAtIndex(removeIndex);

	if (removeIndex + 1 == m_fields->m_ghostActiveCheckpoint)
		switchGhostCheckpoint(0, true);
	else if (m_fields->m_ghostActiveCheckpoint > 0)
		switchGhostCheckpoint(m_fields->m_ghostActiveCheckpoint - 1, true);

	if (switchCheckpoint)
		switchCurrentCheckpoint(m_fields->m_activeCheckpoint - 1, true);
	else {
		if (updateActiveCheckpoint)
			m_fields->m_activeCheckpoint--;

		updateModUI();
	}

	serializeCheckpoints();
}

void ModPlayLayer::removeCurrentPersistentCheckpoint() {
	if (isUnloadableError()) {
		updateModUI();
		return;
	}

	if (m_fields->m_activeCheckpoint > 0) {
		PersistentCheckpoint* checkpoint =
			reinterpret_cast<PersistentCheckpoint*>(
				m_fields->m_persistentCheckpointArray->objectAtIndex(
					m_fields->m_activeCheckpoint - 1
				)
			);
		removePersistentCheckpoint(checkpoint);
	}
}

void ModPlayLayer::removeGhostPersistentCheckpoint() {
	if (isUnloadableError())
		return;

	if (m_fields->m_ghostActiveCheckpoint > 0) {
		PersistentCheckpoint* checkpoint =
			reinterpret_cast<PersistentCheckpoint*>(
				m_fields->m_persistentCheckpointArray->objectAtIndex(
					m_fields->m_ghostActiveCheckpoint - 1
				)
			);
		removePersistentCheckpoint(checkpoint);
	}
}

void ModPlayLayer::swapPersistentCheckpoints(
	unsigned int left, unsigned int right
) {
	m_fields->m_persistentCheckpointArray->exchangeObjectAtIndex(left, right);

	if (m_fields->m_activeCheckpoint == left + 1)
		m_fields->m_activeCheckpoint = right + 1;
	else if (m_fields->m_activeCheckpoint == right + 1)
		m_fields->m_activeCheckpoint = left + 1;

	if (m_fields->m_ghostActiveCheckpoint == left + 1)
		switchGhostCheckpoint(right + 1, true);
	else if (m_fields->m_ghostActiveCheckpoint == right + 1)
		switchGhostCheckpoint(left + 1, true);

	serializeCheckpoints();
	updateModUI();
}
