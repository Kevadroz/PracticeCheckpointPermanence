#include "../SaveParser.hpp"

#define LEVELS_PER_PAGE 10

using namespace geode::prelude;

struct SaveLayerInfo {
	SaveHeader header;
	std::filesystem::path filePath;
};

struct LevelSaveInfo {
	int levelID;
	gd::string levelName;
	std::vector<SaveLayerInfo> standardLayers = std::vector<SaveLayerInfo>();
	std::vector<SaveLayerInfo> ldmLayers = std::vector<SaveLayerInfo>();

	uintmax_t fullFileSize;
};

class SaveManager : public Popup {
public:
	static SaveManager* create();
	bool init();

	void previousPage();
	void nextPage();
private:
	unsigned int m_page = 0;
	CCLabelBMFont* m_pageLabel = nullptr;
	CCMenuItemSpriteExtra* m_previousPageBtn = nullptr;
	CCMenuItemSpriteExtra* m_nextPageBtn = nullptr;

	CCLabelBMFont* m_levelCountLabel = nullptr;
	CCLabelBMFont* m_totalSizeLabel = nullptr;

	CCLayerColor* m_listContainer = nullptr;
	ListView* m_listView = nullptr;
	CCLabelBMFont* m_emptyListLabel = nullptr;

	Ref<CCArray> m_cellsArray = CCArray::create();

	std::vector<LevelSaveInfo> m_saveList;

	void createList(bool resetPosition = false);
	CCNode* createLevelCell(LevelSaveInfo info);

	void updatePageUI();
	void updateGeneralInfoUI();

	unsigned int getMaxPages();

	static std::vector<LevelSaveInfo> getSaveList();
	static SaveHeader parseSaveFile(std::string path);

	static std::string byteSizeAsFormattedString(uintmax_t size);
};
