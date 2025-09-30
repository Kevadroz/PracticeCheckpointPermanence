#include "UIPOptionsLayer.hpp"
#include "UILayer.hpp"

#define SWITCHER_SCALE_SLIDER_MIN .25f
#define SWITCHER_SCALE_SLIDER_MAX 3.0f

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
			saveSwitcherSettings();
		},
		[this](CCTouch* touch, CCEvent* event) {
			m_fields->m_movingSwitcher = false;
		}
	);

	m_fields->m_switcherMenu->setTouchPriority(-700);

	CCPoint screenCenter = CCDirector::get()->getWinSize() / 2;

	CCLabelBMFont* switcherScaleLabel =
		CCLabelBMFont::create("Switcher Scale:", "chatFont.fnt");
	switcherScaleLabel->setScale(0.8f);
	switcherScaleLabel->setAnchorPoint(ccp(1, .5f));

	m_fields->m_switcherScaleSlider = Slider::create(
		this, menu_selector(ModUIPOptionsLayer::onSwitcherScaleSliderUpdated),
		0.3f
	);
	m_fields->m_switcherScaleSlider->setAnchorPoint(ccp(0, 0));
	CCNode* switcherScaleSliderSprite =
		m_fields->m_switcherScaleSlider->getChildByIndex(0);
	switcherScaleSliderSprite->setScaleY(.64f);
	switcherScaleSliderSprite->getChildByIndex(0)->setVisible(false);
	m_fields->m_switcherScaleSlider->getChildByIndex(1)
		->getChildByIndex(0)
		->setScale(.64f);
	updateSwitcherScaleSlider(m_fields->m_switcherMenu->getScaleX());

	
	m_fields->m_switcherScaleInput = TextInput::create(54, "Num");
	m_fields->m_switcherScaleInput->setCommonFilter(CommonFilter::Float);
	m_fields->m_switcherScaleInput->setMaxCharCount(7);
	m_fields->m_switcherScaleInput->setString(
		fmt::format("{:.2f}", m_fields->m_switcherMenu->getScaleX())
	);
	m_fields->m_switcherScaleInput->setScale(.8f);
	m_fields->m_switcherScaleInput->setCallback([this](std::string value) {
		float scale;
		try {
			scale = std::stof(value);
		} catch (std::invalid_argument) {
			scale = SWITCHER_SCALE;
		};

		m_fields->m_switcherMenu->setScale(scale);
		updateSwitcherScaleSlider(scale);
		saveSwitcherSettings();
	});

	m_fields->m_switcherScaleInput->getChildByIndex(0)->setContentSize(
		ccp(112 / .8f, 48 / .8f)
	);

	switcherScaleLabel->setPosition(screenCenter + ccp(-56, -30));
	m_fields->m_switcherScaleInput->setPosition(screenCenter + ccp(-20, -30));
	m_fields->m_switcherScaleSlider->setPosition(screenCenter + ccp(60, -30));

	m_mainLayer->addChild(m_fields->m_switcherMenu);
	m_mainLayer->addChild(switcherScaleLabel);
	m_mainLayer->addChild(m_fields->m_switcherScaleInput);
	m_mainLayer->addChild(m_fields->m_switcherScaleSlider);

	return true;
}

void ModUIPOptionsLayer::onClose(CCObject* sender) {
	saveSwitcherSettings();

	if (PlayLayer* playLayer = PlayLayer::get()) {
		SwitcherMenu* playSwitcher =
			static_cast<ModUILayer*>(playLayer->m_uiLayer)
				->m_fields->m_switcherMenu;

		playSwitcher->setPosition(m_fields->m_switcherMenu->getPosition());
		playSwitcher->setScale(m_fields->m_switcherMenu->getScaleX());
	}

	UIPOptionsLayer::onClose(sender);
}

void ModUIPOptionsLayer::onReset(CCObject* sender) {
	CCDirector* director = CCDirector::sharedDirector();
	m_fields->m_switcherMenu->setPosition(ccp(
		director->getScreenLeft() + SWITCHER_OFFSET_X,
		director->getScreenTop() - SWITCHER_OFFSET_Y
	));
	m_fields->m_switcherMenu->setScale(SWITCHER_SCALE);
	saveSwitcherSettings();
	UIPOptionsLayer::onReset(sender);
}

void ModUIPOptionsLayer::updateSwitcherScaleSlider(float scale) {
	m_fields->m_switcherScaleSlider->setValue(
		std::lerp(
			.265265265f, .734734735f,
			inverseLerp(
				SWITCHER_SCALE_SLIDER_MIN, SWITCHER_SCALE_SLIDER_MAX, scale
			)
		)
	);
}

void ModUIPOptionsLayer::saveSwitcherSettings() {
	Mod* mod = Mod::get();
	auto switcherPosition = m_fields->m_switcherMenu->getPosition();
	mod->setSavedValue("switcherMenuPositionX", switcherPosition.x);
	mod->setSavedValue("switcherMenuPositionY", switcherPosition.y);
	mod->setSavedValue(
		"switcherMenuScale", m_fields->m_switcherMenu->getScaleX()
	);
}

void ModUIPOptionsLayer::onSwitcherScaleSliderUpdated(CCObject* object) {
	float scale = std::lerp(
		SWITCHER_SCALE_SLIDER_MIN, SWITCHER_SCALE_SLIDER_MAX,
		inverseLerp(.265265265f, .734734735f, m_fields->m_switcherScaleSlider->getValue())
	);

	m_fields->m_switcherMenu->setScale(scale);
	m_fields->m_switcherScaleInput->setString(fmt::format("{:.2f}", scale));
	saveSwitcherSettings();
}

float inverseLerp(float a, float b, float v) { return (v - a) / (b - a); }
