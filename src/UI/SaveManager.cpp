#include "SaveManager.hpp"
#include "ListMenu.hpp"

#include <asp/fs/fs.hpp>

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

	CCSprite* infoBtnSprite =
		CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
	CCMenuItemSpriteExtra* infoBtn = CCMenuItemExt::createSpriteExtra(
		infoBtnSprite, [](CCMenuItemSpriteExtra* sender) {
			FLAlertLayer::create(
				"Save Manager",
				"<cy>Checkpoint</c> - The total number of Persistent Checkpoints\n"
				"<cb>Folder</c> - Normal save layers\n"
				"<cg>ldm Folder</c> - Low detail mode layers\n"
				"<cr>Red Exclamations</c> - A save layer can't be loaded, two if "
				"no save layer can be loaded normally\n"
				"<co>Orange Exclamations</c> - A save layer can only be loaded in "
				"fallback mode, two if this is the case for all save layers\n",
				"Ok"
			)
				->show();
		}
	);
	infoBtn->setID("info");

	m_pageLabel = Label::create("Page 0/0", "bigFont.fnt");
	m_pageLabel->setID("page-label");
	m_pageLabel->setScale(.6);

	m_totalSizeLabel = Label::create("Total:\n0B", "bigFont.fnt");
	m_totalSizeLabel->setID("total-size");
	m_totalSizeLabel->setScale(.35);
	m_totalSizeLabel->setOpacity(160);
	m_totalSizeLabel->setAnchorPoint(ccp(0.0, 0.5));
	m_totalSizeLabel->setLimitLabelWidth(350.f, .35f, .1f);

	m_levelCountLabel = Label::create("70 to 80\nof 136", "bigFont.fnt");
	m_levelCountLabel->setID("level-count");
	m_levelCountLabel->setScale(.35);
	m_levelCountLabel->setOpacity(160);
	m_levelCountLabel->setAnchorPoint(ccp(1.0, 0.5));
	m_levelCountLabel->setAlignment(Label::Alignment::Center);

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
	m_previousPageBtn->setID("previous-page");
	m_nextPageBtn->setID("next-page");
	m_previousPageBtn->m_baseScale = .6;
	m_nextPageBtn->m_baseScale = .6;
	m_previousPageBtn->setScale(.6);
	m_nextPageBtn->setScale(.6);

	m_listContainer = CCLayerColor::create();
	m_listContainer->setID("level-list");
	m_listContainer->setContentSize(ccp(400, 170));
	m_listContainer->setAnchorPoint(ccp(.5, 1));
	m_listContainer->setColor(ccc3(27, 27, 27));
	m_listContainer->setOpacity(255);
	m_listContainer->setZOrder(5);

	m_emptyListLabel = Label::create("No saves\nfound", "bigFont.fnt");
	m_emptyListLabel->setID("empty-list-label");
	m_emptyListLabel->setScale(.7);
	m_emptyListLabel->setOpacity(220);
	m_emptyListLabel->setAlignment(Label::Alignment::Center);

	m_listContainer->addChildAtPosition(m_emptyListLabel, Anchor::Center);

	m_buttonMenu->addChildAtPosition(infoBtn, Anchor::BottomRight);
	m_buttonMenu->addChildAtPosition(
		m_previousPageBtn, Anchor::TopLeft, ccp(25, -50)
	);
	m_buttonMenu->addChildAtPosition(
		m_nextPageBtn, Anchor::TopRight, ccp(-25, -50)
	);
	m_mainLayer->addChildAtPosition(m_pageLabel, Anchor::Top, ccp(0, -50));
	m_mainLayer->addChildAtPosition(
		m_totalSizeLabel, Anchor::TopLeft, ccp(25, -20)
	);
	m_mainLayer->addChildAtPosition(
		m_levelCountLabel, Anchor::TopRight, ccp(-10, -20)
	);
	m_mainLayer->addChildAtPosition(m_listContainer, Anchor::Top, ccp(0, -73));

	m_saveList = getSaveList();

	createList();

	ListBorders* borders = ListBorders::create();
	borders->setID("borders");
	borders->setContentSize(m_listView->getContentSize() + ccp(7, 7));
	borders->setZOrder(15);
	borders->setSpriteFrames(
		"darkGrayNegativeBorderTop.png"_spr, "darkGrayNegativeBorderSide.png"_spr
	);
	m_listContainer->addChildAtPosition(borders, Anchor::Center);

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
	m_listView->setID("view");
	m_listView->setPrimaryCellColor(ccc3(35, 35, 35));
	m_listView->setSecondaryCellColor(ccc3(30, 30, 30));
	m_listView->setZOrder(10);

	CCNode* listContent = m_listView->getChildByIndex(0)->getChildByIndex(0);
	if (carryPosition && m_cellsArray->count() >= 5) {
		listContent->setPositionY(contentPosition);
		if (listContent->getPositionY() > 0)
			listContent->setPositionY(0);
	}

	m_listContainer->addChildAtPosition(m_listView, Anchor::BottomLeft);
}

