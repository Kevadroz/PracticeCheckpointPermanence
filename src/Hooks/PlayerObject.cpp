#include "PlayerObject.hpp"
#include "PlayLayer.hpp"

void ModPlayerObject::tryPlaceCheckpoint() {
	if (ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get()))
		if (playLayer->m_fields->m_activeCheckpoint > 0 &&
			 Mod::get()->getSettingValue<bool>("supress-auto-checkpoints"))
			return;

	PlayerObject::tryPlaceCheckpoint();
}
