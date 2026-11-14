#include "PersistentCheckpoint.hpp"

#include <Geode/binding/CheckpointObject.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PlayerCheckpoint.hpp>

void serializeCCPoint(std::ofstream& out, CCPoint p) {
	out << p.x;
	out << p.y;
}

CCPoint deserializeCCPoint(std::ifstream& in) {
	CCPoint p;

	in >> p.x;
	in >> p.y;

	return p;
}

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

	m_time = playLayer->m_timePlayed;
	m_percent = playLayer->getCurrentPercent();
	m_persistentItemCountMap =
		playLayer->m_effectManager->m_persistentItemCountMap;
	m_persistentTimerItemSet =
		playLayer->m_effectManager->m_persistentTimerItemSet;

	// Fallback
	PlayerCheckpoint* p1Checkpoint = m_checkpoint->m_player1Checkpoint;
	PlayerCheckpoint* p2Checkpoint = m_checkpoint->m_player2Checkpoint;

	bool isDual =
		p2Checkpoint != nullptr && m_checkpoint->m_gameState.m_isDualMode;

	StartPosObject* startPos = StartPosObject::create();
	LevelSettingsObject* spSettings = startPos->m_startSettings;

	spSettings->m_startMode =
		static_cast<int>(getGamemodeFromCheckpoint(p1Checkpoint));

	spSettings->m_startSpeed =
		p1Checkpoint->m_playerSpeed < 0.8f	 ? Speed::Slow
		: p1Checkpoint->m_playerSpeed > 1.0f ? Speed::Fast
		: p1Checkpoint->m_playerSpeed > 1.2f ? Speed::Faster
		: p1Checkpoint->m_playerSpeed > 1.4f ? Speed::Fastest
														 : Speed::Normal;

	spSettings->m_startMini = p1Checkpoint->m_isMini;
	spSettings->m_startDual = isDual;
	spSettings->m_mirrorMode =
		m_checkpoint->m_gameState.m_unkBool10; // Mirror Mode
	spSettings->m_rotateGameplay = p1Checkpoint->m_isSideways;
	spSettings->m_isFlipped = p1Checkpoint->m_isUpsideDown;
	spSettings->m_reverseGameplay = p1Checkpoint->m_isGoingLeft;

	// Space reserved, but I don't think that this is possible.
	spSettings->m_targetOrder = 0;

	spSettings->m_targetChannel = m_checkpoint->m_gameState.m_currentChannel;

	// Custom Data
	m_fallbackData.p1Velocity =
		ccp(p1Checkpoint->m_platformerXVelocity, p1Checkpoint->m_yVelocity);

	if (isDual) {
		m_fallbackData.p2Velocity =
			ccp(p2Checkpoint->m_platformerXVelocity, p2Checkpoint->m_yVelocity);
		m_fallbackData.p2Position = p2Checkpoint->m_position;
		m_fallbackData.p2Gamemode = getGamemodeFromCheckpoint(p2Checkpoint);
		m_fallbackData.p2IsMini = p2Checkpoint->m_isMini;
		m_fallbackData.p2IsFlipped = p2Checkpoint->m_isUpsideDown;
		m_fallbackData.p2IsReverseGameplay = p2Checkpoint->m_isGoingLeft;
	}

	m_fallbackData.freeMode =
		m_checkpoint->m_gameState.m_isFreeMode; // Free Mode
	m_fallbackData.cameraPosition = m_checkpoint->m_gameState.m_cameraPosition;
	m_fallbackData.cameraOffset = m_checkpoint->m_gameState.m_cameraOffset;
	m_fallbackData.cameraZoom = m_checkpoint->m_gameState.m_cameraZoom;
	// Custom Data end

	startPos->setSettings(spSettings);
	startPos->setStartPos(m_objectPos);

	m_fallbackData.startPos = startPos;

	if (checkpoint->m_physicalCheckpointObject != nullptr) {
		m_objectPos = checkpoint->m_physicalCheckpointObject->m_startPosition;

		setupPhysicalObject();
	}
}

void PersistentCheckpoint::serialize(OutStream& out) {
	std::string checkData = "";
	if (m_isDegraded) {
		out << checkData;
		return;
	}

	checkData = serializeCheckpoint(m_checkpoint);
	out << checkData;

	// Custom data
	out << m_persistentItemCountMap;
	out << m_persistentTimerItemSet;
}

void PersistentCheckpoint::deserialize(
	InStream& in, SaveHeader header, GJBaseGameLayer* layer
) {
	std::string checkData;
	in >> checkData;
	if (checkData.empty() || m_isDegraded) {
		m_isDegraded = true;
		return;
	}

	deserializeCheckpoint(m_checkpoint, checkData, layer);

	// Custom data
	in >> m_persistentItemCountMap;
	in >> m_persistentTimerItemSet;
}

