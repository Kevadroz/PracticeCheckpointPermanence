#include "SaveManager.hpp"
#include "ListMenu.hpp"

SaveManager* SaveManager::create() {
	SaveManager* ret = new SaveManager();
	if (ret->init()) {
		ret->autorelease();
		return ret;
	}

	delete ret;
	return nullptr;
}

bool SaveManager::init() {
	if (!Popup::init(440.f, 260.f, "GJ_square05.png"))
		return false;

	m_noElasticity = true;

	setTitle("PCP Save Manager");

	m_pageLabel = CCLabelBMFont::create("Page 0/0", "bigFont.fnt");
	m_pageLabel->setScale(.6);

	m_totalSizeLabel = CCLabelBMFont::create("Total:\n0B", "bigFont.fnt");
	m_totalSizeLabel->setScale(.35);
	m_totalSizeLabel->setOpacity(160);
	m_totalSizeLabel->setAnchorPoint(ccp(0.0, 0.5));
	m_totalSizeLabel->limitLabelWidth(350.0, .35, 0.1);

	m_levelCountLabel = CCLabelBMFont::create("70 to 80\nof 136", "bigFont.fnt");
	m_levelCountLabel->setScale(.35);
	m_levelCountLabel->setOpacity(160);
	m_levelCountLabel->setAnchorPoint(ccp(1.0, 0.5));
	m_levelCountLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);

	CCSprite* previousPageSpr =
		CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	CCSprite* nextPageSpr =
		CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	nextPageSpr->setFlipX(true);

	m_previousPageBtn = CCMenuItemExt::createSpriteExtra(
		previousPageSpr, [this](CCMenuItemSpriteExtra* sender) { previousPage(); }
	);
	m_nextPageBtn = CCMenuItemExt::createSpriteExtra(
		nextPageSpr, [this](CCMenuItemSpriteExtra* sender) { nextPage(); }
	);
	m_previousPageBtn->m_baseScale = .6;
	m_nextPageBtn->m_baseScale = .6;
	m_previousPageBtn->setScale(.6);
	m_nextPageBtn->setScale(.6);

	m_listContainer = CCLayerColor::create();
	m_listContainer->setContentSize(ccp(400, 170));
	m_listContainer->setAnchorPoint(ccp(.5, 1));
	m_listContainer->setColor(ccc3(27, 27, 27));
	m_listContainer->setOpacity(255);
	m_listContainer->setZOrder(5);

	m_emptyListLabel =
		CCLabelBMFont::create("No saves found", "bigFont.fnt", 215);
	m_emptyListLabel->setScale(.7);
	m_emptyListLabel->setOpacity(220);
	m_emptyListLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);

	m_listContainer->addChildAtPosition(m_emptyListLabel, geode::Anchor::Center);

	m_buttonMenu->addChildAtPosition(
		m_previousPageBtn, geode::Anchor::TopLeft, ccp(25, -50)
	);
	m_buttonMenu->addChildAtPosition(
		m_nextPageBtn, geode::Anchor::TopRight, ccp(-25, -50)
	);
	m_mainLayer->addChildAtPosition(
		m_pageLabel, geode::Anchor::Top, ccp(0, -50)
	);
	m_mainLayer->addChildAtPosition(
		m_totalSizeLabel, geode::Anchor::TopLeft, ccp(25, -20)
	);
	m_mainLayer->addChildAtPosition(
		m_levelCountLabel, geode::Anchor::TopRight, ccp(-10, -20)
	);
	m_mainLayer->addChildAtPosition(
		m_listContainer, geode::Anchor::Top, ccp(0, -73)
	);

	m_saveList = getSaveList();

	createList();

	ListBorders* borders = ListBorders::create();
	borders->setContentSize(m_listView->getContentSize() + ccp(7, 7));
	borders->setZOrder(15);
	borders->setSpriteFrames(
		"darkGrayNegativeBorderTop.png"_spr, "darkGrayNegativeBorderSide.png"_spr
	);
	m_listContainer->addChildAtPosition(borders, geode::Anchor::Center);

	updatePageUI();
	updateGeneralInfoUI();

	return true;
}

