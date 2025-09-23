#pragma once
#include "../UI/SwitcherMenu.hpp"

#include <Geode/modify/UIPOptionsLayer.hpp>

using namespace geode::prelude;

class $modify(ModUIPOptionsLayer, UIPOptionsLayer) {
	struct Fields {
		SwitcherMenu* m_switcherMenu = nullptr;
		bool m_movingSwitcher;
		CCPoint m_lastPos;
	};

	bool init();
	void onClose(CCObject* sender);
	void onReset(CCObject* sender);

	void saveSwitcherPosition();
};
