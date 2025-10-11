#include <Geode/Geode.hpp>
#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>
#endif

#include <filesystem>

using namespace geode::prelude;
#ifndef GEODE_IS_IOS
using namespace keybinds;
#endif

$execute {
	std::string dataDir = string::pathToString(Mod::get()->getSaveDir());
	std::filesystem::create_directories(fmt::format("{}/saves/main/", dataDir));
	std::filesystem::create_directories(
		fmt::format("{}/saves/editor/", dataDir)
	);

	#ifndef GEODE_IS_IOS
	BindManager::get()->registerBindable(
		{// ID, should be prefixed with mod ID
		 "create_checkpoint"_spr,
		 // Name
		 "Create Checkpoint",
		 // Description, leave empty for none
		 "Creates a new persistent checkpoint",
		 // Default binds
		 {Keybind::create(KEY_Z, Modifier::Alt)},
		 // Category; use slashes for specifying subcategories. See the
		 // Category class for default categories
		 "PCP"
		}
	);
	BindManager::get()->registerBindable(
		{"remove_checkpoint"_spr,
		 "Remove Checkpoint",
		 "Removes the active persistent checkpoint",
		 {Keybind::create(KEY_X, Modifier::Alt)},
		 "PCP"}
	);
	BindManager::get()->registerBindable(
		{"previous_checkpoint"_spr,
		 "Previous Checkpoint",
		 "Activates the previous persistent checkpoint",
		 {Keybind::create(KEY_Q, Modifier::Alt)},
		 "PCP"}
	);
	BindManager::get()->registerBindable(
		{"next_checkpoint"_spr,
		 "Next Checkpoint",
		 "Activates the next persistent checkpoint",
		 {Keybind::create(KEY_E, Modifier::Alt)},
		 "PCP"}
	);
	BindManager::get()->registerBindable(
		{"previous_layer"_spr,
		 "Previous Layer",
		 "Switches to the previous save layer",
		 {Keybind::create(KEY_Q, Modifier::Alt | Modifier::Shift)},
		 "PCP"}
	);
	BindManager::get()->registerBindable(
		{"next_layer"_spr,
		 "Next Layer",
		 "Switcher to the next save layer",
		 {Keybind::create(KEY_E, Modifier::Alt | Modifier::Shift)},
		 "PCP"}
	);
	#endif
}
