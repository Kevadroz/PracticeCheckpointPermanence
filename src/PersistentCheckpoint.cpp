#include "PersistentCheckpoint.hpp"

#include <Geode/binding/CheckpointObject.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PlayerCheckpoint.hpp>

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

	StartPosObject* startPos = playLayer->createLiveStartPosObject();
	LevelSettingsObject* spSettings = startPos->m_startSettings;

	// Space reserved, but I don't think that this is possible.
	spSettings->m_targetOrder = 0;

	// Custom Data
	m_fallbackData.p1XVelocity = p1Checkpoint->m_yVelocity;

	if (isDual) {
		m_fallbackData.p2Velocity =
			ccp(p2Checkpoint->m_platformerXVelocity, p2Checkpoint->m_yVelocity);
		m_fallbackData.p2Position = p2Checkpoint->m_position;
		m_fallbackData.p2Gamemode = getGamemodeFromCheckpoint(p2Checkpoint);
		m_fallbackData.p2IsMini = p2Checkpoint->m_isMini;
		m_fallbackData.p2IsFlipped = p2Checkpoint->m_isUpsideDown;
		m_fallbackData.p2IsReverseGameplay = p2Checkpoint->m_isGoingLeft;
	}
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
	checkData = ZipUtils::compressString(checkData, false, -1, 0xb);
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

	checkData = ZipUtils::decompressString(checkData, false, 0xb);
	deserializeCheckpoint(m_checkpoint, checkData, layer);

	// Custom data
	in >> m_persistentItemCountMap;
	in >> m_persistentTimerItemSet;
}

void PersistentCheckpoint::serializeExternal(OutStream& out) {
	out << m_objectPos;
	out << m_time;
	out << m_percent;

	out << m_checkpointName;
	out << m_color;

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
	CCPoint p1Velocity(m_fallbackData.p1XVelocity, settings->m_yVelocity);
	out << p1Velocity;

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
}

void PersistentCheckpoint::deserializeExternal(
	InStream& in, SaveHeader header
) {
	in >> m_objectPos;
	in >> m_time;
	in >> m_percent;

	in >> m_checkpointName;
	if (header.saveVersion >= 4)
		in >> m_color;

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
	CCPoint p1Velocity;
	in >> p1Velocity;
	m_fallbackData.p1XVelocity = p1Velocity.x;
	settings->m_yVelocity = p1Velocity.y;

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
	if (header.saveVersion < 4) {
		in.ignore(sizeof(CCPoint));
		in.ignore(sizeof(CCPoint));
		in.ignore(sizeof(float));
	}
	// Custom Data end

	startPos->setSettings(settings);
	startPos->setStartPos(m_objectPos);

	m_fallbackData.startPos = startPos;
}

void PersistentCheckpoint::setupPhysicalObject() {
	CheckpointStyle style = getStyle(false);
	m_physicalObject = GameObject::createWithFrame(style.baseFrame);

	m_physicalObject->setCascadeOpacityEnabled(true);
	m_physicalObject->setOpacity(
		Mod::get()->getSettingValue<double>("inactive-checkpoint-opacity") * 255
	);
	m_physicalObject->m_objectID = 0x2c;
	m_physicalObject->m_objectType = GameObjectType::Decoration;
	m_physicalObject->m_glowSprite = nullptr;

	m_physicalObject->setStartPos(m_objectPos);

	m_physicalObjectColor =
		CCSprite::createWithSpriteFrameName(style.colorFrame);
	if (!std::isnan(m_color))
		m_physicalObjectColor->setColor(style.color);
	m_physicalObjectColor->ignoreAnchorPointForPosition(true);
	m_physicalObject->addChild(m_physicalObjectColor);
	m_physicalObjectColor->updateDisplayedOpacity(
		m_physicalObject->getOpacity()
	);
}

void PersistentCheckpoint::toggleActive(bool active, bool isGhost) {
	CheckpointStyle style = getStyle(active && !isGhost);
	double opacity;
	if (isGhost && active)
		opacity = 1;
	else {
		const char* settingKey =
			active ? "active-checkpoint-opacity" : "inactive-checkpoint-opacity";
		opacity = Mod::get()->getSettingValue<double>(settingKey);
	}

	m_physicalObject->setDisplayFrame(
		CCSpriteFrameCache::get()->spriteFrameByName(style.baseFrame)
	);
	m_physicalObjectColor->setDisplayFrame(
		CCSpriteFrameCache::get()->spriteFrameByName(style.colorFrame)
	);
	m_physicalObject->setOpacity(opacity * 255);
}

void PersistentCheckpoint::refreshStyle(bool active) {
	CheckpointStyle style = getStyle(active);

	m_physicalObjectColor->setDisplayFrame(
		CCSpriteFrameCache::get()->spriteFrameByName(style.colorFrame)
	);
	m_physicalObjectColor->setColor(style.color);
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

CheckpointStyle PersistentCheckpoint::getStyle(bool active) {
	bool hasColor = !std::isnan(m_color);

	const char* baseFrame;
	const char* colorFrame;
	if (active)
		if (m_isDegraded) {
			baseFrame = "activeDegradedCheckpoint_base.png"_spr;
			if (hasColor)
				colorFrame = "activeDegradedCheckpoint_color.png"_spr;
			else
				colorFrame = "activeDegradedCheckpoint_default.png"_spr;
		} else {
			baseFrame = "activeCheckpoint_base.png"_spr;
			if (hasColor)
				colorFrame = "activeCheckpoint_color.png"_spr;
			else
				colorFrame = "activeCheckpoint_default.png"_spr;
		}
	else if (m_isDegraded) {
		baseFrame = "inactiveDegradedCheckpoint_base.png"_spr;
		if (hasColor)
			colorFrame = "inactiveDegradedCheckpoint_color.png"_spr;
		else
			colorFrame = "inactiveDegradedCheckpoint_default.png"_spr;
	} else {
		baseFrame = "inactiveCheckpoint_base.png"_spr;
		if (hasColor)
			colorFrame = "inactiveCheckpoint_color.png"_spr;
		else
			colorFrame = "inactiveCheckpoint_default.png"_spr;
	}

	ccColor3B color;
	if (!hasColor)
		color = ccWHITE;
	else {
		RGBA rgba = CCControlUtils::RGBfromHSV(HSV(m_color, 1, 1));
		color = ccColor3B(rgba.r * 255, rgba.g * 255, rgba.b * 255);
	}

	return CheckpointStyle{baseFrame, colorFrame, color};
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
