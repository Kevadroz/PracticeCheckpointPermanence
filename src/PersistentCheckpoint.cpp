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

using namespace persistenceAPI;

PersistentCheckpoint::~PersistentCheckpoint() { CC_SAFE_DELETE(m_checkpoint); }

PersistentCheckpoint* PersistentCheckpoint::create() {
	PersistentCheckpoint* checkpoint = new PersistentCheckpoint();
	checkpoint->m_checkpoint = CheckpointObject::create();
	CC_SAFE_RETAIN(checkpoint->m_checkpoint);

	checkpoint->autorelease();

	return checkpoint;
}

PersistentCheckpoint* PersistentCheckpoint::createFromCheckpoint(
	CheckpointObject* checkpoint, int attempts, int time, double percent,
	gd::unordered_map<int, int> persistentItemCountMap,
	gd::unordered_set<int> persistentTimerItemSet
) {
	PersistentCheckpoint* newCheckpoint = new PersistentCheckpoint();

	newCheckpoint->m_checkpoint = checkpoint;
	CC_SAFE_RETAIN(newCheckpoint->m_checkpoint);

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
	reinterpret_cast<PACCNode*>(m_checkpoint)->save(out);

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
	out << m_objectPos;
	out << m_attempts;
	out << m_time;
	out << m_percent;

	out << m_persistentItemCountMap;
	out << m_persistentTimerItemSet;
}

void PersistentCheckpoint::deserialize(Stream& in, unsigned int saveVersion) {
	reinterpret_cast<PACCNode*>(m_checkpoint)->load(in);

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
	CC_SAFE_RETAIN(m_checkpoint->m_player1Checkpoint);
	reinterpret_cast<PAPlayerCheckpoint*>(m_checkpoint->m_player1Checkpoint)
		->load(in);
	if (hasP2) {
		m_checkpoint->m_player2Checkpoint = PlayerCheckpoint::create();
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
		CC_SAFE_RETAIN(m_checkpoint->m_gradientTriggerObjectArray);
		static_cast<PACCArray*>(m_checkpoint->m_gradientTriggerObjectArray)
			->load<GradientTriggerObject>(in);
	}

	// geode::log::debug("gradients {}", hasGradients);

	in >> m_checkpoint->m_unk11e8;
	in >> m_checkpoint->m_sequenceTriggerStateUnorderedMap;
	in >> m_checkpoint->m_commandIndex;

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
	CC_SAFE_RELEASE(m_checkpoint->m_physicalCheckpointObject);
	m_checkpoint->m_physicalCheckpointObject =
		GameObject::createWithFrame("inactiveCheckpoint.png"_spr);
	CC_SAFE_RETAIN(m_checkpoint->m_physicalCheckpointObject);

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
