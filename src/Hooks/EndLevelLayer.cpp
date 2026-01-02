#include "EndLevelLayer.hpp"
#include "PlayLayer.hpp"

void ModEndLevelLayer::onRestartCheckpoint(cocos2d::CCObject* sender) {
	if (static_cast<ModPlayLayer*>(m_playLayer)->m_fields->m_activeCheckpoint >
			 0 &&
		 Mod::get()->getSettingValue<bool>("reset-attempts"))
		m_playLayer->m_attempts = 0;

	EndLevelLayer::onRestartCheckpoint(sender);
}
