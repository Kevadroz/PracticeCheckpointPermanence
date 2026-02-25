#include "ListMenu.hpp"

ListMenu* ListMenu::create(CCLayerColor* listContainer) {
	ListMenu* ret = new ListMenu();
	ret->m_listContainer = listContainer;

	if (ret->init()) {
		ret->autorelease();
		return ret;
	}

	delete ret;
	return nullptr;
}

bool ListMenu::ccTouchBegan(CCTouch* touch, CCEvent* event) {
	cocos2d::CCPoint pos =
		m_listContainer->convertToNodeSpace(touch->getLocation());

	if (pos.x < 0 || pos.y < 0 || pos.x > m_listContainer->getContentWidth() ||
		 pos.y > m_listContainer->getContentHeight())
		return false;

	return CCMenu::ccTouchBegan(touch, event);
}
