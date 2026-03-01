#include "PersistentCheckpoint.hpp"

#include <Geode/binding/CheckpointObject.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PlayerCheckpoint.hpp>

#include <sabe.persistenceapi/include/PersistenceAPI.hpp>

using namespace persistenceAPI;

PersistentCheckpoint* PersistentCheckpoint::create() {
	PersistentCheckpoint* checkpoint = new PersistentCheckpoint();
	checkpoint->m_checkpoint = CheckpointObject::create();

	checkpoint->autorelease();

	return checkpoint;
}

void PersistentCheckpoint::storeData(
	CheckpointObject* checkpoint, PlayLayer* playLayer
) {
	m_checkpoint = checkpoint;

	if (checkpoint->m_physicalCheckpointObject != nullptr) {
		m_objectPos = checkpoint->m_physicalCheckpointObject->m_startPosition;

		setupPhysicalObject();
	}

	m_time = playLayer->m_timePlayed;
	m_percent = playLayer->getCurrentPercent();
	m_persistentItemCountMap =
		playLayer->m_effectManager->m_persistentItemCountMap;
	m_persistentTimerItemSet =
		playLayer->m_effectManager->m_persistentTimerItemSet;
}

void PersistentCheckpoint::serialize(Stream& out) {
	reinterpret_cast<PACCNode*>(m_checkpoint.data())->save(out);

	bool hasP2 = m_checkpoint->m_player2Checkpoint != nullptr;
	bool hasGradients = m_checkpoint->m_gradientTriggerObjectArray != nullptr;

	out << hasP2;
	out << hasGradients;

	reinterpret_cast<PAGJGameState*>(&m_checkpoint->m_gameState)->save(out);
	reinterpret_cast<PAGJShaderState*>(&m_checkpoint->m_shaderState)->save(out);
	reinterpret_cast<PAFMODAudioState*>(&m_checkpoint->m_audioState)->save(out);

	reinterpret_cast<PAPlayerCheckpoint*>(m_checkpoint->m_player1Checkpoint)
		->save(out);
	if (hasP2)
		reinterpret_cast<PAPlayerCheckpoint*>(m_checkpoint->m_player2Checkpoint)
			->save(out);

	out << m_checkpoint->m_unke78;
	out << m_checkpoint->m_unke7c;
	out << m_checkpoint->m_unke80;
	out << m_checkpoint->m_ground2Invisible;
	out << m_checkpoint->m_streakBlend;
	out << m_checkpoint->m_uniqueID;
	out << m_checkpoint->m_respawnID;
	out << m_checkpoint->m_vectorSavedObjectStateRef;
	out << m_checkpoint->m_vectorActiveSaveObjectState;
	out << m_checkpoint->m_vectorSpecialSaveObjectState;

	reinterpret_cast<PAEffectManagerState*>(&m_checkpoint->m_effectManagerState)
		->save(out);
	if (hasGradients)
		static_cast<PACCArray*>(m_checkpoint->m_gradientTriggerObjectArray)
			->save<GradientTriggerObject>(out);

	out << m_checkpoint->m_unk11e8;
	out << m_checkpoint->m_sequenceTriggerStateUnorderedMap;
	out << m_checkpoint->m_commandIndex;

	// Custom data
	out << m_persistentItemCountMap;
	out << m_persistentTimerItemSet;
}

