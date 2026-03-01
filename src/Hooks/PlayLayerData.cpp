#include "PlayLayer.hpp"
#include "sabe.persistenceapi/include/util/Stream.hpp"

void ModPlayLayer::serializeCheckpoints() {
	if (m_fields->m_loadError != LoadError::None)
		return;

	unsigned int checkpointCount =
		m_fields->m_persistentCheckpointArray->count();

	if (checkpointCount == 0) {
		removeCurrentSaveLayer();
		return;
	}

#ifndef PCP_DEBUG
	if (m_level->m_levelType == GJLevelType::Editor)
		return;
#endif

	char platform = PLATFORM;
	unsigned int saveVersion = CURRENT_VERSION;
	gd::string gameVersion = geode::Loader::get()->getGameVersion();

	persistenceAPI::Stream stream;
	stream.setFile(string::pathToString(getSavePath()), 2, true);

	gd::string header = SAVE_HEADER;
	stream << header;
	stream << saveVersion;
	stream << gameVersion;
	stream << platform;
	stream << m_level->m_levelVersion;
	stream << m_level->m_levelName;
	stream << checkpointCount;

	for (PersistentCheckpoint* checkpoint : CCArrayExt<PersistentCheckpoint*>(
			  m_fields->m_persistentCheckpointArray
		  ))
		checkpoint->serializeExternal(stream);

	for (PersistentCheckpoint* checkpoint : CCArrayExt<PersistentCheckpoint*>(
			  m_fields->m_persistentCheckpointArray
		  ))
		checkpoint->serialize(stream);

	stream.end();
}

void ModPlayLayer::deserializeCheckpoints(bool ignoreVerification) {
#ifndef PCP_DEBUG
	if (m_level->m_levelType == GJLevelType::Editor)
		return;
#endif

	unloadPersistentCheckpoints();
	m_fields->m_loadError = LoadError::None;

	std::string savePath = string::pathToString(getSavePath());
	if (!std::filesystem::exists(savePath))
		return;

	persistenceAPI::Stream stream;
	stream.setFile(savePath, 2);

	SaveHeader header = SaveParser::fromStream(stream, m_level);

	// >DEBUG Force Fallback
	// header.loadError = LoadError::LevelVersionMismatch;

	removeAllCheckpoints();

	m_fields->m_loadError = header.loadError;

	if (header.loadError != None && !isInFallbackMode())
		return;

	for (unsigned int i = header.checkpointCount; i > 0; i--) {
		PersistentCheckpoint* checkpoint = PersistentCheckpoint::create();

		checkpoint->deserializeExternal(stream, header);

		checkpoint->setupPhysicalObject();
		storePersistentCheckpoint(checkpoint, false);
	}

	if (!ignoreVerification) {
		if (header.loadError != LoadError::None) {
			stream.end();

			for (PersistentCheckpoint* checkpoint :
				  CCArrayExt<PersistentCheckpoint*>(
					  m_fields->m_persistentCheckpointArray
				  ))

				return;
		}
	} else {
		header.loadError = LoadError::None;
	}

	for (PersistentCheckpoint* checkpoint : CCArrayExt<PersistentCheckpoint*>(
			  m_fields->m_persistentCheckpointArray
		  )) {
		// try {
		checkpoint->deserialize(stream, header);
		// } catch (...) { // TODO maybe implement exception logging
		// 	unloadPersistentCheckpoints();
		// 	log::error("Exception thrown while loading checkpoint");

		// 	stream.end();

		// 	m_fields->m_loadError = LoadError::Crash;
		// 	return;
		// }
	}

	stream.end();
}

void ModPlayLayer::unloadPersistentCheckpoints() {
	for (PersistentCheckpoint* checkpoint : CCArrayExt<PersistentCheckpoint*>(
			  m_fields->m_persistentCheckpointArray
		  )) {
		checkpoint->m_checkpoint->m_physicalCheckpointObject->removeFromParent();
	}
	m_fields->m_activeCheckpoint = 0;

	if (isInFallbackMode()) {
		m_currentCheckpoint = nullptr;
		setStartPosObject(nullptr);
	}

	m_fields->m_persistentCheckpointArray->removeAllObjects();
}

void ModPlayLayer::resave() {
	if (isInFallbackMode()) {
		for (unsigned int i = 0;
			  i < m_fields->m_persistentCheckpointArray->count(); ++i) {
			PersistentCheckpoint* checkpoint = static_cast<PersistentCheckpoint*>(
				m_fields->m_persistentCheckpointArray->objectAtIndex(i)
			);
			switchCurrentCheckpoint(i + 1);
			loadStartPosObject();
			checkpoint->storeData(m_currentCheckpoint, this);
		}
		m_fields->m_loadError = LoadError::None;
		serializeCheckpoints();

		switchCurrentCheckpoint(0);
	} else
		serializeCheckpoints();
}

std::filesystem::path ModPlayLayer::getSavePath() {
	return getSavePath(m_level, m_lowDetailMode, m_fields->m_activeSaveLayer);
}

std::filesystem::path ModPlayLayer::getSavePath(
	GJGameLevel* level, bool lowDetail, unsigned int saveLayer
) {
	std::string savePath = fmt::format(
		"{}/saves/{}", string::pathToString(Mod::get()->getSaveDir()),
		level->m_levelID.value()
	);

	if (lowDetail)
		savePath.append("-lowDetail");

#ifdef PCP_DEBUG
	if (level->m_levelType == GJLevelType::Editor)
		savePath = fmt::format(
			"{}/saves/editor", string::pathToString(Mod::get()->getSaveDir())
		);
#endif

	savePath.append(fmt::format("_{}.pcp", saveLayer));

	return savePath;
}
