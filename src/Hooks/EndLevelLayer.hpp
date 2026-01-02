#pragma once
#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

class $modify(ModEndLevelLayer, EndLevelLayer) {
	void onRestartCheckpoint(cocos2d::CCObject* sender);
};
