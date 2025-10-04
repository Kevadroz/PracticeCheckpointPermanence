#include "PlayLayer.hpp"
#include "sabe.persistenceapi/include/util/Stream.hpp"
#include <Geode/Enums.hpp>
#include <filesystem>
#include <optional>

const char SAVE_HEADER[] = "PCP SAVE FILE";
const unsigned int CURRENT_VERSION = 1;

void ModPlayLayer::serializeCheckpoints() {
	unsigned int checkpointCount =
		m_fields->m_persistentCheckpointArray->count();

	if (checkpointCount == 0) {
		removeCurrentSaveLayer();
		return;
	}

	char platform = PLATFORM;
	unsigned int version = CURRENT_VERSION;

	persistenceAPI::Stream stream;
	stream.setFile(getSavePath().string(), 2, true);

	stream.write((char*)SAVE_HEADER, sizeof(SAVE_HEADER));
	stream << version;
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

void ModPlayLayer::deserializeCheckpoints() {
	unloadPersistentCheckpoints();

	std::string savePath = getSavePath().string();
	if (!std::filesystem::exists(savePath))
		return;

	persistenceAPI::Stream stream;
	stream.setFile(savePath, 2);

	std::optional<unsigned int> verificationResult = verifySaveStream(stream);
	if (!verificationResult.has_value())
		return;

	unsigned int saveVersion = verificationResult.value();

	removeAllCheckpoints();

	unsigned int checkpointCount;
	stream >> checkpointCount;

	for (unsigned int i = checkpointCount; i > 0; i--) {
		PersistentCheckpoint* checkpoint = PersistentCheckpoint::create();

		checkpoint->deserialize(stream, saveVersion);
		checkpoint->createPhysicalObject();

		storePersistentCheckpoint(checkpoint);
	}

	stream.end();

	updateUISwitcher();
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

std::optional<unsigned int>
ModPlayLayer::verifySaveStream(persistenceAPI::Stream& stream) {
	stream.ignore(sizeof(SAVE_HEADER));

	unsigned int saveVersion;
	stream >> saveVersion;

	if (saveVersion > CURRENT_VERSION)
		return std::nullopt;

	char savedPlatform;
	stream >> savedPlatform;

	if (savedPlatform != PLATFORM)
		return std::nullopt;

	if (m_level->m_levelType != GJLevelType::Editor) {
		unsigned int levelVersion;
		stream >> levelVersion;
		if (levelVersion != m_level->m_levelVersion)
			return std::nullopt;
	} else {
		size_t levelStringHash;
		stream >> levelStringHash;

		if (!m_fields->m_levelStringHash.has_value())
			m_fields->m_levelStringHash = c_stringHasher(m_level->m_levelString);
		if (levelStringHash != m_fields->m_levelStringHash.value()) {
			// log::debug(
			// 	"Bad Level Hash: {} != {}", levelStringHash,
			// 	m_fields->m_levelStringHash.value()
			// );
			return std::nullopt;
		}
	}

	return saveVersion;
}

std::optional<unsigned int>
ModPlayLayer::verifySavePath(std::filesystem::path path) {
	if (!std::filesystem::exists(path))
		return std::nullopt;

	persistenceAPI::Stream stream;
	stream.setFile(path.string(), 2);

	std::optional<unsigned int> result = verifySaveStream(stream);
	stream.end();

	return result;
}

std::filesystem::path ModPlayLayer::getSavePath() {
	std::string savePath = Mod::get()->getSaveDir().generic_string();
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
