#include "SaveParser.hpp"

SaveHeader
SaveParser::fromStream(persistenceAPI::Stream& stream, GJGameLevel* level) {
	gd::string loadedHeader;
	LoadError loadError = LoadError::None;
	unsigned int saveVersion;
	gd::string gameVersion;
	char platform;
	unsigned int levelVersion;
	gd::string levelName;
	unsigned int checkpointCount;

	stream >> loadedHeader;
	if (loadedHeader != SAVE_HEADER)
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

	stream >> gameVersion;
	stream >> platform;
	stream >> levelVersion;
	stream >> levelName;
	stream >> checkpointCount;

	if (gameVersion != geode::Loader::get()->getGameVersion())
		loadError = LoadError::OutdatedData;

	else if (platform != PLATFORM)
		loadError = LoadError::OtherPlatform;

	else if (level && levelVersion != level->m_levelVersion)
		loadError = LoadError::LevelVersionMismatch;

	return SaveHeader{loadError,	  saveVersion,		 gameVersion, platform,
							levelVersion, checkpointCount, levelName};
}

std::optional<SaveHeader>
SaveParser::fromPath(std::filesystem::path path, GJGameLevel* level) {
	if (!std::filesystem::exists(path))
		return std::nullopt;

	persistenceAPI::Stream stream;
	stream.setFile(geode::utils::string::pathToString(path), 2);

	SaveHeader header = fromStream(stream, level);
	stream.end();

	return header;
}
