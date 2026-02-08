#include "PlayLayer.hpp"
#include "sabe.persistenceapi/include/util/Stream.hpp"
#include <filesystem>
#include <optional>
#include <variant>

const char SAVE_HEADER[] = "PCP SAVE FILE";
#define CURRENT_VERSION 3

void ModPlayLayer::serializeCheckpoints() {
	if (m_fields->m_loadError != LoadError::None)
		return;

	unsigned int checkpointCount =
		m_fields->m_persistentCheckpointArray->count();

	if (checkpointCount == 0) {
		removeCurrentSaveLayer();
		return;
	}

	char platform = PLATFORM;
	unsigned int version = CURRENT_VERSION;
	std::string gameVersion = geode::Loader::get()->getGameVersion();

	persistenceAPI::Stream stream;
	stream.setFile(string::pathToString(getSavePath()), 2, true);

	stream.write((char*)SAVE_HEADER, sizeof(SAVE_HEADER));
	stream << version;
	stream << gameVersion;
	stream << platform;

	if (m_level->m_levelType != GJLevelType::Editor)
		stream << m_level->m_levelVersion;
	else {
		if (!m_fields->m_levelStringHash.has_value())
			m_fields->m_levelStringHash = c_stringHasher(m_level->m_levelString);

		stream << m_fields->m_levelStringHash.value();
	}

	stream << checkpointCount;

	for (PersistentCheckpoint* checkpoint : CCArrayExt<PersistentCheckpoint*>(
			  m_fields->m_persistentCheckpointArray
		  )) {
		checkpoint->serialize(stream);
	}

	stream.end();
}

void ModPlayLayer::deserializeCheckpoints(bool ignoreVerification) {
	unloadPersistentCheckpoints();
	m_fields->m_loadError = LoadError::None;

	std::string savePath = string::pathToString(getSavePath());
	if (!std::filesystem::exists(savePath))
		return;

	persistenceAPI::Stream stream;
	stream.setFile(savePath, 2);

	unsigned int saveVersion;
	std::variant<unsigned int, LoadError> verificationResult =
		verifySaveStream(stream);

	if (!ignoreVerification) {
		if (std::holds_alternative<LoadError>(verificationResult)) {
			stream.end();

			m_fields->m_loadError = std::get<LoadError>(verificationResult);

			return;
		}

		saveVersion = std::get<unsigned int>(verificationResult);
	} else {
		saveVersion = CURRENT_VERSION;
	}

	removeAllCheckpoints();

	unsigned int checkpointCount;
	stream >> checkpointCount;

	for (unsigned int i = checkpointCount; i > 0; i--) {
		PersistentCheckpoint* checkpoint = PersistentCheckpoint::create();

		// try {
		checkpoint->deserialize(stream, saveVersion);
		// } catch (...) { // TODO maybe implement exception logging
		// 	unloadPersistentCheckpoints();
		// 	log::error("Exception thrown while loading checkpoint");

		// 	stream.end();

		// 	m_fields->m_loadError = LoadError::Crash;
		// 	return;
		// }
		checkpoint->setupPhysicalObject();

		storePersistentCheckpoint(checkpoint);
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

	m_fields->m_persistentCheckpointArray->removeAllObjects();
}

std::variant<unsigned int, LoadError>
ModPlayLayer::verifySaveStream(persistenceAPI::Stream& stream) {
	bool isEditorLevel = m_level->m_levelType == GJLevelType::Editor;

	unsigned int saveVersion;
	std::string gameVersion;
	char savedPlatform;
	unsigned int levelVersion;
	size_t levelStringHash;

	stream.ignore(sizeof(SAVE_HEADER));
	stream >> saveVersion;
	stream >> gameVersion;
	stream >> savedPlatform;
	if (!isEditorLevel) {
		stream >> levelVersion;
	} else {
		stream >> levelStringHash;
	}

	if (saveVersion < 3)
		return LoadError::OutdatedData;

	if (gameVersion != geode::Loader::get()->getGameVersion())
		return LoadError::OutdatedData;

	if (saveVersion > CURRENT_VERSION)
		return LoadError::NewData;

	if (savedPlatform != PLATFORM)
		return LoadError::OtherPlatform;

	if (!isEditorLevel) {
		if (levelVersion != m_level->m_levelVersion)
			return LoadError::LevelVersionMismatch;
	} else {
		if (!m_fields->m_levelStringHash.has_value())
			m_fields->m_levelStringHash = c_stringHasher(m_level->m_levelString);
		if (levelStringHash != m_fields->m_levelStringHash.value()) {
			// log::debug(
			// 	"Bad Level Hash: {} != {}", levelStringHash,
			// 	m_fields->m_levelStringHash.value()
			// );
			return LoadError::LevelVersionMismatch;
		}
	}

	return saveVersion;
}

std::variant<unsigned int, LoadError>
ModPlayLayer::verifySavePath(std::filesystem::path path) {
	if (!std::filesystem::exists(path))
		return LoadError::None;

	persistenceAPI::Stream stream;
	stream.setFile(string::pathToString(path), 2);

	std::variant<unsigned int, LoadError> result = verifySaveStream(stream);
	stream.end();

	return result;
}

std::filesystem::path ModPlayLayer::getSavePath() {
	std::string savePath = string::pathToString(Mod::get()->getSaveDir());
	switch (m_level->m_levelType) {
	case GJLevelType::Editor: {
		std::string cleanLevelName = m_level->m_levelName;
		cleanLevelName.erase(
			std::remove(cleanLevelName.begin(), cleanLevelName.end(), '.'),
			cleanLevelName.end()
		);
		cleanLevelName.erase(
			std::remove(cleanLevelName.begin(), cleanLevelName.end(), '/'),
			cleanLevelName.end()
		);
		cleanLevelName.erase(
			std::remove(cleanLevelName.begin(), cleanLevelName.end(), '\\'),
			cleanLevelName.end()
		);
		savePath.append(
			fmt::format(
				"/saves/editor/{}-rev{}", cleanLevelName.c_str(),
				m_level->m_levelRev
			)
		);
	} break;
	default:
		savePath.append(
			fmt::format("/saves/main/{}", m_level->m_levelID.value())
		);
		break;
	}

	if (m_lowDetailMode)
		savePath.append("-lowDetail");

	savePath.append(fmt::format("_{}.pcp", m_fields->m_activeSaveLayer));

	return savePath;
}
