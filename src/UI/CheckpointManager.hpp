#pragma once
#include "../Hooks/PlayLayer.hpp"

using namespace geode::prelude;

class CheckpointManager : public Popup<> {
public:
	bool setup() override;
	static CheckpointManager* create();

private:
	ModPlayLayer* m_playLayer = nullptr;
	CCMenuItemSpriteExtra* m_deleteButton = nullptr;
	CCLabelBMFont* m_saveLayerLabel = nullptr;
	CCMenuItemSpriteExtra* m_previousLayerBtn = nullptr;
	CCMenuItemSpriteExtra* m_nextLayerBtn = nullptr;
	CCMenuItemSpriteExtra* m_moveLayerBackBtn = nullptr;
	CCMenuItemSpriteExtra* m_moveLayerForwardBtn = nullptr;
	CCLayerColor* m_listContainer = nullptr;
	CCArray* m_cellsArray = CCArray::create();
	ListView* m_listView = nullptr;
	CCLabelBMFont* m_emptyListLabel = nullptr;

	void createList(bool resetPosition = false);
	void updateUIElements(bool resetListPosition = false);
};

CCNode* createCheckpointCell(
	PersistentCheckpoint* checkpoint,
	std::function<void(CCMenuItemSpriteExtra*)> moveUpCallback,
	std::function<void(CCMenuItemSpriteExtra*)> moveDownCallback,
	std::function<void(CCMenuItemSpriteExtra*)> selectCallback,
	std::function<void(CCMenuItemSpriteExtra*)> removeCallback
);
