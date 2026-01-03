#include "PlayLayer.hpp"

void ModPlayLayer::nextCheckpoint() {
	if (!m_isPracticeMode || m_levelEndAnimationStarted)
		return;

	unsigned int nextCheckpoint = m_fields->m_activeCheckpoint + 1;
	if (nextCheckpoint > m_fields->m_persistentCheckpointArray->count())
		nextCheckpoint = 0;
	switchCurrentCheckpoint(nextCheckpoint);
}

void ModPlayLayer::previousCheckpoint() {
	if (!m_isPracticeMode || m_levelEndAnimationStarted)
		return;

	unsigned int nextCheckpoint = m_fields->m_activeCheckpoint - 1;
	if (m_fields->m_activeCheckpoint == 0)
		nextCheckpoint = m_fields->m_persistentCheckpointArray->count();
	switchCurrentCheckpoint(nextCheckpoint);
}

void ModPlayLayer::switchCurrentCheckpoint(
	unsigned int nextCheckpoint, bool ignoreLastCheckpoint
) {
	removeAllCheckpoints();

	if (m_fields->m_activeCheckpoint == nextCheckpoint)
		return;

	if (!ignoreLastCheckpoint && m_fields->m_activeCheckpoint != 0)
		reinterpret_cast<PersistentCheckpoint*>(
			m_fields->m_persistentCheckpointArray->objectAtIndex(
				m_fields->m_activeCheckpoint - 1
			)
		)
			->toggleActive(false);

	if (nextCheckpoint != 0)
		reinterpret_cast<PersistentCheckpoint*>(
			(m_fields->m_persistentCheckpointArray)
				->objectAtIndex(nextCheckpoint - 1)
		)
			->toggleActive(true);

	m_fields->m_ghostActiveCheckpoint = 0;
	m_fields->m_activeCheckpoint = nextCheckpoint;

	if (Mod::get()->getSettingValue<bool>("reset-attempts"))
		m_attempts = 0;
	else
		m_attempts--;

	updateModUI();

	resetLevel();
}

void ModPlayLayer::markPersistentCheckpoint() {
	if (m_playerDied || m_levelEndAnimationStarted)
		return;

	if (m_fields->m_loadError != LoadError::None) {
		updateModUI();
		return;
	}

	PersistentCheckpoint* checkpoint =
		PersistentCheckpoint::createFromCheckpoint(
			createCheckpoint(), m_timePlayed, getCurrentPercent(),
			m_effectManager->m_persistentItemCountMap,
			m_effectManager->m_persistentTimerItemSet
		);
	m_fields->m_ghostActiveCheckpoint = storePersistentCheckpoint(checkpoint) + 1;
	serializeCheckpoints();

	if (m_fields->m_persistentCheckpointArray->count() == 1)
		updateSaveLayerCount();

	updateModUI();
}

unsigned int
ModPlayLayer::storePersistentCheckpoint(PersistentCheckpoint* checkpoint) {
	CCArray* array = m_fields->m_persistentCheckpointArray;

	unsigned int index = 0;
	if (array->count() > 0)
		for (PersistentCheckpoint* arrayCheckpoint :
			  CCArrayExt<PersistentCheckpoint*>(array)) {
			if (m_isPlatformer
					 ? arrayCheckpoint->m_time > checkpoint->m_time
					 : arrayCheckpoint->m_percent > checkpoint->m_percent)
				break;
			index++;
		}

	m_fields->m_persistentCheckpointBatchNode->addChild(
		checkpoint->m_checkpoint->m_physicalCheckpointObject
	);
	if (index < array->count())
		array->insertObject(checkpoint, index);
	else
		array->addObject(checkpoint);

	return index;
}

void ModPlayLayer::removePersistentCheckpoint(
	PersistentCheckpoint* checkpoint
) {
	if (m_fields->m_loadError != LoadError::None) {
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

	checkpoint->m_checkpoint->m_physicalCheckpointObject->removeFromParent();
	m_fields->m_persistentCheckpointArray->removeObjectAtIndex(removeIndex);

	if (removeIndex + 1 == m_fields->m_ghostActiveCheckpoint)
		m_fields->m_ghostActiveCheckpoint = 0;
	else if (m_fields->m_ghostActiveCheckpoint > 0)
		m_fields->m_ghostActiveCheckpoint--;

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
	if (m_fields->m_loadError != LoadError::None) {
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
	assert(m_fields->m_loadError == LoadError::None);

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
		m_fields->m_ghostActiveCheckpoint = right + 1;
	else if (m_fields->m_ghostActiveCheckpoint == right + 1)
		m_fields->m_ghostActiveCheckpoint = left + 1;

	serializeCheckpoints();
	updateModUI();
}
