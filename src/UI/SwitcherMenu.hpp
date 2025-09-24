#pragma once
#include "../Hooks/PlayLayer.hpp"

using namespace geode::prelude;

class SwitcherMenu : public CCMenu {
public:
	CCLabelBMFont* m_checkpointLabel = nullptr;
	CCSprite* m_checkpointSprite = nullptr;
	CCLabelBMFont* m_layerLabel = nullptr;

	// If playLayer is nullptr then it will be
	// assumed that this is for the UIPOptionsLayer
	static SwitcherMenu* create(ModPlayLayer* playLayer);
	static SwitcherMenu* createWithTouch(
		ModPlayLayer* playLayer,
		std::function<bool(CCTouch* touch, CCEvent* event)> touchBeganCallback,
		std::function<void(CCTouch* touch, CCEvent* event)> touchMovedCallback,
		std::function<void(CCTouch* touch, CCEvent* event)> touchEndedCallback,
		std::function<void(CCTouch* touch, CCEvent* event)> touchCancelledCallback
	);

private:
	std::function<bool(CCTouch* touch, CCEvent* event)> m_touchBeganCallback = nullptr;
	std::function<void(CCTouch* touch, CCEvent* event)> m_touchMovedCallback = nullptr;
	std::function<void(CCTouch* touch, CCEvent* event)> m_touchEndedCallback = nullptr;
	std::function<void(CCTouch* touch, CCEvent* event)> m_touchCancelledCallback = nullptr;

	bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
	void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
	void ccTouchEnded(CCTouch* touch, CCEvent* event) override;
	void ccTouchCancelled(CCTouch* touch, CCEvent* event) override;
};

CCPoint getSwitcherPosition(CCDirector* director);
