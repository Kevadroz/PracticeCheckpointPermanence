#include "PlayLayer.hpp"
#include <cvolton.level-id-api/EditorIDs.hpp>

const std::hash<std::string> c_stringHasher;

void ModPlayLayer::serializeCheckpoints() {
	if (isUnloadableError())
		return;

	unsigned int checkpointCount =
		m_fields->m_persistentCheckpointArray->count();

	if (checkpointCount == 0) {
		removeCurrentSaveLayer();
		return;
	}

	char platform = PLATFORM;
	unsigned int saveVersion = CURRENT_VERSION;
	std::string gameVersion = Loader::get()->getGameVersion();

	OutStream stream(getSavePath().string());

	char header[] = SAVE_HEADER;
	stream.write(header, sizeof(header));
	stream << saveVersion;
	stream << gameVersion;
	stream << platform;
	if (m_level->m_levelType != GJLevelType::Editor)
		stream << m_level->m_levelVersion;
	else {
		if (!m_fields->m_levelStringHash.has_value())
			m_fields->m_levelStringHash = c_stringHasher(m_level->m_levelString);

		stream << m_fields->m_levelStringHash.value();
	}
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
}

void ModPlayLayer::deserializeCheckpoints(bool ignoreVerification) {
	unloadPersistentCheckpoints();
	m_fields->m_loadError = LoadError::None;

	asp::fs::Path savePath = getSavePath();
	if (!savePath.exists())
		return;

	removeAllCheckpoints();

	InStream stream(savePath);

	SaveHeader header =
		SaveParser::fromStream(stream, m_level, false, ignoreVerification);

	if (
		Mod::get()->getSettingValue<bool>("debug-force-fallback") &&
		!ignoreVerification
	)
		header.loadError = LoadError::ForcedFallback;

	m_fields->m_loadError = header.loadError;

	if (
		header.loadError != LoadError::None &&
		!SaveParser::isErrorFallbackCapable(header.loadError)
	)
		return;

	std::vector<PersistentCheckpoint*> checkpoints;

	for (unsigned int i = header.checkpointCount; i > 0; i--) {
		PersistentCheckpoint* checkpoint = PersistentCheckpoint::create();

		checkpoint->deserializeExternal(stream, header);

		checkpoints.push_back(checkpoint);
	}

	if (header.loadError != LoadError::None) {
		if (SaveParser::isErrorFallbackCapable(header.loadError))
			for (PersistentCheckpoint* checkpoint : checkpoints) {
				checkpoint->m_isDegraded = true;
				checkpoint->setupPhysicalObject();
				storePersistentCheckpoint(checkpoint, false);
			}

		return;
	}

	for (PersistentCheckpoint* checkpoint : checkpoints) {
		// try {
		checkpoint->deserialize(stream, header, this);
		// } catch (...) { // TODO maybe implement exception logging
		// 	unloadPersistentCheckpoints();
		// 	log::error("Exception thrown while loading checkpoint");

		// 	stream.end();

		// 	m_fields->m_loadError = LoadError::Crash;
		// 	return;
		// }

		checkpoint->setupPhysicalObject();
		storePersistentCheckpoint(checkpoint, false);
	}
}

void ModPlayLayer::unloadPersistentCheckpoints() {
	switchGhostCheckpoint(0);

	bool isFallback = isDegradedCheckpoint();

	for (PersistentCheckpoint* checkpoint : CCArrayExt<PersistentCheckpoint*>(
			  m_fields->m_persistentCheckpointArray
		  )) {
		checkpoint->m_physicalObject->removeFromParent();
	}
	m_fields->m_activeCheckpoint = 0;

	if (isFallback)
		setActiveStartPosObject(nullptr);

	m_fields->m_persistentCheckpointArray->removeAllObjects();
}

asp::fs::Path ModPlayLayer::getSavePath() {
	return getSavePath(m_level, m_lowDetailMode, m_fields->m_activeSaveLayer);
}

asp::fs::Path ModPlayLayer::getSavePath(
	GJGameLevel* level, bool lowDetail, unsigned int saveLayer
) {
	std::string savePath =
		fmt::format("{}/saves/", Mod::get()->getSaveDir().string());

	if (level->m_levelType == GJLevelType::Editor)
		savePath.append(fmt::format("editor/{}", EditorIDs::getID(level)));
	else
		savePath.append(fmt::to_string(level->m_levelID.value()));

	if (lowDetail)
		savePath.append("-lowDetail");

	savePath.append(fmt::format("_{}.pcp", saveLayer));

	return asp::fs::Path(savePath);
}
