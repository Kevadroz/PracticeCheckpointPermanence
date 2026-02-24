#include "SaveManager.hpp"

SaveManager* SaveManager::create() {
	SaveManager* ret = new SaveManager();
	if (ret->init()) {
		ret->autorelease();
		return ret;
	}

	delete ret;
	return nullptr;
}

bool SaveManager::init() {
	if (!Popup::init(440.f, 260.f, "GJ_square05.png"))
		return false;

	setTitle("PCP Save Manager");

	return true;
}
