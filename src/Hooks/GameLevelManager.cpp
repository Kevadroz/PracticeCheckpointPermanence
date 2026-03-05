#include "GameLevelManager.hpp"
#include "PlayLayer.hpp"

void ModGameLevelManager::deleteLevel(GJGameLevel* level) {
	if (Mod::get()->getSettingValue<bool>("delete-saves-with-level"))
		for (bool lowDetail : {false, true}) {
			unsigned int layerCount =
				ModPlayLayer::getSaveLayerCount(level, lowDetail);
			for (unsigned int layer = 0; layer < layerCount; ++layer) {
				std::filesystem::path path =
					ModPlayLayer::getSavePath(level, lowDetail, layer);
				std::filesystem::remove(path);
			}
		}

	GameLevelManager::deleteLevel(level);
}
