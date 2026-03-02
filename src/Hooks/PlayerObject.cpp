#include "PlayerObject.hpp"
#include "PlayLayer.hpp"

void ModPlayerObject::tryPlaceCheckpoint() {
	if (ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get())) {
		if (playLayer->m_fields->m_ghostActiveCheckpoint > 0 &&
			 playLayer->m_attemptTime -
					 playLayer->m_fields->m_ghostCheckpointUsedTime <
				 Mod::get()->getSettingValue<double>(
					 "ghost-auto-checkpoint-inactive-time"
				 ))
			return;

		if (playLayer->m_fields->m_activeCheckpoint > 0 &&
			 Mod::get()->getSettingValue<bool>("supress-auto-checkpoints"))
			return;
	}

	PlayerObject::tryPlaceCheckpoint();
}
