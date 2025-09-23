#include "PersistentCheckpoint.hpp"

#include <Geode/binding/CheckpointObject.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PlayerCheckpoint.hpp>

#include <sabe.persistenceapi/include/hooks/EffectManagerState.hpp>
#include <sabe.persistenceapi/include/hooks/FMODAudioState.hpp>
#include <sabe.persistenceapi/include/hooks/GJGameState.hpp>
#include <sabe.persistenceapi/include/hooks/GJShaderState.hpp>
#include <sabe.persistenceapi/include/hooks/PlayerCheckpoint.hpp>
#include <sabe.persistenceapi/include/hooks/cocos2d/CCArray.hpp>
#include <sabe.persistenceapi/include/hooks/cocos2d/CCNode.hpp>
#include <sabe.persistenceapi/include/util/Stream.hpp>

using namespace geode::prelude;
using namespace persistenceAPI;

PersistentCheckpoint* PersistentCheckpoint::create() {
	PersistentCheckpoint* checkpoint = new PersistentCheckpoint();

	checkpoint->autorelease();

	return checkpoint;
}

PersistentCheckpoint* PersistentCheckpoint::createFromCheckpoint(
	CheckpointObject* checkpoint, int attempts, int time, double percent,
	gd::unordered_map<int, int> persistentItemCountMap,
	gd::unordered_set<int> persistentTimerItemSet
) {
	PersistentCheckpoint* newCheckpoint = new PersistentCheckpoint();

	newCheckpoint->m_gameState = checkpoint->m_gameState;
	newCheckpoint->m_shaderState = checkpoint->m_shaderState;
	newCheckpoint->m_audioState = checkpoint->m_audioState;
	newCheckpoint->m_physicalCheckpointObject =
		checkpoint->m_physicalCheckpointObject;
	newCheckpoint->m_player1Checkpoint = checkpoint->m_player1Checkpoint;
	newCheckpoint->m_player2Checkpoint = checkpoint->m_player2Checkpoint;
	newCheckpoint->m_unke78 = checkpoint->m_unke78;
	newCheckpoint->m_unke7c = checkpoint->m_unke7c;
	newCheckpoint->m_unke80 = checkpoint->m_unke80;
	newCheckpoint->m_ground2Invisible = checkpoint->m_ground2Invisible;
	newCheckpoint->m_streakBlend = checkpoint->m_streakBlend;
	newCheckpoint->m_uniqueID = checkpoint->m_uniqueID;
	newCheckpoint->m_respawnID = checkpoint->m_respawnID;
	newCheckpoint->m_vectorSavedObjectStateRef =
		checkpoint->m_vectorSavedObjectStateRef;
	newCheckpoint->m_vectorActiveSaveObjectState =
		checkpoint->m_vectorActiveSaveObjectState;
	newCheckpoint->m_vectorSpecialSaveObjectState =
		checkpoint->m_vectorSpecialSaveObjectState;
	newCheckpoint->m_effectManagerState = checkpoint->m_effectManagerState;
	newCheckpoint->m_gradientTriggerObjectArray =
		checkpoint->m_gradientTriggerObjectArray;
	newCheckpoint->m_unk11e8 = checkpoint->m_unk11e8;
	newCheckpoint->m_sequenceTriggerStateUnorderedMap =
		checkpoint->m_sequenceTriggerStateUnorderedMap;

	CC_SAFE_RETAIN(newCheckpoint->m_player1Checkpoint);
	CC_SAFE_RETAIN(newCheckpoint->m_player2Checkpoint);
	CC_SAFE_RETAIN(newCheckpoint->m_gradientTriggerObjectArray);

	if (checkpoint->m_physicalCheckpointObject) {
		newCheckpoint->m_objectPos =
			checkpoint->m_physicalCheckpointObject->m_startPosition;

		CC_SAFE_RELEASE(checkpoint->m_physicalCheckpointObject);
		newCheckpoint->createPhysicalObject();
	}

	newCheckpoint->m_attempts = attempts;
	newCheckpoint->m_time = time;
	newCheckpoint->m_percent = percent;
	newCheckpoint->m_persistentItemCountMap = persistentItemCountMap;
	newCheckpoint->m_persistentTimerItemSet = persistentTimerItemSet;

	newCheckpoint->autorelease();

	return newCheckpoint;
}

