#pragma once
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

class $modify(ModPauseLayer, PauseLayer) {
	static void onModify(auto& self) {
		self.setHookPriorityBeforePost(
			"PauseLayer::customSetup", "thesillydoggo.qolmod"
		);
	}

	void customSetup();
};
