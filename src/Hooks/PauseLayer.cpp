#include "PauseLayer.hpp"
#include "../UI/CheckpointManager.hpp"
#include "PlayLayer.hpp"

#include <asp/fs/fs.hpp>

void ModPauseLayer::customSetup() {
	ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get());

	bool isEditor = playLayer->m_level->m_levelType == GJLevelType::Editor;

	PauseLayer::customSetup();

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
				asp::fs::Path path = playLayer->getSavePath();
				if (path.exists())
					createQuickPopup(
						"Persistent Checkpoints",
						"Open in practice mode to manage checkpoints.", "Ok",
						"Delete Saved", [path, playLayer](auto, bool confirmed) {
							if (confirmed)
								createQuickPopup(
									"Delete All",
									"Delete all saved checkpoints for this "
									"level?\n"
									"This action cannot be undone.",
									"Cancel", "Delete",
									[playLayer](auto, bool confirmed) {
										if (!confirmed)
											return;
										playLayer->m_fields->m_activeSaveLayer = 0;
										while (true) {
											asp::fs::Path layerPath =
												playLayer->getSavePath();
											if (layerPath.exists())
												asp::fs::remove(layerPath).inspectErr(
													[layerPath](asp::fs::Error err) {
														log::error(
															"Cannot remove save layer with "
															"path '{}': {}",
															layerPath, err
														);
													}
												);
											else
												break;
											playLayer->m_fields->m_activeSaveLayer++;
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

	CCMenu* menu =
		static_cast<CCMenu*>(m_mainLayer->getChildByID("right-button-menu"));
	if (menu) {
		menu->addChild(button);
		menu->updateLayout();
	}
}

#ifdef GEODE_WINDOWS
void ModPauseLayer::onPracticeMode(CCObject* sender) {
	PauseLayer::onPracticeMode(sender);
	static_cast<ModPlayLayer*>(PlayLayer::get())->handleTogglePracticeMode(true);
}
#endif
