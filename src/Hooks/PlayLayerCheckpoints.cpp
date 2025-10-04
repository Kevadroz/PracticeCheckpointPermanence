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

	m_fields->m_activeCheckpoint = nextCheckpoint;

	updateUISwitcher();

	resetLevel();
}

void ModPlayLayer::markPersistentCheckpoint() {
	if (m_playerDied || m_levelEndAnimationStarted)
		return;

	PersistentCheckpoint* checkpoint =
		PersistentCheckpoint::createFromCheckpoint(
			createCheckpoint(), m_attempts, m_timePlayed, getCurrentPercent(),
			m_effectManager->m_persistentItemCountMap,
			m_effectManager->m_persistentTimerItemSet
		);
	storePersistentCheckpoint(checkpoint);
	serializeCheckpoints();

	if (m_fields->m_persistentCheckpointArray->count() == 1)
		updateSaveLayerCount();

	updateUISwitcher();
}

void ModPlayLayer::storePersistentCheckpoint(PersistentCheckpoint* checkpoint) {
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
}

void ModPlayLayer::removePersistentCheckpoint(
	PersistentCheckpoint* checkpoint
) {
	bool switchCheckpoint = m_fields->m_activeCheckpoint > 0 &&
									m_fields->m_persistentCheckpointArray->objectAtIndex(
										m_fields->m_activeCheckpoint - 1
									) == checkpoint;

	checkpoint->m_checkpoint->m_physicalCheckpointObject->removeFromParent();
	m_fields->m_persistentCheckpointArray->removeObject(checkpoint);

	if (switchCheckpoint)
		switchCurrentCheckpoint(m_fields->m_activeCheckpoint - 1, true);
	else
		updateUISwitcher();

	serializeCheckpoints();
}

void ModPlayLayer::removeCurrentPersistentCheckpoint() {
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

void ModPlayLayer::swapPersistentCheckpoints(
	unsigned int left, unsigned int right
) {
	m_fields->m_persistentCheckpointArray->exchangeObjectAtIndex(left, right);
	if (m_fields->m_activeCheckpoint == left + 1)
		m_fields->m_activeCheckpoint = right + 1;
	else if (m_fields->m_activeCheckpoint == right + 1)
		m_fields->m_activeCheckpoint = left + 1;

	serializeCheckpoints();
	updateUISwitcher();
}
