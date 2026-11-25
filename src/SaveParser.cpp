#include "SaveParser.hpp"

const std::hash<std::string> c_stringHasher;

SaveHeader SaveParser::fromStream(
	InStream& stream, GJGameLevel* level, bool isEditor, bool ignoreVerification
) {
	char loadedHeader[sizeof(SAVE_HEADER)];
	LoadError loadError = LoadError::None;
	unsigned int saveVersion;
	std::string gameVersion;
	char platform;
	unsigned int levelVersion;
	size_t levelStringHash;
	std::string levelName;
	unsigned int checkpointCount;

	stream.read(loadedHeader, sizeof(loadedHeader));
	if (strcmp(loadedHeader, SAVE_HEADER) != 0 && !ignoreVerification)
		return SaveHeader{LoadError::BadFile, 0, "Unknown", PLATFORM, 0, 0,
								"Unknown"};

	stream >> saveVersion;

	if (saveVersion < 3)
		loadError = LoadError::OutdatedData;
	else if (saveVersion > CURRENT_VERSION)
		loadError = LoadError::NewData;

	if (loadError != LoadError::None && !ignoreVerification)
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

	if (gameVersion != Loader::get()->getGameVersion())
		loadError = LoadError::GameVersionMismatch;

	if (platform != PLATFORM)
		loadError = LoadError::OtherPlatform;

	if (level) {
		if (!isEditorLevel) {
			if (levelVersion != level->m_levelVersion)
				loadError = LoadError::LevelVersionMismatch;
		} else {
			size_t currentHash = c_stringHasher(level->m_levelString);
			if (levelStringHash != currentHash)
				loadError = LoadError::LevelVersionMismatch;
		}
	}

	if (ignoreVerification)
		loadError = LoadError::None;

	return SaveHeader{loadError,	  saveVersion,		 gameVersion, platform,
							levelVersion, checkpointCount, levelName};
}

std::optional<SaveHeader>
SaveParser::fromPath(asp::fs::Path path, GJGameLevel* level, bool isEditor) {
	if (!path.exists())
		return std::nullopt;

	InStream stream(path.string());

	return fromStream(stream, level, isEditor);
}

bool SaveParser::isErrorFallbackCapable(LoadError error) {
	switch (error) {
	case LoadError::GameVersionMismatch:
	case LoadError::LevelVersionMismatch:
	case LoadError::OtherPlatform:
	case LoadError::Crash:
	case LoadError::ForcedFallback:
		return true;
	default:
		return false;
	}
}
