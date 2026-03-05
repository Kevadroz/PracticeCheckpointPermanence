#include <Geode/modify/GameLevelManager.hpp>

using namespace geode::prelude;

class $modify(ModGameLevelManager, GameLevelManager) {
	void deleteLevel(GJGameLevel* level);
};
