using namespace geode::prelude;

class ListMenu : public CCMenu {
public:
	static ListMenu* create(CCLayerColor* listContainer);

private:
	CCLayerColor* m_listContainer = nullptr;

	bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
};
