#include "UIPOptionsLayer.hpp"
#include "UILayer.hpp"

#include <Geode/binding/PlayLayer.hpp>

bool ModUIPOptionsLayer::init() {
	if (!UIPOptionsLayer::init())
		return false;

	createCheckpointCreateButton(m_practiceNode->getChildByIndex(0), nullptr);
	createCheckpointRemoveButton(m_practiceNode->getChildByIndex(1), nullptr);

	m_fields->m_switcherMenu = SwitcherMenu::createWithTouch(
		nullptr,
		[this](CCTouch* touch, CCEvent* event) {
			CCPoint touchPos =
				m_fields->m_switcherMenu->convertTouchToNodeSpace(touch);
			float w = m_fields->m_switcherMenu->getContentWidth();
			float h = m_fields->m_switcherMenu->getContentHeight();
			if (touchPos.x >= 0 && touchPos.x < w && touchPos.y >= 0 &&
				 touchPos.y < h) {
				m_fields->m_movingSwitcher = true;
				m_fields->m_lastPos = touch->getLocation();
				return true;
			}
			return false;
		},
		[this](CCTouch* touch, CCEvent* event) {
			CCPoint touchPos = touch->getLocation();
			CCPoint delta = touchPos - m_fields->m_lastPos;
			m_fields->m_switcherMenu->setPosition(
				m_fields->m_switcherMenu->getPosition() + delta
			);

			m_fields->m_lastPos = touchPos;
		},
		[this](CCTouch* touch, CCEvent* event) {
			m_fields->m_movingSwitcher = false;
			saveSwitcherPosition();
		},
		[this](CCTouch* touch, CCEvent* event) {
			m_fields->m_movingSwitcher = false;
		}
	);

	m_fields->m_switcherMenu->setTouchPriority(-700);

	m_mainLayer->addChild(m_fields->m_switcherMenu);

	return true;
}

void ModUIPOptionsLayer::onClose(CCObject* sender) {
	saveSwitcherPosition();

	if (PlayLayer* playLayer = PlayLayer::get())
		static_cast<ModUILayer*>(playLayer->m_uiLayer)
			->m_fields->m_switcherMenu->setPosition(
				m_fields->m_switcherMenu->getPosition()
			);

	UIPOptionsLayer::onClose(sender);
}

void ModUIPOptionsLayer::onReset(CCObject* sender) {
	CCDirector* director = CCDirector::sharedDirector();
	m_fields->m_switcherMenu->setPosition(ccp(
		director->getScreenLeft() + SWITCHER_OFFSET_X,
		director->getScreenTop() - SWITCHER_OFFSET_Y
	));
	saveSwitcherPosition();
	UIPOptionsLayer::onReset(sender);
}

void ModUIPOptionsLayer::saveSwitcherPosition() {
	Mod* mod = Mod::get();
	auto switcherPosition = m_fields->m_switcherMenu->getPosition();
	mod->setSavedValue("switcherMenuPositionX", switcherPosition.x);
	mod->setSavedValue("switcherMenuPositionY", switcherPosition.y);
}
