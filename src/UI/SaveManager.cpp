#include "SaveManager.hpp"

SaveManager* SaveManager::create() {
	SaveManager* ret = new SaveManager();
	if (ret->initAnchored(440.f, 260.f, "GJ_square05.png")) {
		ret->autorelease();
		return ret;
	}

	delete ret;
	return nullptr;
}

bool SaveManager::setup() {
	setTitle("PCP Save Manager");

	return true;
}
