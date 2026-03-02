#include "PlayLayer.hpp"
#include "UILayer.hpp"

#include <arc/future/UtilPollables.hpp>
#include <sabe.persistenceapi/include/util/Stream.hpp>

void ModPlayLayer::serializeCheckpoints() {
	if (m_fields->m_currentDiskOperation != DiskOperation::None) {
		m_fields->m_serializationQueued = true;
		return;
	}

	if (m_fields->m_loadError != LoadError::None)
		return;

	if (m_fields->m_persistentCheckpointArray->count() == 0) {
		removeCurrentSaveLayer();
		return;
	}

#ifndef PCP_DEBUG
	if (m_level->m_levelType == GJLevelType::Editor)
		return;
#endif

	m_fields->m_currentDiskOperation = DiskOperation::Serializing;

	CCArrayExt<PersistentCheckpoint*> checkpoints =
		CCArrayExt<PersistentCheckpoint*>(
			m_fields->m_persistentCheckpointArray->shallowCopy()
		);

	char platform = PLATFORM;
	unsigned int saveVersion = CURRENT_VERSION;
	gd::string gameVersion = geode::Loader::get()->getGameVersion();
	unsigned int checkpointCount = checkpoints.size();

	persistenceAPI::Stream stream;
	stream.setFile(string::pathToString(getSavePath()), 2, true);

	gd::string header = SAVE_HEADER;
	stream << header;
	stream << saveVersion;
	stream << gameVersion;
	stream << platform;
	stream << m_level->m_levelVersion;
	stream << m_level->m_levelName;
	stream << checkpointCount;

	async::spawn(
		[&stream, checkpoints] -> arc::Future<> {
			for (PersistentCheckpoint* checkpoint : checkpoints) {
				co_await arc::yield();
				checkpoint->serializeExternal(stream);
			}

			for (PersistentCheckpoint* checkpoint : checkpoints) {
				co_await arc::yield();
				checkpoint->serialize(stream);
			}

			stream.end();

			co_return;
		},
		[]() {
			if (ModPlayLayer* playLayer =
					 static_cast<ModPlayLayer*>(PlayLayer::get())) {
				playLayer->diskOperationFinished();
				static_cast<ModUILayer*>(playLayer->m_uiLayer)->updateSwitcher();
			}
		}
	);
}

void ModPlayLayer::deserializeCheckpoints(bool ignoreVerification) {
	if (m_fields->m_currentDiskOperation != DiskOperation::None) {
		m_fields->m_deserializationQueued = true;
		return;
	}

#ifndef PCP_DEBUG
	if (m_level->m_levelType == GJLevelType::Editor)
		return;
#endif

	std::string savePath = string::pathToString(getSavePath());
	if (!std::filesystem::exists(savePath))
		return;

	m_fields->m_currentDiskOperation = DiskOperation::Deserializing;
	static_cast<ModUILayer*>(m_uiLayer)->updateSwitcher();

	persistenceAPI::Stream stream;
	stream.setFile(savePath, 2);

	SaveHeader header = SaveParser::fromStream(stream, m_level);

	// >DEBUG Force Fallback
	// header.loadError = LoadError::LevelVersionMismatch;

	if (header.loadError != LoadError::None && !isInFallbackMode())
		return;

	async::spawn([ignoreVerification, &stream, header] -> arc::Future<> {
		SaveHeader localHeader = header;

		CCArrayExt<PersistentCheckpoint*> checkpoints =
			CCArrayExt<PersistentCheckpoint*>(CCArray::create());

		for (unsigned int i = localHeader.checkpointCount; i > 0; i--) {
			PersistentCheckpoint* checkpoint = PersistentCheckpoint::create();

			checkpoint->deserializeExternal(stream, localHeader);

			checkpoint->setupPhysicalObject();
			checkpoints.push_back(checkpoint);
		}

		auto test = [header, &checkpoints]() {
			if (ModPlayLayer* playLayer =
					 static_cast<ModPlayLayer*>(PlayLayer::get())) {
				playLayer->unloadPersistentCheckpoints();
				for (PersistentCheckpoint* checkpoint : checkpoints)
					playLayer->storePersistentCheckpoint(checkpoint, false);

				playLayer->m_fields->m_loadError = header.loadError;

				playLayer->removeAllCheckpoints();

				playLayer->diskOperationFinished();
				playLayer->updateModUI();
				DeserializationFinishedEvent().send();
			}
		};

		if (!ignoreVerification) {
			if (localHeader.loadError != LoadError::None) {
				stream.end();
				geode::queueInMainThread(test);
				co_return;
			}
		} else {
			localHeader.loadError = LoadError::None;
		}

		for (PersistentCheckpoint* checkpoint : checkpoints)
			checkpoint->deserialize(stream, localHeader);

		stream.end();
		geode::queueInMainThread(test);
		co_return;
	});
}

void ModPlayLayer::diskOperationFinished() {
	m_fields->m_currentDiskOperation = DiskOperation::None;
	if (m_fields->m_serializationQueued) {
		m_fields->m_serializationQueued = false;
		serializeCheckpoints();
	} else if (m_fields->m_deserializationQueued) {
		m_fields->m_deserializationQueued = false;
		deserializeCheckpoints();
	}
}

void ModPlayLayer::unloadPersistentCheckpoints() {
	for (PersistentCheckpoint* checkpoint : CCArrayExt<PersistentCheckpoint*>(
			  m_fields->m_persistentCheckpointArray
		  )) {
		checkpoint->m_checkpoint->m_physicalCheckpointObject->removeFromParent();
	}
	m_fields->m_activeCheckpoint = 0;

	if (isInFallbackMode()) {
		m_currentCheckpoint = nullptr;
		setStartPosObject(nullptr);
	}

	m_fields->m_persistentCheckpointArray->removeAllObjects();
}

void ModPlayLayer::resave() {
	if (isInFallbackMode()) {
		for (unsigned int i = 0;
			  i < m_fields->m_persistentCheckpointArray->count(); ++i) {
			PersistentCheckpoint* checkpoint = static_cast<PersistentCheckpoint*>(
				m_fields->m_persistentCheckpointArray->objectAtIndex(i)
			);
			switchCurrentCheckpoint(i + 1);
			loadStartPosObject();
			checkpoint->storeData(m_currentCheckpoint, this);
		}
		m_fields->m_loadError = LoadError::None;
		serializeCheckpoints();

		switchCurrentCheckpoint(0);
	} else
		serializeCheckpoints();
}

std::filesystem::path ModPlayLayer::getSavePath() {
	return getSavePath(m_level, m_lowDetailMode, m_fields->m_activeSaveLayer);
}

std::filesystem::path ModPlayLayer::getSavePath(
	GJGameLevel* level, bool lowDetail, unsigned int saveLayer
) {
	std::string savePath = fmt::format(
		"{}/saves/{}", string::pathToString(Mod::get()->getSaveDir()),
		level->m_levelID.value()
	);

	if (lowDetail)
		savePath.append("-lowDetail");

#ifdef PCP_DEBUG
	if (level->m_levelType == GJLevelType::Editor)
		savePath = fmt::format(
			"{}/saves/editor", string::pathToString(Mod::get()->getSaveDir())
		);
#endif

	savePath.append(fmt::format("_{}.pcp", saveLayer));

	return savePath;
}