void PersistentCheckpoint::deserialize(Stream& in, SaveHeader header) {
	reinterpret_cast<PACCNode*>(m_checkpoint.data())->load(in);

	bool hasP2;
	bool hasGradients;

	in >> hasP2;
	in >> hasGradients;

	// geode::log::debug("p2 {}", hasP2);

	reinterpret_cast<PAGJGameState*>(&m_checkpoint->m_gameState)->load(in);
	// geode::log::debug("gs");
	reinterpret_cast<PAGJShaderState*>(&m_checkpoint->m_shaderState)->load(in);
	// geode::log::debug("ss");
	reinterpret_cast<PAFMODAudioState*>(&m_checkpoint->m_audioState)->load(in);
	// geode::log::debug("as");

	m_checkpoint->m_player1Checkpoint = PlayerCheckpoint::create();
	// Realeased by m_checkpoint's destructor
	CC_SAFE_RETAIN(m_checkpoint->m_player1Checkpoint);
	reinterpret_cast<PAPlayerCheckpoint*>(m_checkpoint->m_player1Checkpoint)
		->load(in);
	if (hasP2) {
		m_checkpoint->m_player2Checkpoint = PlayerCheckpoint::create();
		// Realeased by m_checkpoint's destructor
		CC_SAFE_RETAIN(m_checkpoint->m_player2Checkpoint);
		reinterpret_cast<PAPlayerCheckpoint*>(m_checkpoint->m_player2Checkpoint)
			->load(in);
	}

	// geode::log::debug("checkpoints");

	in >> m_checkpoint->m_unke78;
	in >> m_checkpoint->m_unke7c;
	in >> m_checkpoint->m_unke80;
	in >> m_checkpoint->m_ground2Invisible;
	in >> m_checkpoint->m_streakBlend;
	in >> m_checkpoint->m_uniqueID;
	in >> m_checkpoint->m_respawnID;
	in >> m_checkpoint->m_vectorSavedObjectStateRef;
	in >> m_checkpoint->m_vectorActiveSaveObjectState;
	in >> m_checkpoint->m_vectorSpecialSaveObjectState;

	// geode::log::debug("vars");

	reinterpret_cast<PAEffectManagerState*>(&m_checkpoint->m_effectManagerState)
		->load(in);
	if (hasGradients) {
		m_checkpoint->m_gradientTriggerObjectArray = CCArray::create();
		// Realeased by m_checkpoint's destructor
		CC_SAFE_RETAIN(m_checkpoint->m_gradientTriggerObjectArray);
		static_cast<PACCArray*>(m_checkpoint->m_gradientTriggerObjectArray)
			->load<GradientTriggerObject>(in);
	}

	// geode::log::debug("gradients {}", hasGradients);

	in >> m_checkpoint->m_unk11e8;
	in >> m_checkpoint->m_sequenceTriggerStateUnorderedMap;
	in >> m_checkpoint->m_commandIndex;

	// geode::log::debug("eof {}", m_commandIndex);

	// Custom data
	in >> m_persistentItemCountMap;
	in >> m_persistentTimerItemSet;
}

void PersistentCheckpoint::serializeExternal(Stream& out) {
	out << m_objectPos;
	out << m_time;
	out << m_percent;

	// Fallback
	PlayerCheckpoint* p1Checkpoint = m_checkpoint->m_player1Checkpoint;
	PlayerCheckpoint* p2Checkpoint = m_checkpoint->m_player2Checkpoint;

	bool isDualMode =
		p2Checkpoint != nullptr && m_checkpoint->m_gameState.m_isDualMode;

	int mode = getGamemodeFromCheckpoint(p1Checkpoint);
	out << mode;

	int speed = (int)(p1Checkpoint->m_playerSpeed < 0.8f	 ? Speed::Slow
							: p1Checkpoint->m_playerSpeed > 1.0f ? Speed::Fast
							: p1Checkpoint->m_playerSpeed > 1.2f ? Speed::Faster
							: p1Checkpoint->m_playerSpeed > 1.4f ? Speed::Fastest
																			 : Speed::Normal);
	out << speed;

	out << p1Checkpoint->m_isMini;
	out << isDualMode;
	out << m_checkpoint->m_gameState.m_unkBool10; // Mirror Mode
	out << p1Checkpoint->m_isSideways;
	out << p1Checkpoint->m_isUpsideDown;
	out << p1Checkpoint->m_isGoingLeft;

	// Space reserved, but I don't think that this is possible.
	int targetOrder = 0;
	out << targetOrder;

	out << m_checkpoint->m_gameState.m_currentChannel;

	// Custom Data
	CCPoint p1Velocity =
		ccp(p1Checkpoint->m_platformerXVelocity, p1Checkpoint->m_yVelocity);

	out << p1Velocity;

	if (isDualMode) {
		CCPoint p2Velocity =
			ccp(p2Checkpoint->m_platformerXVelocity, p2Checkpoint->m_yVelocity);
		int p2Gamemode = getGamemodeFromCheckpoint(p2Checkpoint);

		out << p2Velocity;
		out << p2Checkpoint->m_position;
		out << p2Gamemode;
		out << p2Checkpoint->m_isMini;
		out << p2Checkpoint->m_isUpsideDown;
		out << p2Checkpoint->m_isGoingLeft;
	}

	out << m_checkpoint->m_gameState.m_unkBool8; // Free Mode
	out << m_checkpoint->m_gameState.m_cameraPosition;
	out << m_checkpoint->m_gameState.m_cameraOffset;
	out << m_checkpoint->m_gameState.m_cameraZoom;
}