void PersistentCheckpoint::serializeExternal(OutStream& out) {
	out << m_objectPos;
	out << m_time;
	out << m_percent;

	out << m_name;

	// Fallback
	LevelSettingsObject* settings = m_fallbackData.startPos->m_startSettings;

	int speed = (int)settings->m_startSpeed;

	out << settings->m_startMode;
	out << speed;

	out << settings->m_startMini;
	out << settings->m_startDual;
	out << settings->m_mirrorMode;
	out << settings->m_rotateGameplay;
	out << settings->m_isFlipped;
	out << settings->m_reverseGameplay;

	// Space reserved, but I don't think that this is possible.
	out << settings->m_targetOrder;

	out << m_checkpoint->m_gameState.m_currentChannel;

	// Custom Data
	out << m_fallbackData.p1Velocity;

	if (settings->m_startDual) {
		int p2Gamemode = (int)m_fallbackData.p2Gamemode;

		out << m_fallbackData.p2Velocity;
		out << m_fallbackData.p2Position;
		out << p2Gamemode;
		out << m_fallbackData.p2IsMini;
		out << m_fallbackData.p2IsFlipped;
		out << m_fallbackData.p2IsReverseGameplay;
	}

	out << m_fallbackData.freeMode;
	out << m_fallbackData.cameraPosition;
	out << m_fallbackData.cameraOffset;
	out << m_fallbackData.cameraZoom;
}

void PersistentCheckpoint::deserializeExternal(
	InStream& in, SaveHeader header
) {
	in >> m_objectPos;
	in >> m_time;
	in >> m_percent;

	in >> m_name;

	// Fallback
	StartPosObject* startPos = StartPosObject::create();

	LevelSettingsObject* settings = startPos->m_startSettings;
	settings->m_startsWithStartPos = true;

	int speed;

	in >> settings->m_startMode;

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
	in >> m_fallbackData.cameraPosition;
	in >> m_fallbackData.cameraOffset;
	in >> m_fallbackData.cameraZoom;
	// Custom Data end

	startPos->setSettings(settings);
	startPos->setStartPos(m_objectPos);

	m_fallbackData.startPos = startPos;
}

void PersistentCheckpoint::setupPhysicalObject() {
	const char* frameName = m_isDegraded ? "inactiveDegradedCheckpoint.png"_spr
													 : "inactiveCheckpoint.png"_spr;
	m_physicalObject = GameObject::createWithFrame(frameName);

	m_physicalObject->setOpacity(
		Mod::get()->getSettingValue<double>("inactive-checkpoint-opacity") * 255
	);
	m_physicalObject->m_objectID = 0x2c;
	m_physicalObject->m_objectType = GameObjectType::Decoration;
	m_physicalObject->m_glowSprite = nullptr;

	m_physicalObject->setStartPos(m_objectPos);
	// m_physicalObject->setStartPos(m_fallbackData.startPos->m_startPosition);
}

void PersistentCheckpoint::toggleActive(bool active, bool isGhost) {
	const char* frameName;
	if (active && !isGhost)
		frameName = m_isDegraded ? "activeDegradedCheckpoint.png"_spr
										 : "activeCheckpoint.png"_spr;
	else
		frameName = m_isDegraded ? "inactiveDegradedCheckpoint.png"_spr
										 : "inactiveCheckpoint.png"_spr;
	double opacity;
	if (isGhost && active)
		opacity = 1;
	else {
		const char* settingKey =
			active ? "active-checkpoint-opacity" : "inactive-checkpoint-opacity";
		opacity = Mod::get()->getSettingValue<double>(settingKey);
	}

	m_physicalObject->setDisplayFrame(
		CCSpriteFrameCache::get()->spriteFrameByName(frameName)
	);
	m_physicalObject->setOpacity(opacity * 255);
}

std::string PersistentCheckpoint::getDefaultLabel(bool isPlatformer) {
	std::string label;
	if (isPlatformer) {
		int time = m_time;

		label = fmt::format("{}s", time % 60);

		if (time >= 60) {
			label = fmt::format("{}m", (time % 3600) / 60) + label;

			if (time >= 3600)
				label = fmt::format("{}h", time / 3600) + label;
		}
	} else {
		int decimals =
			Mod::get()->getSettingValue<int64_t>("percentage-display-decimals");
		label = fmt::format("{:.{}f}%", (float)m_percent, decimals);
	}

	return label;
}

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
