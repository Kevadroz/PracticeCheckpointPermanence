#pragma once
#include "SaveParser.hpp"

#include <Geode/binding/CheckpointObject.hpp>
#include <Geode/modify/CheckpointObject.hpp>

using namespace geode::prelude;

enum class StartPosGameModes : int {
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

	float p1XVelocity;
	CCPoint p2Velocity;
	CCPoint p2Position;
	StartPosGameModes p2Gamemode;
	bool p2IsMini;
	bool p2IsFlipped;
	bool p2IsReverseGameplay;

	bool freeMode;
};

struct CheckpointStyle {
	const char* baseFrame;
	const char* colorFrame;
	ccColor3B color;
};

class PersistentCheckpoint : public CCObject {
public:
	FallbackData m_fallbackData;
	bool m_isDegraded = false;

	Ref<CheckpointObject> m_checkpoint = nullptr;
	Ref<GameObject> m_physicalObject = nullptr;
	Ref<CCSprite> m_physicalObjectColor = nullptr;
	CCPoint m_objectPos;
	double m_time;
	double m_percent;
	std::unordered_map<int, int> m_persistentItemCountMap;
	std::unordered_set<int> m_persistentTimerItemSet;

	std::string m_checkpointName = "";
	float m_color = NAN;

	static PersistentCheckpoint* create();
	void storeData(CheckpointObject* checkpoint, PlayLayer* playLayer);

	void serialize(OutStream& out);
	void deserialize(InStream& in, SaveHeader, GJBaseGameLayer* layer);
	void serializeExternal(OutStream& out);
	void deserializeExternal(InStream& in, SaveHeader);
	void setupPhysicalObject();
	void toggleActive(bool active, bool isGhost = false);
	void refreshStyle(bool active);

	std::string getDefaultLabel(bool isPlatformer);

	CheckpointStyle getStyle(bool active);

	static StartPosGameModes
	getGamemodeFromCheckpoint(PlayerCheckpoint* checkpoint);
};