void PersistentCheckpoint::deserializeExternal(Stream& in, SaveHeader header) {
	in >> m_objectPos;
	in >> m_time;
	in >> m_percent;

	// Fallback
	StartPosObject* startPos = StartPosObject::create();

	LevelSettingsObject* settings = startPos->m_startSettings;
	settings->m_startsWithStartPos = true;

	in >> settings->m_startMode;

	int speed;
	in >> speed;
	settings->m_startSpeed = (Speed)speed;

	in >> settings->m_startMini;
	in >> settings->m_startDual;
	in >> settings->m_mirrorMode;
	in >> settings->m_rotateGameplay;
	in >> settings->m_isFlipped;
	in >> settings->m_reverseGameplay;
	in >> settings->m_targetOrder;
	in >> settings->m_targetChannel;

	// Custom Data
	CCPoint cameraPosition;

	in >> m_fallbackData.p1Velocity;

	if (settings->m_startDual) {
		int p2Gamemode;

		in >> m_fallbackData.p2Velocity;
		in >> m_fallbackData.p2Position;
		in >> p2Gamemode;
		in >> m_fallbackData.p2IsMini;
		in >> m_fallbackData.p2IsFlipped;
		in >> m_fallbackData.p2IsReverseGameplay;

		m_fallbackData.p2Gamemode = (StartPosGameModes)p2Gamemode;
	}

	in >> m_fallbackData.freeMode;
	in >> cameraPosition;
	in >> m_fallbackData.cameraOffset;
	in >> m_fallbackData.cameraZoom;
	// Custom Data end

	startPos->setSettings(settings);
	startPos->setStartPos(cameraPosition);

	m_fallbackData.startPos = startPos;
}

void PersistentCheckpoint::setupPhysicalObject() {
	if (m_checkpoint->m_physicalCheckpointObject == nullptr)
		m_checkpoint->m_physicalCheckpointObject =
			GameObject::createWithFrame("inactiveCheckpoint.png"_spr);
	else
		m_checkpoint->m_physicalCheckpointObject->setDisplayFrame(
			CCSpriteFrameCache::get()->spriteFrameByName(
				"inactiveCheckpoint.png"_spr
			)
		);

	m_checkpoint->m_physicalCheckpointObject->setOpacity(
		Mod::get()->getSettingValue<double>("inactive-checkpoint-opacity") * 255
	);
	m_checkpoint->m_physicalCheckpointObject->m_objectID = 0x2c;
	m_checkpoint->m_physicalCheckpointObject->m_objectType =
		GameObjectType::Decoration;
	m_checkpoint->m_physicalCheckpointObject->m_glowSprite = nullptr;

	m_checkpoint->m_physicalCheckpointObject->setStartPos(m_objectPos);
}

void PersistentCheckpoint::toggleActive(bool active) {
	const char* frameName;
	if (active) {
		frameName = "activeCheckpoint.png"_spr;
		m_checkpoint->m_physicalCheckpointObject->setOpacity(255);
		m_checkpoint->m_physicalCheckpointObject->setOpacity(
			Mod::get()->getSettingValue<double>("active-checkpoint-opacity") * 255
		);
	} else {
		frameName = "inactiveCheckpoint.png"_spr;
		m_checkpoint->m_physicalCheckpointObject->setOpacity(
			Mod::get()->getSettingValue<double>("inactive-checkpoint-opacity") *
			255
		);
	}
	m_checkpoint->m_physicalCheckpointObject->setDisplayFrame(
		CCSpriteFrameCache::get()->spriteFrameByName(frameName)
	);
}

