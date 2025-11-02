#pragma once
#include <Geode/binding/CheckpointObject.hpp>
#include <Geode/modify/CheckpointObject.hpp>

#include <sabe.persistenceapi/include/util/Stream.hpp>

using namespace geode::prelude;

class PersistentCheckpoint : public CCObject {
public:
	Ref<CheckpointObject> m_checkpoint = nullptr;
	CCPoint m_objectPos;
	int m_attempts;
	double m_time;
	double m_percent;
	gd::unordered_map<int, int> m_persistentItemCountMap;
	gd::unordered_set<int> m_persistentTimerItemSet;

	static PersistentCheckpoint* create();
	static PersistentCheckpoint* createFromCheckpoint(
		CheckpointObject* checkpoint, int attempts, int time, double percent,
		gd::unordered_map<int, int> persistentItemCountMap,
		gd::unordered_set<int> persistentTimerItemSet
	);

	void serialize(persistenceAPI::Stream& out);
	void deserialize(persistenceAPI::Stream& in, unsigned int saveVersion);
	void setupPhysicalObject();
	void toggleActive(bool);
};
