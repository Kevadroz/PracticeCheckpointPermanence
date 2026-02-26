#include "PauseLayer.hpp"
#include "../UI/CheckpointManager.hpp"
#include "PlayLayer.hpp"

#include <filesystem>

void ModPauseLayer::customSetup() {
	ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());

	PauseLayer::customSetup();

	if (playLayer->m_level->m_levelType == GJLevelType::Editor)
		return;

	CircleButtonSprite* buttonSprite =
		CircleButtonSprite::createWithSpriteFrameName("activeCheckpoint.png"_spr);
	buttonSprite->setScale(.6f);

	CCNode* sprite = buttonSprite->getChildByIndex(0);
	sprite->setRotation(15.f);

	CCMenuItemSpriteExtra* button = CCMenuItemExt::createSpriteExtra(
		buttonSprite, [playLayer](CCObject* sender) {
			if (playLayer->isPersistentSystemActive())
				CheckpointManager::create()->show();
			else {
				std::filesystem::path path = playLayer->getSavePath();
				if (std::filesystem::exists(path))
					geode::createQuickPopup(
						"Persistent Checkpoints",
						"Open in practice mode to manage checkpoints.", "Ok",
						"Delete Saved", [path, playLayer](auto, bool confirmed) {
							if (confirmed)
								geode::createQuickPopup(
									"Delete All",
									"Delete all saved checkpoints for this level?\n"
									"This action cannot be undone.",
									"Cancel", "Delete",
									[path, playLayer](auto, bool confirmed) {
										if (confirmed)
											std::filesystem::remove(path);
										while (true) {
											playLayer->m_fields->m_activeSaveLayer++;
											std::filesystem::path layerPath =
												playLayer->getSavePath();
											if (std::filesystem::exists(layerPath))
												std::filesystem::remove(layerPath);
											else
												break;
										}
										playLayer->m_fields->m_activeSaveLayer = 0;
									}
								);
						}
					);
				else
					FLAlertLayer::create(
						"Persistent Checkpoints",
						"Open in practice mode to manage checkpoints.", "Ok"
					)
						->show();
			}
		}
	);

	CCMenu* menu = static_cast<CCMenu*>(getChildByID("right-button-menu"));
	menu->addChild(button);
	menu->updateLayout();
}