#if defined(PA_DEBUG) && defined(PA_DESCRIBE)
void PersistentCheckpoint::describe() {
	log::info("[PersistentCheckpoint - describe] start");
	reinterpret_cast<PACCObject*>(m_checkpoint->getObject())->describe();
	reinterpret_cast<PACCNode*>(m_checkpoint->getObject())->describe();
	reinterpret_cast<PAGJGameState*>(&m_checkpoint->m_gameState)->describe();
	reinterpret_cast<PAGJShaderState*>(&m_checkpoint->m_shaderState)->describe();
	reinterpret_cast<PAFMODAudioState*>(&m_checkpoint->m_audioState)->describe();
	log::info(
		"[PersistentCheckpoint - describe] "
		"m_physicalCheckpointObject->m_startPosition: {}",
		m_checkpoint->m_physicalCheckpointObject->m_startPosition
	);
	reinterpret_cast<PAPlayerCheckpoint*>(m_checkpoint->m_player1Checkpoint)
		->describe();
	if (m_checkpoint->m_player2Checkpoint) {
		reinterpret_cast<PAPlayerCheckpoint*>(m_checkpoint->m_player2Checkpoint)
			->describe();
	}
	log::info(
		"[PersistentCheckpoint - describe] m_unke78: {}", m_checkpoint->m_unke78
	);
	log::info(
		"[PersistentCheckpoint - describe] m_unke7c: {}", m_checkpoint->m_unke7c
	);
	log::info(
		"[PersistentCheckpoint - describe] m_unke80: {}", m_checkpoint->m_unke80
	);
	log::info(
		"[PersistentCheckpoint - describe] m_ground2Invisible: {}",
		m_checkpoint->m_ground2Invisible
	);
	log::info(
		"[PersistentCheckpoint - describe] m_streakBlend: {}",
		m_checkpoint->m_streakBlend
	);
	log::info(
		"[PersistentCheckpoint - describe] m_uniqueID: {}",
		m_checkpoint->m_uniqueID
	);
	log::info(
		"[PersistentCheckpoint - describe] m_respawnID: {}",
		m_checkpoint->m_respawnID
	);
	int l_size = m_checkpoint->m_vectorSavedObjectStateRef.size();
	log::info(
		"[PersistentCheckpoint - describe] m_vectorSavedObjectStateRef.size(): "
		"{}",
		l_size
	);
	for (int i = 0; i < l_size; i++) {
		log::info(
			"[PersistentCheckpoint - describe] m_vectorSavedObjectStateRef[{}]:", i
		);
		reinterpret_cast<PASavedObjectStateRef*>(
			&m_checkpoint->m_vectorSavedObjectStateRef[i]
		)
			->describe();
	}
	l_size = m_checkpoint->m_vectorActiveSaveObjectState.size();
	log::info(
		"[PersistentCheckpoint - describe] m_vectorActiveSaveObjectState.size(): "
		"{}",
		l_size
	);
	for (int i = 0; i < l_size; i++) {
		log::info(
			"[PersistentCheckpoint - describe] m_vectorActiveSaveObjectState[{}]:",
			i
		);
		reinterpret_cast<PASavedActiveObjectState*>(
			&m_checkpoint->m_vectorActiveSaveObjectState[i]
		)
			->describe();
	}
	l_size = m_checkpoint->m_vectorSpecialSaveObjectState.size();
	log::info(
		"[PersistentCheckpoint - describe] "
		"m_vectorSpecialSaveObjectState.size(): "
		"{}",
		l_size
	);
	for (int i = 0; i < l_size; i++) {
		log::info(
			"[PersistentCheckpoint - describe] "
			"m_vectorSpecialSaveObjectState[{}]:",
			i
		);
		reinterpret_cast<PASavedSpecialObjectState*>(
			&m_checkpoint->m_vectorSpecialSaveObjectState[i]
		)
			->describe();
	}
	reinterpret_cast<PAEffectManagerState*>(&m_checkpoint->m_effectManagerState)
		->describe();
	if (m_checkpoint->m_gradientTriggerObjectArray) {
		reinterpret_cast<PACCArray*>(m_checkpoint->m_gradientTriggerObjectArray)
			->describe<GradientTriggerObject>();
	}
	log::info(
		"[PersistentCheckpoint - describe] m_unk11e8: {}", m_checkpoint->m_unk11e8
	);
	l_size = m_checkpoint->m_sequenceTriggerStateUnorderedMap.size();
	log::info(
		"[PersistentCheckpoint - describe] "
		"m_sequenceTriggerStateUnorderedMap.size(): {}",
		l_size
	);
	int i = 0;
	for (gd::pair<int, SequenceTriggerState> l_pair :
		  m_checkpoint->m_sequenceTriggerStateUnorderedMap) {
		log::info(
			"[PersistentCheckpoint - describe] m_sequenceTriggerStateUnorderedMap "
			"element {} key: {}",
			i, l_pair.first
		);
		log::info(
			"[PersistentCheckpoint - describe] m_sequenceTriggerStateUnorderedMap "
			"element {} value:",
			i
		);
		reinterpret_cast<PASequenceTriggerState*>(&l_pair.second)->describe();
		i++;
	}
	log::info(
		"[PersistentCheckpoint - describe] m_commandIndex: {}",
		m_checkpoint->m_commandIndex
	);

	// Custom Data

	log::info("[PersistentCheckpoint - describe] m_objectPos: {}", m_objectPos);
	log::info("[PersistentCheckpoint - describe] m_time: {}", m_time);
	log::info("[PersistentCheckpoint - describe] m_percent: {}", m_percent);
	log::info(
		"[PersistentCheckpoint - describe] m_persistentItemCountMap: {}",
		m_persistentItemCountMap
	);
	log::info(
		"[PersistentCheckpoint - describe] m_persistentTimerItemSet: {}",
		m_persistentTimerItemSet
	);
}
#endif

StartPosGameModes
PersistentCheckpoint::getGamemodeFromCheckpoint(PlayerCheckpoint* checkpoint) {
	return checkpoint->m_isShip	  ? StartPosGameModes::Ship
			 : checkpoint->m_isBall	  ? StartPosGameModes::Ball
			 : checkpoint->m_isBird	  ? StartPosGameModes::UFO
			 : checkpoint->m_isDart	  ? StartPosGameModes::Wave
			 : checkpoint->m_isRobot  ? StartPosGameModes::Robot
			 : checkpoint->m_isSpider ? StartPosGameModes::Spider
			 : checkpoint->m_isSwing  ? StartPosGameModes::Swing
											  : StartPosGameModes::Normal;
}
