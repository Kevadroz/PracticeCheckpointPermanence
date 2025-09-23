#include "PlayLayer.hpp"
#include <filesystem>

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

	if (m_fields->m_activeSaveLayer == m_fields->m_saveLayerCount)
		updateUISwitcher();
}

void ModPlayLayer::removeCurrentSaveLayer() {
	unsigned int deletedSaveLayer = m_fields->m_activeSaveLayer;

	std::filesystem::path path = getSavePath();
	if (std::filesystem::exists(path))
		std::filesystem::remove(path);
	else
		return;

	while (true) {
		m_fields->m_activeSaveLayer++;
		std::filesystem::path lastPath = path;
		path = getSavePath();

		if (std::filesystem::exists(path))
			std::filesystem::rename(path, lastPath);
		else
			break;
	}

	if (deletedSaveLayer != 0)
		deletedSaveLayer--;
	m_fields->m_activeSaveLayer = deletedSaveLayer;

	updateSaveLayerCount();
	deserializeCheckpoints();
}

void ModPlayLayer::swapSaveLayers(unsigned int left, unsigned int right) {
	if (left >= m_fields->m_saveLayerCount ||
		 right >= m_fields->m_saveLayerCount)
		return;

	unsigned int activeSaveLayer = m_fields->m_activeSaveLayer;

	m_fields->m_activeSaveLayer = left;
	std::filesystem::path leftPath = getSavePath();
	if (!std::filesystem::exists(leftPath))
		return;

	m_fields->m_activeSaveLayer = right;
	std::filesystem::path rightPath = getSavePath();
	if (!std::filesystem::exists(rightPath))
		return;

	std::filesystem::path tempPath = leftPath;
	tempPath.concat(".temp_move");

	std::filesystem::rename(leftPath, tempPath);
	std::filesystem::rename(rightPath, leftPath);
	std::filesystem::rename(tempPath, rightPath);

	m_fields->m_activeSaveLayer = activeSaveLayer;
}

void ModPlayLayer::updateSaveLayerCount() {
	unsigned int activeSaveLayer = m_fields->m_activeSaveLayer;
	m_fields->m_saveLayerCount = 0;

	while (true) {
		m_fields->m_activeSaveLayer = m_fields->m_saveLayerCount;
		std::filesystem::path path = getSavePath();
		if (verifySavePath(path).has_value())
			m_fields->m_saveLayerCount++;
		else
			break;
	}

	m_fields->m_activeSaveLayer =
		std::min(activeSaveLayer, m_fields->m_saveLayerCount);
}