void PersistentCheckpoint::serialize(Stream& out) {
	reinterpret_cast<PACCNode*>(this)->save(out);

	bool hasP2 = m_player2Checkpoint != nullptr;
	bool hasGradients = m_gradientTriggerObjectArray != nullptr;

	out << hasP2;
	out << hasGradients;

	reinterpret_cast<PAGJGameState*>(&m_gameState)->save(out);
	reinterpret_cast<PAGJShaderState*>(&m_shaderState)->save(out);
	reinterpret_cast<PAFMODAudioState*>(&m_audioState)->save(out);

	reinterpret_cast<PAPlayerCheckpoint*>(m_player1Checkpoint)->save(out);
	if (hasP2)
		reinterpret_cast<PAPlayerCheckpoint*>(m_player2Checkpoint)->save(out);

	out << m_unke78;
	out << m_unke7c;
	out << m_unke80;
	out << m_ground2Invisible;
	out << m_streakBlend;
	out << m_uniqueID;
	out << m_respawnID;
	out << m_vectorSavedObjectStateRef;
	out << m_vectorActiveSaveObjectState;
	out << m_vectorSpecialSaveObjectState;

	reinterpret_cast<PAEffectManagerState*>(&m_effectManagerState)->save(out);
	if (hasGradients)
		static_cast<PACCArray*>(m_gradientTriggerObjectArray)
			->save<GradientTriggerObject>(out);

	out << m_unk11e8;
	out << m_sequenceTriggerStateUnorderedMap;
	out << m_commandIndex;

	// Custom data
	out << m_objectPos;
	out << m_attempts;
	out << m_time;
	out << m_percent;
	out << m_persistentItemCountMap;
	out << m_persistentTimerItemSet;
}

void PersistentCheckpoint::deserialize(Stream& in, unsigned int saveVersion) {
	reinterpret_cast<PACCNode*>(this)->load(in);

	bool hasP2;
	bool hasGradients;

	in >> hasP2;
	in >> hasGradients;

	// geode::log::debug("p2 {}", hasP2);

	reinterpret_cast<PAGJGameState*>(&m_gameState)->load(in);
	// geode::log::debug("gs");
	reinterpret_cast<PAGJShaderState*>(&m_shaderState)->load(in);
	// geode::log::debug("ss");
	reinterpret_cast<PAFMODAudioState*>(&m_audioState)->load(in);
	// geode::log::debug("as");

	m_player1Checkpoint = PlayerCheckpoint::create();
	CC_SAFE_RETAIN(m_player1Checkpoint);
	reinterpret_cast<PAPlayerCheckpoint*>(m_player1Checkpoint)->load(in);
	if (hasP2) {
		m_player2Checkpoint = PlayerCheckpoint::create();
		CC_SAFE_RETAIN(m_player2Checkpoint);
		reinterpret_cast<PAPlayerCheckpoint*>(m_player2Checkpoint)->load(in);
	}

	// geode::log::debug("checkpoints");

	in >> m_unke78;
	in >> m_unke7c;
	in >> m_unke80;
	in >> m_ground2Invisible;
	in >> m_streakBlend;
	in >> m_uniqueID;
	in >> m_respawnID;
	in >> m_vectorSavedObjectStateRef;
	in >> m_vectorActiveSaveObjectState;
	in >> m_vectorSpecialSaveObjectState;

	// geode::log::debug("vars");

	reinterpret_cast<PAEffectManagerState*>(&m_effectManagerState)->load(in);
	if (hasGradients) {
		m_gradientTriggerObjectArray = CCArray::create();
		CC_SAFE_RETAIN(m_gradientTriggerObjectArray);
		static_cast<PACCArray*>(m_gradientTriggerObjectArray)
			->load<GradientTriggerObject>(in);
	}

	// geode::log::debug("gradients {}", hasGradients);

	in >> m_unk11e8;
	in >> m_sequenceTriggerStateUnorderedMap;
	in >> m_commandIndex;

	// Custom data
	in >> m_objectPos;
	in >> m_attempts;
	in >> m_time;
	in >> m_percent;
	in >> m_persistentItemCountMap;
	in >> m_persistentTimerItemSet;

	// geode::log::debug("eof {}", m_commandIndex);
}

void PersistentCheckpoint::createPhysicalObject() {
	CC_SAFE_RELEASE(m_physicalCheckpointObject);
	m_physicalCheckpointObject =
		GameObject::createWithFrame("inactiveCheckpoint.png"_spr);
	CC_SAFE_RETAIN(m_physicalCheckpointObject);

	m_physicalCheckpointObject->setOpacity(
		Mod::get()->getSettingValue<double>("inactive-checkpoint-opacity") * 255
	);
	m_physicalCheckpointObject->m_objectID = 0x2c;
	m_physicalCheckpointObject->m_objectType = GameObjectType::Decoration;
	m_physicalCheckpointObject->m_glowSprite = nullptr;

	m_physicalCheckpointObject->setStartPos(m_objectPos);
}

void PersistentCheckpoint::toggleActive(bool active) {
	const char* frameName;
	if (active) {
		frameName = "activeCheckpoint.png"_spr;
		m_physicalCheckpointObject->setOpacity(255);
	} else {
		frameName = "inactiveCheckpoint.png"_spr;
		m_physicalCheckpointObject->setOpacity(
			Mod::get()->getSettingValue<double>("inactive-checkpoint-opacity") *
			255
		);
	}
	m_physicalCheckpointObject->setDisplayFrame(
		CCSpriteFrameCache::get()->spriteFrameByName(frameName)
	);
}

GameObject* PersistentCheckpoint::getPhysicalObject() {
	return m_physicalCheckpointObject;
}