void SaveManager::createList(bool resetPosition) {
	bool recreated = m_listView != nullptr;
	bool carryPosition =
		!resetPosition && recreated &&
		m_listView->getChildByIndex(0)->getChildByIndex(0)->getChildrenCount() >
			0;
	float contentPosition;

	if (recreated) {
		m_cellsArray->removeAllObjects();
		m_listView->removeFromParent();

		if (carryPosition)
			contentPosition =
				m_listView->getChildByIndex(0)->getChildByIndex(0)->getPositionY();
	}

	for (unsigned int i = m_page * LEVELS_PER_PAGE;
		  i < m_saveList.size() && m_cellsArray->count() < LEVELS_PER_PAGE; ++i)
		m_cellsArray->addObject(createLevelCell(m_saveList[i]));

	m_listView = ListView::create(
		m_cellsArray, 50, m_listContainer->getContentWidth(),
		m_listContainer->getContentHeight()
	);
	m_listView->setPrimaryCellColor(ccc3(35, 35, 35));
	m_listView->setSecondaryCellColor(ccc3(30, 30, 30));
	m_listView->setZOrder(10);

	CCNode* listContent = m_listView->getChildByIndex(0)->getChildByIndex(0);
	if (carryPosition && m_cellsArray->count() >= 5) {
		listContent->setPositionY(contentPosition);
		if (listContent->getPositionY() > 0)
			listContent->setPositionY(0);
	}

	m_listContainer->addChildAtPosition(m_listView, geode::Anchor::BottomLeft);
}

