#include "SaveParser.hpp"

const std::hash<std::string> c_stringHasher;

SaveHeader SaveParser::fromStream(
	persistenceAPI::Stream& stream, GJGameLevel* level, bool isEditor
) {
	char loadedHeader[sizeof(SAVE_HEADER)];
	LoadError loadError = LoadError::None;
	unsigned int saveVersion;
	gd::string gameVersion;
	char platform;
	unsigned int levelVersion;
	size_t levelStringHash;
	gd::string levelName;
	unsigned int checkpointCount;

	stream.read(loadedHeader, sizeof(SAVE_HEADER));
	if (strcmp(loadedHeader, SAVE_HEADER) != 0)
		return SaveHeader{LoadError::BadFile, 0, "Unknown", PLATFORM, 0, 0,
								"Unknown"};

	stream >> saveVersion;

	if (saveVersion < 3)
		loadError = LoadError::OutdatedData;
	else if (saveVersion > CURRENT_VERSION)
		loadError = LoadError::NewData;

	if (loadError != LoadError::None)
		return SaveHeader{loadError, saveVersion, "Unknown", PLATFORM,
								0,			  0,				"Unknown"};

	bool isEditorLevel =
		level ? level->m_levelType == GJLevelType::Editor : isEditor;

	stream >> gameVersion;
	stream >> platform;
	if (!isEditorLevel)
		stream >> levelVersion;
	else
		stream >> levelStringHash;
	stream >> levelName;
	stream >> checkpointCount;

	// if (gameVersion != geode::Loader::get()->getGameVersion())
	// 	loadError = LoadError::GameVersionMismatch;

	if (platform != PLATFORM)
		loadError = LoadError::OtherPlatform;

	else if (level) {
		if (!isEditorLevel) {
			if (levelVersion != level->m_levelVersion)
				loadError = LoadError::LevelVersionMismatch;
		} else {
			size_t currentHash = c_stringHasher(level->m_levelString);
			if (levelStringHash != currentHash)
				loadError = LoadError::LevelVersionMismatch;
		}
	}

	return SaveHeader{loadError,	  saveVersion,		 gameVersion, platform,
							levelVersion, checkpointCount, levelName};
}

std::optional<SaveHeader> SaveParser::fromPath(
	std::filesystem::path path, GJGameLevel* level, bool isEditor
) {
	if (!std::filesystem::exists(path))
		return std::nullopt;

	persistenceAPI::Stream stream;
	stream.setFile(geode::utils::string::pathToString(path), 2);

	SaveHeader header = fromStream(stream, level, isEditor);
	stream.end();

	return header;
}

bool SaveParser::isErrorFallbackCapable(LoadError error) {
	switch (error) {
	case LoadError::GameVersionMismatch:
	case LoadError::LevelVersionMismatch:
	case LoadError::Crash:
	case LoadError::ForcedFallback:
		return true;
	default:
		return false;
	}
}
