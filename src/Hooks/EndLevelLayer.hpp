#pragma once
#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

class $modify(ModEndLevelLayer, EndLevelLayer) {
	void onRestartCheckpoint(CCObject* sender);
};
