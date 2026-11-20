#include "GameLevelManager.hpp"
#include "PlayLayer.hpp"

#include <asp/fs/fs.hpp>

void ModGameLevelManager::deleteLevel(GJGameLevel* level) {
	if (Mod::get()->getSettingValue<bool>("delete-saves-with-level"))
		for (bool lowDetail : {false, true}) {
			unsigned int layerCount =
				ModPlayLayer::getSaveLayerCount(level, lowDetail);
			for (unsigned int layer = 0; layer < layerCount; ++layer) {
				asp::fs::Path path =
					ModPlayLayer::getSavePath(level, lowDetail, layer);
				asp::fs::remove(path).inspectErr([path](asp::fs::Error err) {
					log::error(
						"Cannot remove save layer with path '{}': "
						"{}",
						path, err
					);
				});
			}
		}

	GameLevelManager::deleteLevel(level);
}
