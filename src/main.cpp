#include <Geode/Geode.hpp>
#include <asp/fs/fs.hpp>

using namespace geode::prelude;

$execute {
	asp::fs::Path dataDir = Mod::get()->getSaveDir();
	asp::fs::createDirAll(dataDir.append("saves/editor"))
		.inspectErr([](asp::fs::Error err) {
			geode::log::warn("Cannot create save directories: {}", err);
		});
}