CCNode* SaveManager::createLevelCell(LevelSaveInfo info) {
	CCMenu* menu = ListMenu::create(m_listContainer);
	menu->setContentSize(ccp(400, 50));

	CCLabelBMFont* nameLabel =
		CCLabelBMFont::create(info.levelName.c_str(), "bigFont.fnt");
	nameLabel->setAnchorPoint(ccp(0, .5));
	nameLabel->setScale(.75);

	std::string fileSizeString = byteSizeAsFormattedString(info.fullFileSize);

	unsigned int totalCheckpointCount = 0;
	for (std::vector<SaveLayerInfo> layers :
		  {info.standardLayers, info.ldmLayers})
		for (SaveLayerInfo layer : info.standardLayers)
			totalCheckpointCount += layer.header.checkpointCount;

	CCMenu* detailMenu = CCMenu::create();
	detailMenu->setAnchorPoint(ccp(0.0, 0.5));
	RowLayout* detailMenuLayout = RowLayout::create();
	detailMenuLayout->setAutoScale(false);
	detailMenuLayout->setAutoGrowAxis(0.0F);
	detailMenuLayout->setGap(15.0F);
	detailMenu->setLayout(detailMenuLayout);

	RowLayout* detailItemLayout = RowLayout::create();
	detailItemLayout->setAutoScale(false);
	detailItemLayout->setAutoGrowAxis(0.0F);
	detailItemLayout->setGap(7.0F);

	CCLabelBMFont* sizeLabel =
		CCLabelBMFont::create(fileSizeString.c_str(), "bigFont.fnt");
	sizeLabel->setAnchorPoint(ccp(0, .5));
	sizeLabel->setScale(.55);

	CCMenu* checkpointsMenu = CCMenu::create();
	checkpointsMenu->setLayout(detailItemLayout);

	CCSprite* checkpointSprite =
		CCSprite::createWithSpriteFrameName("activeCheckpoint.png"_spr);
	checkpointSprite->setScale(.55);

	CCLabelBMFont* checkpointCountLabel = CCLabelBMFont::create(
		numToString(totalCheckpointCount).c_str(), "bigFont.fnt"
	);
	checkpointCountLabel->setScale(.55);

	checkpointsMenu->addChild(checkpointSprite);
	checkpointsMenu->addChild(checkpointCountLabel);
	checkpointsMenu->updateLayout();

	CCMenu* standardLayersMenu = CCMenu::create();
	standardLayersMenu->setLayout(detailItemLayout);

	CCSprite* standardLayersSprite =
		CCSprite::createWithSpriteFrameName("folderIcon_001.png");
	standardLayersSprite->setScale(.55);

	CCLabelBMFont* standardLayersCountLabel = CCLabelBMFont::create(
		numToString(info.standardLayers.size()).c_str(), "bigFont.fnt"
	);
	standardLayersCountLabel->setScale(.55);

	standardLayersMenu->addChild(standardLayersSprite);
	standardLayersMenu->addChild(standardLayersCountLabel);
	standardLayersMenu->updateLayout();

	CCMenu* ldmLayersMenu = CCMenu::create();
	ldmLayersMenu->setLayout(detailItemLayout);

	CCSprite* ldmLayersSprite =
		CCSprite::createWithSpriteFrameName("ldmFolder.png"_spr);
	ldmLayersSprite->setScale(.55);

	CCLabelBMFont* ldmLayersCountLabel = CCLabelBMFont::create(
		numToString(info.ldmLayers.size()).c_str(), "bigFont.fnt"
	);
	ldmLayersCountLabel->setScale(.55);

	ldmLayersMenu->addChild(ldmLayersSprite);
	ldmLayersMenu->addChild(ldmLayersCountLabel);
	ldmLayersMenu->updateLayout();

	detailMenu->addChild(sizeLabel);
	detailMenu->addChild(checkpointsMenu);
	detailMenu->addChild(standardLayersMenu);
	detailMenu->addChild(ldmLayersMenu);
	detailMenu->updateLayout();

	CCSprite* removeSprite =
		CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
	CCMenuItemSpriteExtra* removeBtn = CCMenuItemExt::createSpriteExtra(
		removeSprite, [this, info](CCMenuItemSpriteExtra* sender) {
			for (std::vector<SaveLayerInfo> layers :
				  {info.standardLayers, info.ldmLayers})
				for (SaveLayerInfo layer : layers)
					std::filesystem::remove(layer.filePath);

			int levelID = info.levelID;
			auto index = std::find_if(
				m_saveList.begin(), m_saveList.end(),
				[levelID](LevelSaveInfo entry) { return entry.levelID == levelID; }
			);
			m_saveList.erase(index);

			if (m_page >= getMaxPages())
				previousPage();
			else
				createList();

			updateGeneralInfoUI();
		}
	);
	removeBtn->m_baseScale = .9;
	removeBtn->setScale(.9);

	menu->addChildAtPosition(nameLabel, geode::Anchor::Left, ccp(5, 12));
	menu->addChildAtPosition(detailMenu, geode::Anchor::Left, ccp(5, -12));
	menu->addChildAtPosition(removeBtn, geode::Anchor::Right, ccp(-25, 0));

	menu->updateLayout();

	return menu;
}

void SaveManager::updatePageUI() {
	unsigned int maxPages = getMaxPages();

	m_pageLabel->setString(
		fmt::format("Page {}/{}", m_page + 1, maxPages).c_str()
	);

	if (m_page == 0) {
		m_previousPageBtn->m_animationEnabled = false;
		m_previousPageBtn->setColor(ccc3(90, 90, 90));
		m_previousPageBtn->setOpacity(200);
	} else {
		m_previousPageBtn->m_animationEnabled = true;
		m_previousPageBtn->setColor(ccc3(255, 255, 255));
		m_previousPageBtn->setOpacity(255);
	}
	if (m_page >= maxPages - 1) {
		m_nextPageBtn->m_animationEnabled = false;
		m_nextPageBtn->setColor(ccc3(90, 90, 90));
		m_nextPageBtn->setOpacity(200);
	} else {
		m_nextPageBtn->m_animationEnabled = true;
		m_nextPageBtn->setColor(ccc3(255, 255, 255));
		m_nextPageBtn->setOpacity(255);
	}
}

void SaveManager::previousPage() {
	if (m_page == 0)
		return;

	m_page--;
	createList(true);
	updatePageUI();
}

void SaveManager::nextPage() {
	if (m_page >= getMaxPages() - 1)
		return;

	m_page++;
	createList(true);
	updatePageUI();
}

