#pragma once
#include "../PersistentCheckpoint.hpp"
#include "sabe.persistenceapi/include/util/Stream.hpp"

#include <Geode/modify/PlayLayer.hpp>
#include <functional>
#include <optional>
#include <variant>
#include <sabe.persistenceapi/include/PersistenceAPI.hpp>
#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>
#endif

using namespace geode::prelude;
using namespace persistenceAPI;
#ifndef GEODE_IS_IOS
using namespace keybinds;
#endif

// Copied from PlatformerSaves
#if defined(GEODE_IS_WINDOWS)
#define UNIQUE_ID_OFFSET 0x69c158
#define PLATFORM 0
#elif defined(GEODE_IS_ANDROID64)
#define UNIQUE_ID_OFFSET 0x11fe018
#define PLATFORM 1
#elif defined(GEODE_IS_ANDROID32)
#define UNIQUE_ID_OFFSET 0xa9f00c
#define PLATFORM 2
#elif defined(GEODE_IS_ARM_MAC)
#define UNIQUE_ID_OFFSET 0x8aa39c
#define PLATFORM 3
#elif defined(GEODE_IS_INTEL_MAC)
#define UNIQUE_ID_OFFSET 0x985500
#define PLATFORM 4
#elif defined(GEODE_IS_IOS)
#define UNIQUE_ID_OFFSET 0x83f2e8
#define PLATFORM 5
#endif

const std::hash<std::string> c_stringHasher;

enum LoadError : char {
	None,
	Crash,
	OutdatedData,
	NewData,
	OtherPlatform,
	LevelVersionMismatch,
};

class $modify(ModPlayLayer, PlayLayer) {
	struct Fields {
		bool m_startedLoadingObjects = false;
		LoadError m_loadError = LoadError::None;
		bool m_hasAttemptedToLoadCheckpoints = false;

		Ref<cocos2d::CCArray> m_persistentCheckpointArray = nullptr;
		Ref<cocos2d::CCSpriteBatchNode> m_persistentCheckpointBatchNode = nullptr;

		unsigned int m_activeCheckpoint = 0;
		unsigned int m_activeSaveLayer = 0;
		unsigned int m_saveLayerCount = 0;

		std::optional<size_t> m_levelStringHash;

		CCNodeRGBA* m_pbCheckpointContainer = nullptr;
	};

	// Hooks
	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects);
	void setupHasCompleted();
	void destructor();

	void processCreateObjectsFromSetup();

	void resetLevel();
	void loadFromCheckpoint(CheckpointObject* p0);

	void togglePracticeMode(bool enabled);

	// Custom
	void registerKeybindListeners();
	void updateModUI();

	// Data
	void serializeCheckpoints();
	void deserializeCheckpoints();
	void unloadPersistentCheckpoints();
	std::variant<unsigned int, LoadError>
	verifySaveStream(persistenceAPI::Stream& stream);
	std::variant<unsigned int, LoadError>
	verifySavePath(std::filesystem::path path);
	std::filesystem::path getSavePath();

	// Checkpoints
	void nextCheckpoint();
	void previousCheckpoint();
	void
	switchCurrentCheckpoint(unsigned int, bool ignoreLastCheckpoint = false);
	void markPersistentCheckpoint();
	void storePersistentCheckpoint(PersistentCheckpoint* checkpoint);
	void removePersistentCheckpoint(PersistentCheckpoint* checkpoint);
	void removeCurrentPersistentCheckpoint();
	void swapPersistentCheckpoints(unsigned int left, unsigned int right);

	// Save Layers
	void nextSaveLayer();
	void previousSaveLayer();
	void switchCurrentSaveLayer(unsigned int);
	void removeCurrentSaveLayer();
	void swapSaveLayers(unsigned int left, unsigned int right);
	void updateSaveLayerCount();

	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("PlayLayer::setupHasCompleted", Priority::VeryLate))
			log::warn("Failed to set PlayLayer::setupHasCompleted hook priority!");
	}
};
