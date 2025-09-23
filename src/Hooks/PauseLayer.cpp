#include "PauseLayer.hpp"
#include "../UI/CheckpointManager.hpp"
#include "PlayLayer.hpp"

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/PauseLayer.hpp>
#include <Geode/binding/PlayLayer.hpp>

#include <filesystem>

void ModPauseLayer::customSetup() {
	PlayLayer* playLayer = PlayLayer::get();

	PauseLayer::customSetup();

	CircleButtonSprite* buttonSprite =
		CircleButtonSprite::createWithSpriteFrameName("activeCheckpoint.png"_spr);
	buttonSprite->setScale(.6f);

	CCNode* sprite = buttonSprite->getChildByIndex(0);
	sprite->setRotation(15.f);

	CCMenuItemSpriteExtra* button = CCMenuItemExt::createSpriteExtra(
		buttonSprite, [playLayer](CCObject* sender) {
			if (playLayer->m_isPracticeMode)
				CheckpointManager::create()->show();
			else {
				ModPlayLayer* modPlayLayer = static_cast<ModPlayLayer*>(playLayer);
				std::filesystem::path path = modPlayLayer->getSavePath();
				if (std::filesystem::exists(path))
					geode::createQuickPopup(
						"Persistent Checkpoints",
						"Open in practice mode to manage checkpoints.", "Ok",
						"Delete Saved", [path, modPlayLayer](auto, bool confirmed) {
							if (confirmed)
								geode::createQuickPopup(
									"Delete All",
									"Delete all saved checkpoints for this level?\n"
									"This action cannot be undone.",
									"Cancel", "Delete",
									[path, modPlayLayer](auto, bool confirmed) {
										if (confirmed)
											std::filesystem::remove(path);
										while (true) {
											modPlayLayer->m_fields->m_activeSaveLayer++;
											std::filesystem::path layerPath =
												modPlayLayer->getSavePath();
											if (std::filesystem::exists(layerPath))
												std::filesystem::remove(layerPath);
											else
												break;
										}
										modPlayLayer->m_fields->m_activeSaveLayer = 0;
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

	getChildByID("right-button-menu")->addChild(button);
}
