#include "GJBaseGameLayer.hpp"
#include "PlayLayer.hpp"

void ModGJBaseGameLayer::loadStartPosObject() {
	GJBaseGameLayer::loadStartPosObject();

	if (ModPlayLayer* playLayer = static_cast<ModPlayLayer*>(PlayLayer::get()))
		for (PersistentCheckpoint* checkpoint : CCArrayExt<PersistentCheckpoint*>(
				  playLayer->m_fields->m_persistentCheckpointArray
			  ))
			if (m_startPosObject == checkpoint->m_fallbackData.startPos) {
				FallbackData fallbackData = checkpoint->m_fallbackData;

				m_player1->setYVelocity(fallbackData.p1Velocity.y, 0);
				m_player1->m_platformerXVelocity = fallbackData.p1Velocity.x;

				if (fallbackData.startPos->m_startSettings->m_startDual) {
					m_player2->setPosition(fallbackData.p2Position);
					m_player2->m_position = fallbackData.p2Position;
					m_player2->setYVelocity(fallbackData.p2Velocity.y, 0);
					m_player2->m_platformerXVelocity = fallbackData.p2Velocity.x;

					switch (fallbackData.p2Gamemode) {
					case StartPosGameModes::Normal:
						m_player2->toggleFlyMode(false, false);
						m_player2->toggleRollMode(false, false);
						m_player2->toggleBirdMode(false, false);
						m_player2->toggleDartMode(false, false);
						m_player2->toggleRobotMode(false, false);
						m_player2->toggleSpiderMode(false, false);
						m_player2->toggleSwingMode(false, false);
						break;
					case StartPosGameModes::Ship:
						m_player2->toggleFlyMode(true, true);
						break;
					case StartPosGameModes::Ball:
						m_player2->toggleRollMode(true, true);
						break;
					case StartPosGameModes::UFO:
						m_player2->toggleBirdMode(true, true);
						break;
					case StartPosGameModes::Wave:
						m_player2->toggleDartMode(true, true);
						break;
					case StartPosGameModes::Robot:
						m_player2->toggleRobotMode(true, true);
						break;
					case StartPosGameModes::Spider:
						m_player2->toggleSpiderMode(true, true);
						break;
					case StartPosGameModes::Swing:
						m_player2->toggleSwingMode(true, true);
						break;
					}

					m_player2->togglePlayerScale(fallbackData.p2IsMini, true);
					m_player2->flipGravity(fallbackData.p2IsFlipped, true);
					m_player2->doReversePlayer(fallbackData.p2IsReverseGameplay);
				}

				if (fallbackData.freeMode) {
					TeleportPortalObject* tpObj = TeleportPortalObject::create(
						"edit_eGameRotBtn_001.png", true
					);
					tpObj->m_cameraIsFreeMode = true;
					tpObj->m_cameraEasingValue = 10;
					tpObj->m_cameraPaddingValue = .5f;
					playerWillSwitchMode(m_player1, tpObj);
					playLayer->m_fields->m_holdedTeleportObject = tpObj;
				}

				m_gameState.m_cameraPosition = fallbackData.cameraPosition;
				m_gameState.m_cameraOffset = fallbackData.cameraOffset;
				m_gameState.m_cameraZoom = fallbackData.cameraZoom;

				break;
			}
}
