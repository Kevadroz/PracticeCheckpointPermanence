#include "LevelBrowserLayer.hpp"
#include "../UI/SaveManager.hpp"

bool ModLevelBrowserLayer::init(GJSearchObject* p0) {
	if (!LevelBrowserLayer::init(p0))
		return false;

	if (CCNode* savedMenu = getChildByID("saved-menu")) {
		CircleButtonSprite* buttonSprite =
			CircleButtonSprite::createWithSpriteFrameName(
				"activeCheckpoint.png"_spr
			);
		buttonSprite->setScale(.8f);

		CCNode* sprite = buttonSprite->getChildByIndex(0);
		sprite->setRotation(15.f);

		CCMenuItemSpriteExtra* button =
			CCMenuItemExt::createSpriteExtra(buttonSprite, [](CCObject* sender) {
				SaveManager::create()->show();
			});

		CCNode* favoriteButton = savedMenu->getChildByID("favorite-button");
		if (favoriteButton == nullptr)
			return true;

		CCPoint buttonPosition = favoriteButton->getPosition();
		button->setPosition(buttonPosition + ccp(0., 40.));
		button->setID("save-manager"_spr);

		savedMenu->addChild(button);
	}

	return true;
}