CCNode* SaveManager::createLevelCell(LevelSaveInfo info) {
	CCMenu* menu = ListMenu::create(m_listContainer);
	menu->setContentSize(ccp(400, 50));

	Label* nameLabel = Label::create(info.levelName.c_str(), "bigFont.fnt");
	nameLabel->setID("name");
	nameLabel->setAnchorPoint(ccp(0, .5));
	nameLabel->setScale(.75);

	std::string fileSizeString = byteSizeAsFormattedString(info.fullFileSize);

	unsigned int totalCheckpointCount = 0;
	unsigned int criticalErrorCount = 0;
	unsigned int warningErrorCount = 0;
	bool allErrors = true;

	for (std::vector<SaveLayerInfo> layers :
		  {info.standardLayers, info.ldmLayers})
		for (SaveLayerInfo layer : layers) {
			totalCheckpointCount += layer.header.checkpointCount;
			if (layer.header.loadError != LoadError::None) {
				if (SaveParser::isErrorFallbackCapable(layer.header.loadError))
					warningErrorCount++;
				else
					criticalErrorCount++;
			} else
				allErrors = false;
		}

	CCMenu* detailMenu = CCMenu::create();
	detailMenu->setID("details");
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

	Label* sizeLabel = Label::create(fileSizeString.c_str(), "bigFont.fnt");
	sizeLabel->setID("size");
	sizeLabel->setAnchorPoint(ccp(0, .5));
	sizeLabel->setScale(.55);

	CCMenu* checkpointsMenu = CCMenu::create();
	checkpointsMenu->setID("checkpoints");
	checkpointsMenu->setLayout(detailItemLayout);

	CCSprite* checkpointSprite =
		CCSprite::createWithSpriteFrameName("activeCheckpoint.png"_spr);
	checkpointSprite->setScale(.55);

	Label* checkpointCountLabel =
		Label::create(numToString(totalCheckpointCount).c_str(), "bigFont.fnt");
	checkpointCountLabel->setScale(.55);

	checkpointsMenu->addChild(checkpointSprite);
	checkpointsMenu->addChild(checkpointCountLabel);
	checkpointsMenu->updateLayout();

	CCMenu* standardLayersMenu = CCMenu::create();
	standardLayersMenu->setID("standard-layers");
	standardLayersMenu->setLayout(detailItemLayout);

	CCSprite* standardLayersSprite =
		CCSprite::createWithSpriteFrameName("folderIcon_001.png");
	standardLayersSprite->setScale(.55);

	Label* standardLayersCountLabel = Label::create(
		numToString(info.standardLayers.size()).c_str(), "bigFont.fnt"
	);
	standardLayersCountLabel->setScale(.55);

	standardLayersMenu->addChild(standardLayersSprite);
	standardLayersMenu->addChild(standardLayersCountLabel);
	standardLayersMenu->updateLayout();

	CCMenu* ldmLayersMenu = CCMenu::create();
	ldmLayersMenu->setID("ldm-layers");
	ldmLayersMenu->setLayout(detailItemLayout);

	CCSprite* ldmLayersSprite =
		CCSprite::createWithSpriteFrameName("ldmFolder.png"_spr);
	ldmLayersSprite->setScale(.55);

	Label* ldmLayersCountLabel =
		Label::create(numToString(info.ldmLayers.size()).c_str(), "bigFont.fnt");
	ldmLayersCountLabel->setScale(.55);

	ldmLayersMenu->addChild(ldmLayersSprite);
	ldmLayersMenu->addChild(ldmLayersCountLabel);
	ldmLayersMenu->updateLayout();

	detailMenu->addChild(sizeLabel);
	detailMenu->addChild(checkpointsMenu);
	detailMenu->addChild(standardLayersMenu);
	detailMenu->addChild(ldmLayersMenu);

	if (warningErrorCount > 0 || criticalErrorCount > 0) {
		CCMenu* errorMenu = CCMenu::create();
		errorMenu->setID("errors");

		RowLayout* errorMenuLayout = RowLayout::create();
		errorMenuLayout->setAutoScale(false);
		errorMenuLayout->setAutoGrowAxis(0.0F);
		errorMenuLayout->setGap(2.0F);
		errorMenu->setLayout(errorMenuLayout);

		if (criticalErrorCount > 0) {
			Label* criticalIndicator =
				Label::create(allErrors ? "!!" : "!", "bigFont.fnt");
			criticalIndicator->setColor(ccc3(224, 111, 111));
			criticalIndicator->setScale(.7);

			errorMenu->addChild(criticalIndicator);
		}

		if (warningErrorCount > 0) {
			Label* warningIndicator = Label::create(
				allErrors && criticalErrorCount == 0 ? "!!" : "!", "bigFont.fnt"
			);
			warningIndicator->setColor(ccc3(224, 166, 111));
			warningIndicator->setScale(.7);

			errorMenu->addChild(warningIndicator);
		}
		errorMenu->updateLayout();

		detailMenu->addChild(errorMenu);
	}

	detailMenu->updateLayout();

	CCSprite* removeSprite =
		CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
	CCMenuItemSpriteExtra* removeBtn = CCMenuItemExt::createSpriteExtra(
		removeSprite, [this, info](CCMenuItemSpriteExtra* sender) {
			for (std::vector<SaveLayerInfo> layers :
				  {info.standardLayers, info.ldmLayers})
				for (SaveLayerInfo layer : layers)
					asp::fs::remove(layer.filePath)
						.inspectErr([path = layer.filePath](asp::fs::Error err) {
							log::error(
								"Couldn't remove save layer with path '{}': {}", path,
								err
							);
						});

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
	removeBtn->setID("remove");
	removeBtn->m_baseScale = .9;
	removeBtn->setScale(.9);

	menu->addChildAtPosition(nameLabel, Anchor::Left, ccp(5, 12));
	menu->addChildAtPosition(detailMenu, Anchor::Left, ccp(5, -12));
	menu->addChildAtPosition(removeBtn, Anchor::Right, ccp(-25, 0));

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
	std::map<int, LevelSaveInfo> levels;
	std::map<int, LevelSaveInfo> editorLevels;

	for (std::map<int, LevelSaveInfo>* levelList : {&levels, &editorLevels}) {
		std::vector<std::pair<SaveHeader, asp::fs::DirectoryEntry>> saves;

		const char* subPath;
		bool isEditor;
		if (levelList == &levels) {
			subPath = "";
			isEditor = false;
		} else {
			subPath = "editor/";
			isEditor = true;
		}

		Result<asp::fs::DirectoryIterator> iterator = asp::fs::iterDir(
			fmt::format("{}/saves/{}", Mod::get()->getSaveDir().string(), subPath)
		);
		if (iterator.isErr())
			continue;

		for (auto rawEntry : iterator.unwrap()) {
			if (rawEntry.isErr())
				continue;

			asp::fs::DirectoryEntry entry = rawEntry.unwrap();
			if (entry.isDir())
				continue;

			saves.push_back(
				std::pair(
					SaveParser::fromPath(entry.path, nullptr, isEditor).value(),
					entry
				)
			);
		}

		for (std::pair<SaveHeader, asp::fs::DirectoryEntry> save : saves) {
			if (save.second.isDir())
				continue;

			asp::fs::Path path = save.second.path;
			std::string filename = path.filename()->string();
			std::string filenameNoExtension = string::split(filename, ".").front();

			std::vector<std::string> layerSplit =
				string::split(filenameNoExtension, "_");
			std::optional<unsigned int> saveLayerResult =
				utils::numFromString<unsigned int>(layerSplit.back()).ok();
			if (!saveLayerResult.has_value())
				continue;
			unsigned int saveLayer = saveLayerResult.value();

			std::vector<std::string> lowDetailSplit =
				string::split(layerSplit.front(), "-");
			bool lowDetail = lowDetailSplit.back() == "lowDetail";

			std::optional<int> levelIDResult =
				utils::numFromString<int>(lowDetailSplit.front()).ok();
			if (!levelIDResult.has_value())
				continue;
			int levelID = levelIDResult.value();

			SaveLayerInfo saveLayerInfo = SaveLayerInfo{save.first, path};

			LevelSaveInfo* info =
				&levelList
					 ->try_emplace(
						 levelID, LevelSaveInfo{levelID, save.first.levelName}
					 )
					 .first->second;

			std::vector<SaveLayerInfo>* layers =
				lowDetail ? &info->ldmLayers : &info->standardLayers;
			layers->push_back(saveLayerInfo);

			info->fullFileSize += asp::fs::fileSize(save.second.path).unwrapOr(0);
		}
	}

	std::vector<LevelSaveInfo> levelList;
	levelList.reserve(levels.size() + editorLevels.size());
	for (std::pair<const int, LevelSaveInfo> level : editorLevels)
		levelList.push_back(level.second);
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
