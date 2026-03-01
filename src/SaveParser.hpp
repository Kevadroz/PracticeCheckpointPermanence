#include <sabe.persistenceapi/include/util/Stream.hpp>

#if defined(GEODE_IS_WINDOWS)
#define PLATFORM 0
#elif defined(GEODE_IS_ANDROID64)
#define PLATFORM 1
#elif defined(GEODE_IS_ANDROID32)
#define PLATFORM 2
#elif defined(GEODE_IS_ARM_MAC)
#define PLATFORM 3
#elif defined(GEODE_IS_INTEL_MAC)
#define PLATFORM 4
#elif defined(GEODE_IS_IOS)
#define PLATFORM 5
#endif

#define SAVE_HEADER "PCP SAVE FILE"
#define CURRENT_VERSION 3

enum LoadError : char {
	None,
	Crash,
	GameVersionMismatch,
	OutdatedData,
	NewData,
	OtherPlatform,
	LevelVersionMismatch,
	BadFile,
};

struct SaveHeader {
	LoadError loadError;

	unsigned int saveVersion;
	gd::string gameVersion;
	char savedPlatform;
	unsigned int levelVersion;
	unsigned int checkpointCount;

	gd::string levelName;
};

namespace SaveParser {
SaveHeader
fromStream(persistenceAPI::Stream& stream, GJGameLevel* level = nullptr);
std::optional<SaveHeader>
fromPath(std::filesystem::path path, GJGameLevel* level = nullptr);
}; // namespace SaveParser
