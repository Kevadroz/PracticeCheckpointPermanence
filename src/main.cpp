#include <Geode/Geode.hpp>

#include <filesystem>

using namespace geode::prelude;

$execute {
	std::string dataDir = string::pathToString(Mod::get()->getSaveDir());
	std::filesystem::create_directories(fmt::format("{}/saves/main/", dataDir));
	std::filesystem::create_directories(
		fmt::format("{}/saves/editor/", dataDir)
	);
}
