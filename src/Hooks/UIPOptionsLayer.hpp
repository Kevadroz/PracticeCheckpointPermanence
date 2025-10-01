#pragma once
#include "../UI/SwitcherMenu.hpp"

#include <Geode/modify/UIPOptionsLayer.hpp>

using namespace geode::prelude;

class $modify(ModUIPOptionsLayer, UIPOptionsLayer) {
	struct Fields {
		SwitcherMenu* m_switcherMenu = nullptr;
		bool m_movingSwitcher;
		CCPoint m_lastPos;

		TextInput* m_switcherScaleInput;
		Slider* m_switcherScaleSlider;
	};

	bool init();
	void onClose(CCObject* sender);
	void onReset(CCObject* sender);

	void updateSwitcherScale(float scale);
	void updateSwitcherScaleSlider(float scale);
	void saveSwitcherSettings();

	void onSwitcherScaleSliderUpdated(CCObject* slider);
};

float inverseLerp(float a, float b, float v);
