#include "UIPOptionsLayer.hpp"
#include "UILayer.hpp"

#include <Geode/loader/Dispatch.hpp>

#define SWITCHER_SCALE_SLIDER_MIN .25f
#define SWITCHER_SCALE_SLIDER_MAX 3.0f

bool ModUIPOptionsLayer::init() {
	if (!UIPOptionsLayer::init())
		return false;

	createCheckpointCreateButton(m_practiceNode->getChildByIndex(0), nullptr);
	createCheckpointRemoveButton(m_practiceNode->getChildByIndex(1), nullptr);

	CCPoint screenCenter = CCDirector::get()->getWinSize() / 2;
	// better plat prac ui loaded
	auto pui = Loader::get()->getLoadedMod("naxrin.better_pui");

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
			float scale = m_fields->m_switcherMenu->getParent()->getScale();
			CCPoint delta = touchPos - m_fields->m_lastPos;
			CCPoint destPos = m_fields->m_switcherMenu->getPosition() + delta / scale;
			m_fields->m_switcherMenu->setPosition(destPos);
			// post event to bpui
			DispatchEvent("naxrin.better_pui", destPos).post();
			m_fields->m_lastPos = touchPos;

		},
		[this, pui, screenCenter](CCTouch* touch, CCEvent* event) {
			m_fields->m_movingSwitcher = false;
			// grid snap from bpui
			if (pui && pui->getSavedValue<bool>("snap")) {
				auto t = m_mainLayer->getChildByID("map")->convertTouchToNodeSpace(touch);

				int hd = pui->getSettingValue<int64_t>("hori-distance");
				int vd = pui->getSettingValue<int64_t>("vert-distance");

				auto dest = ccp(screenCenter.x + hd * round((t.x - screenCenter.x) / hd),
					screenCenter.y + vd * round((t.y - screenCenter.y) / vd));
				m_fields->m_switcherMenu->runAction(CCEaseExponentialOut::create(CCMoveTo::create(0.4, dest)));
				this->saveSwitcherPosition(dest);
			}
			
		},
		[this](CCTouch* touch, CCEvent* event) {
			m_fields->m_movingSwitcher = false;
		}
	);

	m_fields->m_switcherMenu->setTouchPriority(-700);

	CCLabelBMFont* switcherScaleLabel =
		CCLabelBMFont::create("Switcher Scale:", "chatFont.fnt");
	switcherScaleLabel->setID("switcher_scale_label"_spr);

	if (pui) {
		m_fields->m_switcherScaleSlider = Slider::create(
			this, menu_selector(ModUIPOptionsLayer::onSwitcherScaleSliderUpdated)
		);	
	} else {
		m_fields->m_switcherScaleSlider = Slider::create(
			this, menu_selector(ModUIPOptionsLayer::onSwitcherScaleSliderUpdated),
			0.3f
		);	
		CCNode* switcherScaleSliderSprite =
			m_fields->m_switcherScaleSlider->getChildByIndex(0);
		switcherScaleSliderSprite->setScaleY(.64f);
		switcherScaleSliderSprite->getChildByIndex(0)->setVisible(false);
		m_fields->m_switcherScaleSlider->getChildByIndex(1)
			->getChildByIndex(0)
			->setScale(.64f);			
	}

	m_fields->m_switcherScaleSlider->setAnchorPoint(ccp(0, 0));

	updateSwitcherScaleSlider(m_fields->m_switcherMenu->getScaleX());
	m_fields->m_switcherScaleSlider->setID("switcher_scale_slider"_spr);

	m_fields->m_switcherScaleInput = TextInput::create(54, "Num");
	m_fields->m_switcherScaleInput->setCommonFilter(CommonFilter::Float);
	m_fields->m_switcherScaleInput->setMaxCharCount(7);
	m_fields->m_switcherScaleInput->setString(
		fmt::format("{:.2f}", m_fields->m_switcherMenu->getScaleX())
	);

	m_fields->m_switcherScaleInput->setCallback([this](std::string value) {
		float scale;
		// recommend
		//scale = numFromString<int>(value).unwrapOr(SWITCHER_SCALE);
		try {
			scale = std::stof(value);
		} catch (std::invalid_argument) {
			scale = SWITCHER_SCALE;
		};

		m_fields->m_switcherMenu->setScale(scale);
		updateSwitcherScaleSlider(scale);
		Mod::get()->setSavedValue("switcherMenuScale", scale);
	});
	m_fields->m_switcherScaleInput->setID("switcher_scale_input"_spr);

	if (pui) {
		// latest version (compatible update)
		if (auto map = m_mainLayer->getChildByID("map"))
			map->addChild(m_fields->m_switcherMenu);
		// former version
		else
			m_practiceNode->getParent()->addChild(m_fields->m_switcherMenu);

		auto menu = CCMenu::create();
		menu->setPosition(ccp(screenCenter.x * 3, screenCenter.y / 2 - 50.f));
		menu->setScale(0.f);
		menu->setContentSize(ccp(0.f, 0.f));
		menu->setID("switcher-scale");
		m_mainLayer->addChild(menu);

		menu->runAction(CCEaseExponentialOut::create(CCMoveTo::create(0.3,
			ccp(screenCenter.x * 3, screenCenter.y / 2 - 20.f))));
		menu->runAction(CCEaseExponentialOut::create(CCScaleTo::create(0.3, 1)));

		switcherScaleLabel->setPosition(ccp(-120.f, 0.f));
		switcherScaleLabel->setColor(pui->getSettingValue<ccColor3B>("ui-color"));
		menu->addChild(switcherScaleLabel);

		m_fields->m_switcherScaleInput->setPosition(ccp(-30.f, 0.f));
		m_fields->m_switcherScaleInput->setScale(0.9f);
		menu->addChild(m_fields->m_switcherScaleInput);

		m_fields->m_switcherScaleSlider->setPosition(ccp(80.f, 0.f));
		m_fields->m_switcherScaleSlider->setScale(0.6f);
		//m_fields->m_switcherScaleSlider->m_groove->setScale(1.f);
		menu->addChild(m_fields->m_switcherScaleSlider);
	} else {
		switcherScaleLabel->setPosition(screenCenter + ccp(-56, -30));
		m_fields->m_switcherScaleInput->setPosition(screenCenter + ccp(-20, -30));
		m_fields->m_switcherScaleSlider->setPosition(screenCenter + ccp(60, -30));

		switcherScaleLabel->setScale(0.8f);
		switcherScaleLabel->setAnchorPoint(ccp(1, .5f));
		m_fields->m_switcherScaleInput->setScale(.8f);
		m_fields->m_switcherScaleInput->getChildByIndex(0)->setContentSize(
			ccp(112 / .8f, 48 / .8f)
		);

		m_mainLayer->addChild(m_fields->m_switcherMenu);
		m_mainLayer->addChild(switcherScaleLabel);
		m_mainLayer->addChild(m_fields->m_switcherScaleInput);
		m_mainLayer->addChild(m_fields->m_switcherScaleSlider);
	}

	return true;
}

