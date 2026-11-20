#include "PlayLayer.hpp"

#include <asp/fs/fs.hpp>

void ModPlayLayer::nextSaveLayer() {
	if (m_fields->m_saveLayerCount == 0)
		return;

	unsigned int layer = m_fields->m_activeSaveLayer + 1;
	if (layer > m_fields->m_saveLayerCount)
		layer = 0;

	switchCurrentSaveLayer(layer);
}

void ModPlayLayer::previousSaveLayer() {
	if (m_fields->m_saveLayerCount == 0)
		return;

	unsigned int layer = m_fields->m_activeSaveLayer - 1;
	if (m_fields->m_activeSaveLayer == 0)
		layer = m_fields->m_saveLayerCount;

	switchCurrentSaveLayer(layer);
}

void ModPlayLayer::switchCurrentSaveLayer(unsigned int saveLayer) {
	updateSaveLayerCount();

	m_fields->m_activeSaveLayer =
		std::clamp(saveLayer, (unsigned int)0, m_fields->m_saveLayerCount);

	deserializeCheckpoints();
	updateModUI();
}

void ModPlayLayer::removeCurrentSaveLayer() {
	unsigned int deletedSaveLayer = m_fields->m_activeSaveLayer;

	asp::fs::Path path = getSavePath();
	if (path.exists())
		asp::fs::remove(path).inspectErr([path](asp::fs::Error err) {
			log::error("Couldn't remove save layer with path '{}': {}", path, err);
		});
	else
		return;

	while (true) {
		m_fields->m_activeSaveLayer++;
		asp::fs::Path lastPath = path;
		path = getSavePath();

		if (path.exists())
			asp::fs::rename(path, lastPath).inspectErr([path](asp::fs::Error err) {
				log::error(
					"Couldn't rename save layer with path '{}': {}", path, err
				);
			});
		else
			break;
	}

	if (deletedSaveLayer != 0)
		deletedSaveLayer--;
	m_fields->m_activeSaveLayer = deletedSaveLayer;

	updateSaveLayerCount();
	deserializeCheckpoints();
	updateModUI();
}

void ModPlayLayer::swapSaveLayers(unsigned int left, unsigned int right) {
	if (
		left >= m_fields->m_saveLayerCount || right >= m_fields->m_saveLayerCount
	)
		return;

	unsigned int activeSaveLayer = m_fields->m_activeSaveLayer;

	m_fields->m_activeSaveLayer = left;
	asp::fs::Path leftPath = getSavePath();
	if (!leftPath.exists())
		return;

	m_fields->m_activeSaveLayer = right;
	asp::fs::Path rightPath = getSavePath();
	if (!rightPath.exists())
		return;

	asp::fs::Path tempPath = leftPath;
	tempPath.concat(".temp_move");

	asp::fs::rename(leftPath, tempPath)
		.inspectErr([leftPath](asp::fs::Error err) {
			log::error(
				"Couldn't rename save layer with path '{}': {}", leftPath, err
			);
		});
	asp::fs::rename(rightPath, leftPath)
		.inspectErr([rightPath](asp::fs::Error err) {
			log::error(
				"Couldn't rename save layer with path '{}': {}", rightPath, err
			);
		});
	asp::fs::rename(tempPath, rightPath)
		.inspectErr([tempPath](asp::fs::Error err) {
			log::error(
				"Couldn't rename save layer with path '{}': {}", tempPath, err
			);
		});

	m_fields->m_activeSaveLayer = activeSaveLayer;
}

void ModPlayLayer::updateSaveLayerCount() {
	m_fields->m_saveLayerCount = getSaveLayerCount(m_level, m_lowDetailMode);
	m_fields->m_activeSaveLayer =
		std::min(m_fields->m_activeSaveLayer, m_fields->m_saveLayerCount);
}

unsigned int
ModPlayLayer::getSaveLayerCount(GJGameLevel* level, bool lowDetail) {
	unsigned int saveLayerCount = 0;

	while (true) {
		std::optional<SaveHeader> optionalHeader = SaveParser::fromPath(
			getSavePath(level, lowDetail, saveLayerCount), level
		);

		if (!optionalHeader.has_value())
			break;
		else
			saveLayerCount++;
	}

	return saveLayerCount;
}
