#pragma once
#include "../UI/SwitcherMenu.hpp"
#include "PlayLayer.hpp"

#include <Geode/modify/UILayer.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;

class $modify(ModUILayer, UILayer) {
	struct Fields {
		SwitcherMenu* m_switcherMenu = nullptr;
		CCNodeRGBA* m_createCheckpointButton = nullptr;
		CCNodeRGBA* m_removeCheckpointButton = nullptr;
	};

	bool init(GJBaseGameLayer*);
	void updateSwitcher();
	void resetSwitcherOpacity();
};

// If playLayer is nullptr then it will be
// assumed that this is for the UIPOptionsLayer
CCNodeRGBA* createCheckpointCreateButton(CCNode* sibling, ModPlayLayer* playLayer);
CCNodeRGBA* createCheckpointRemoveButton(CCNode* sibling, ModPlayLayer* playLayer);

void createButtonBindsLabel(
	CCNode* parent, const keybinds::ActionID& action, bool right
);