void ModUIPOptionsLayer::onClose(CCObject* sender) {
	// actually not needed
	//saveSwitcherSettings();

	if (PlayLayer* playLayer = PlayLayer::get()) {
		ModUILayer* uiLayer = static_cast<ModUILayer*>(playLayer->m_uiLayer);
		SwitcherMenu* playSwitcher = uiLayer->m_fields->m_switcherMenu;

		playSwitcher->setPosition(m_fields->m_switcherMenu->getPosition());
		playSwitcher->setScale(m_fields->m_switcherMenu->getScaleX());

		GLubyte opacity = GameManager::get()->m_practiceOpacity * 255;
		uiLayer->m_fields->m_createCheckpointButton->setOpacity(opacity);
		uiLayer->m_fields->m_removeCheckpointButton->setOpacity(opacity);
	}

	UIPOptionsLayer::onClose(sender);
}

void ModUIPOptionsLayer::onReset(CCObject* sender) {
	// unused if bpui loaded
	if (Loader::get()->isModLoaded("naxrin.better_pui"))
		return;

	CCDirector* director = CCDirector::sharedDirector();
	CCPoint oriPos = ccp(
		director->getScreenLeft() + SWITCHER_OFFSET_X,
		director->getScreenTop() - SWITCHER_OFFSET_Y
	);
	m_fields->m_switcherMenu->setPosition(oriPos);
	m_fields->m_switcherMenu->setScale(SWITCHER_SCALE);

	saveSwitcherPosition(oriPos);
	Mod::get()->setSavedValue("switcherMenuScale", SWITCHER_SCALE);
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

void ModUIPOptionsLayer::saveSwitcherPosition() {
	this->saveSwitcherPosition(m_fields->m_switcherMenu->getPosition());
}

void ModUIPOptionsLayer::saveSwitcherPosition(CCPoint const& pos) {
	Mod* mod = Mod::get();
	mod->setSavedValue("switcherMenuPositionX", pos.x);
	mod->setSavedValue("switcherMenuPositionY", pos.y);
}

void ModUIPOptionsLayer::onSwitcherScaleSliderUpdated(CCObject* object) {
	float scale = std::lerp(
		SWITCHER_SCALE_SLIDER_MIN, SWITCHER_SCALE_SLIDER_MAX,
		inverseLerp(
			.265265265f, .734734735f, m_fields->m_switcherScaleSlider->getValue()
		)
	);

	m_fields->m_switcherMenu->setScale(scale);
	m_fields->m_switcherScaleInput->setString(fmt::format("{:.2f}", scale));
	Mod::get()->setSavedValue("switcherMenuScale", scale);
}

float inverseLerp(float a, float b, float v) { return (v - a) / (b - a); }