void SaveManager::updateGeneralInfoUI() {
	uintmax_t totalFileSize = 0;
	for (LevelSaveInfo info : m_saveList)
		totalFileSize += info.fullFileSize;

	unsigned int pageLevelStart = m_page * LEVELS_PER_PAGE;
	m_levelCountLabel->setString(
		fmt::format(
			"{} to {}\nof {}", m_saveList.size() > 0 ? pageLevelStart + 1 : 0,
			std::min(
				pageLevelStart + LEVELS_PER_PAGE, (unsigned int)m_saveList.size()
			),
			m_saveList.size()
		)
			.c_str()
	);
	m_totalSizeLabel->setString(
		fmt::format("Total:\n{}", byteSizeAsFormattedString(totalFileSize))
			.c_str()
	);

	m_emptyListLabel->setVisible(m_saveList.empty());
}

unsigned int SaveManager::getMaxPages() {
	return std::max(
		1U, (unsigned int)ceil((float)m_saveList.size() / LEVELS_PER_PAGE)
	);
}

std::vector<LevelSaveInfo> SaveManager::getSaveList() {
	std::vector<std::pair<SaveHeader, std::filesystem::directory_entry>> saves;
	for (std::filesystem::directory_entry entry :
		  std::filesystem::directory_iterator(
			  fmt::format(
				  "{}/saves/", string::pathToString(Mod::get()->getSaveDir())
			  )
		  )) {

		saves.push_back(
			std::pair(SaveParser::fromPath(entry.path()).value(), entry)
		);
	}

	std::map<int, LevelSaveInfo> levels;
	for (std::pair<SaveHeader, std::filesystem::directory_entry> save : saves) {

		if (save.second.is_directory())
			continue;

		std::filesystem::path path = save.second.path();
		std::string filename = string::pathToString(path.filename());
		std::string filenameNoExtension = string::split(filename, ".").front();

		std::vector<std::string> layerSplit =
			string::split(filenameNoExtension, "_");
		std::optional<unsigned int> saveLayerResult =
			geode::utils::numFromString<unsigned int>(layerSplit.back()).ok();
		if (!saveLayerResult.has_value())
			continue;
		unsigned int saveLayer = saveLayerResult.value();

		std::vector<std::string> lowDetailSplit =
			string::split(layerSplit.front(), "-");
		bool lowDetail = lowDetailSplit.back() == "lowDetail";

		std::optional<int> levelIDResult =
			geode::utils::numFromString<int>(lowDetailSplit.front()).ok();
		if (!levelIDResult.has_value())
			continue;
		int levelID = levelIDResult.value();

		SaveLayerInfo saveLayerInfo = SaveLayerInfo{save.first, path};

		LevelSaveInfo* info =
			&levels
				 .try_emplace(levelID, LevelSaveInfo{levelID, save.first.levelName})
				 .first->second;

		std::vector<SaveLayerInfo>* layers =
			lowDetail ? &info->ldmLayers : &info->standardLayers;
		layers->push_back(saveLayerInfo);

		info->fullFileSize += save.second.file_size();
	}

	std::vector<LevelSaveInfo> levelList;
	levelList.reserve(levels.size());
	for (std::pair<const int, LevelSaveInfo> level : levels)
		levelList.push_back(level.second);

	return levelList;
}

std::string SaveManager::byteSizeAsFormattedString(uintmax_t size) {
	std::string sizeString;
	if (size < 1024)
		sizeString = fmt::format("{} B", size);
	else {
		double aproximatedFilesize = ((float)size) / 1024.0;
		if (aproximatedFilesize < 1024.0)
			sizeString = fmt::format("{:.2f} KiB", aproximatedFilesize);
		else {
			aproximatedFilesize /= 1024.0;
			if (aproximatedFilesize < 1024.0)
				sizeString = fmt::format("{:.2f} MiB", aproximatedFilesize);
			else
				sizeString =
					fmt::format("{:.2f} GiB", aproximatedFilesize / 1024.0);
		}
	}

	return sizeString;
}
