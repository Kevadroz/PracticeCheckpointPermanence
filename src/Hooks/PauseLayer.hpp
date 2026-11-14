#pragma once
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

class $modify(ModPauseLayer, PauseLayer) {
	void customSetup();

#ifdef GEODE_WINDOWS
	void onPracticeMode(CCObject* sender);
#endif
};
