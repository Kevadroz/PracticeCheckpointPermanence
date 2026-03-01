#pragma once
#include "SaveParser.hpp"

#include <Geode/binding/CheckpointObject.hpp>
#include <Geode/modify/CheckpointObject.hpp>

#include <sabe.persistenceapi/include/util/Stream.hpp>

using namespace geode::prelude;

enum StartPosGameModes : int {
	Normal = 0,
	Ship = 1,
	Ball = 2,
	UFO = 3,
	Wave = 4,
	Robot = 5,
	Spider = 6,
	Swing = 7,
};

struct FallbackData {
	Ref<StartPosObject> startPos = nullptr;

	CCPoint p1Velocity;
	CCPoint p2Velocity;
	CCPoint p2Position;
	StartPosGameModes p2Gamemode;
	bool p2IsMini;
	bool p2IsFlipped;
	bool p2IsReverseGameplay;

	bool freeMode;
	CCPoint cameraOffset;
	float cameraZoom;
};

class PersistentCheckpoint : public CCObject {
public:
	FallbackData m_fallbackData;

	Ref<CheckpointObject> m_checkpoint = nullptr;
	CCPoint m_objectPos;
	double m_time;
	double m_percent;
	gd::unordered_map<int, int> m_persistentItemCountMap;
	gd::unordered_set<int> m_persistentTimerItemSet;

	static PersistentCheckpoint* create();
	void storeData(CheckpointObject* checkpoint, PlayLayer* playLayer);

	void serialize(persistenceAPI::Stream& out);
	void deserialize(persistenceAPI::Stream& in, SaveHeader);
	void serializeExternal(persistenceAPI::Stream& out);
	void deserializeExternal(persistenceAPI::Stream& in, SaveHeader);
	void setupPhysicalObject();
	void toggleActive(bool);

#if defined(PA_DEBUG) && defined(PA_DESCRIBE)
	void describe();
#endif

	static StartPosGameModes
	getGamemodeFromCheckpoint(PlayerCheckpoint* checkpoint);
};
