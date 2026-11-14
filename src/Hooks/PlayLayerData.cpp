#include "PlayLayer.hpp"
// #include <cvolton.level-id-api/include/EditorIDs.hpp>

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
	std::string gameVersion = geode::Loader::get()->getGameVersion();

	OutStream stream(getSavePath().string());

	char header[] = SAVE_HEADER;
	stream.write(header, sizeof(header));
	stream << saveVersion;
	stream << platform;
	stream << gameVersion;
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

	geode::log::debug("0");

	std::string savePath = getSavePath().string();
	if (!std::filesystem::exists(savePath))
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

	geode::log::debug("1");

	if (
		header.loadError != LoadError::None &&
		!SaveParser::isErrorFallbackCapable(header.loadError)
	)
		return;

	for (unsigned int i = header.checkpointCount; i > 0; i--) {
		PersistentCheckpoint* checkpoint = PersistentCheckpoint::create();

		checkpoint->deserializeExternal(stream, header);

		checkpoint->setupPhysicalObject();
		storePersistentCheckpoint(checkpoint, false);
	}

	log::debug("C: {}", m_fields->m_persistentCheckpointArray->count());

	geode::log::debug("2");

	if (header.loadError != LoadError::None) {
		if (SaveParser::isErrorFallbackCapable(header.loadError))
			for (PersistentCheckpoint* checkpoint :
				  CCArrayExt<PersistentCheckpoint*>(
					  m_fields->m_persistentCheckpointArray
				  ))
				checkpoint->m_isFallback = true;

		return;
	}

	for (PersistentCheckpoint* checkpoint : CCArrayExt<PersistentCheckpoint*>(
			  m_fields->m_persistentCheckpointArray
		  )) {
		// try {
		checkpoint->deserialize(stream, header, this);
		// } catch (...) { // TODO maybe implement exception logging
		// 	unloadPersistentCheckpoints();
		// 	log::error("Exception thrown while loading checkpoint");

		// 	stream.end();

		// 	m_fields->m_loadError = LoadError::Crash;
		// 	return;
		// }
	}

	geode::log::debug("3");
}

void ModPlayLayer::unloadPersistentCheckpoints() {
	switchGhostCheckpoint(0);

	bool isFallback = isFallbackCheckpoint();

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

std::filesystem::path ModPlayLayer::getSavePath() {
	return getSavePath(m_level, m_lowDetailMode, m_fields->m_activeSaveLayer);
}

std::filesystem::path ModPlayLayer::getSavePath(
	GJGameLevel* level, bool lowDetail, unsigned int saveLayer
) {
	std::string savePath =
		fmt::format("{}/saves/", Mod::get()->getSaveDir().string());

	if (level->m_levelType == GJLevelType::Editor)
		savePath.append(
			fmt::format("editor/{}", 0)
		); // EditorIDs::getID(level)));
	else
		savePath.append(fmt::to_string(level->m_levelID.value()));

	if (lowDetail)
		savePath.append("-lowDetail");

	savePath.append(fmt::format("_{}.pcp", saveLayer));

	return savePath;
}
