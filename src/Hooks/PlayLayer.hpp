#pragma once
#include "../PersistentCheckpoint.hpp"

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(ModPlayLayer, PlayLayer) {
	struct Fields {
		bool m_startedLoadingObjects = false;
		LoadError m_loadError = LoadError::None;
		bool m_hasAttemptedToLoadCheckpoints = false;

		Ref<CCArray> m_persistentCheckpointArray = nullptr;
		Ref<CCSpriteBatchNode> m_persistentCheckpointBatchNode = nullptr;

		CCArrayExt<GameObject> m_holdedFallbackObjects;

		unsigned int m_activeCheckpoint = 0;
		unsigned int m_activeSaveLayer = 0;
		unsigned int m_saveLayerCount = 0;
		unsigned int m_ghostActiveCheckpoint = 0;

		double m_ghostCheckpointUsedTime = 0;

		std::optional<size_t> m_levelStringHash;

		CCNodeRGBA* m_pbCheckpointContainer = nullptr;
	};

	// Hooks
	bool init(GJGameLevel* level, bool dontCreateObjects);
	void setupHasCompleted();
	void destructor();

	void resetLevel();
	void loadFromCheckpoint(CheckpointObject* p0);

	void togglePracticeMode(bool enabled);
	void handleTogglePracticeMode(bool enabled);

	void storeCheckpoint(CheckpointObject* p0);

	// Custom
	void registerKeybindListeners();
	void updateModUI();
	void updateCheckpointManager(bool fullUpdate = false);
	bool isPersistentSystemActive();
	bool isModUIVisible();
	bool isDegradedCheckpoint();
	bool isFallbackMode();
	bool isUnloadableError();
	bool keybindsAvailable();

	// Data
	void serializeCheckpoints();
	void deserializeCheckpoints(bool ignoreVerification = false);
	void unloadPersistentCheckpoints();

	std::filesystem::path getSavePath();
	static std::filesystem::path
	getSavePath(GJGameLevel* level, bool lowDetail, unsigned int saveLayer);

	// Checkpoints
	void nextCheckpoint();
	void previousCheckpoint();
	void switchCurrentCheckpoint(
		unsigned int nextCheckpoint, bool ignoreLastCheckpoint = false,
		bool noVisualUpdates = false
	);
	void switchGhostCheckpoint(
		unsigned int nextCheckpoint, bool noVisualUpdates = false,
		bool updateUI = true
	);
	void markPersistentCheckpoint();
	unsigned int
	storePersistentCheckpoint(PersistentCheckpoint* checkpoint, bool reorder);
	void removePersistentCheckpoint(PersistentCheckpoint* checkpoint);
	void removeCurrentPersistentCheckpoint();
	void removeGhostPersistentCheckpoint();
	void swapPersistentCheckpoints(unsigned int left, unsigned int right);

	// Save Layers
	void nextSaveLayer();
	void previousSaveLayer();
	void switchCurrentSaveLayer(unsigned int);
	void removeCurrentSaveLayer();
	void swapSaveLayers(unsigned int left, unsigned int right);
	void updateSaveLayerCount();
	static unsigned int getSaveLayerCount(GJGameLevel* level, bool lowDetail);

	static void onModify(auto& self) {
		if (!self.setHookPriorityPost(
				 "PlayLayer::setupHasCompleted", Priority::VeryLate
			 ))
			log::warn("Failed to set PlayLayer::setupHasCompleted hook priority!");
	}
};
