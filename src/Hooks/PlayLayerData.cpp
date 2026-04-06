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

	char header[] = SAVE_HEADER;
	stream.write(header, sizeof(SAVE_HEADER));
	stream << saveVersion;
	stream << gameVersion;
	stream << platform;
	stream << m_level->m_levelVersion;
	stream << m_level->m_levelName;
	stream << checkpointCount;

	auto postFn = [](ModPlayLayer* playLayer) {
		playLayer->diskOperationFinished();
		static_cast<ModUILayer*>(playLayer->m_uiLayer)->updateSwitcher();
	};

	if (m_fields->m_isDiskOperationBlocking) {
		for (PersistentCheckpoint* checkpoint : checkpoints)
			checkpoint->serializeExternal(stream);

		for (PersistentCheckpoint* checkpoint : checkpoints)
			checkpoint->serialize(stream);

		stream.end();

		postFn(this);
	} else
		async::spawn(
			[readonlyStream = stream, checkpoints] -> arc::Future<> {
				Stream stream = std::move(readonlyStream);

				for (PersistentCheckpoint* checkpoint : checkpoints) {
					log::info("save external");
					checkpoint->serializeExternal(stream);
					co_await arc::yield();
				}

				for (PersistentCheckpoint* checkpoint : checkpoints) {
					log::info("save");
					checkpoint->serialize(stream);
					co_await arc::yield();
				}

				stream.end();

				co_return;
			},
			[postFn]() {
				log::info("save postFn");
				if (ModPlayLayer* playLayer =
						 static_cast<ModPlayLayer*>(PlayLayer::get()))
					postFn(playLayer);
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

	if (Mod::get()->getSettingValue<bool>("debug-force-fallback"))
		header.loadError = LoadError::ForcedFallback;

	if (header.loadError != LoadError::None && !isInFallbackMode())
		return;

	auto loadExternalFn =
		[header](Stream& stream, CCArrayExt<PersistentCheckpoint*>& checkpoints) {
			PersistentCheckpoint* checkpoint = PersistentCheckpoint::create();

			checkpoint->deserializeExternal(stream, header);

			checkpoint->setupPhysicalObject();
			checkpoints.push_back(checkpoint);
		};

	auto postFn = [header](
						  ModPlayLayer* playLayer,
						  CCArrayExt<PersistentCheckpoint*> checkpoints
					  ) {
		playLayer->unloadPersistentCheckpoints();
		for (PersistentCheckpoint* checkpoint : checkpoints)
			playLayer->storePersistentCheckpoint(checkpoint, false);

		playLayer->m_fields->m_loadError = header.loadError;

		playLayer->removeAllCheckpoints();

		playLayer->diskOperationFinished();
		playLayer->updateModUI();
	};

	if (m_fields->m_isDiskOperationBlocking) {
		CCArrayExt<PersistentCheckpoint*> checkpoints =
			CCArrayExt<PersistentCheckpoint*>(CCArray::create());

		for (unsigned int i = header.checkpointCount; i > 0; i--)
			loadExternalFn(stream, checkpoints);

		if (!ignoreVerification) {
			if (header.loadError != LoadError::None) {
				stream.end();
				postFn(this, checkpoints);
			}
		} else {
			header.loadError = LoadError::None;
		}

		for (PersistentCheckpoint* checkpoint : checkpoints)
			checkpoint->deserialize(stream, header);

		stream.end();
		postFn(this, checkpoints);
	} else
		async::spawn(
			[ignoreVerification, readonlyStream = stream, readonlyHeader = header,
			 loadExternalFn, postFn] -> arc::Future<> {
				Stream stream = std::move(readonlyStream);
				SaveHeader header = std::move(readonlyHeader);

				CCArrayExt<PersistentCheckpoint*> checkpoints =
					CCArrayExt<PersistentCheckpoint*>(CCArray::create());

				for (unsigned int i = header.checkpointCount; i > 0; i--) {
					log::info("load external");
					loadExternalFn(stream, checkpoints);
					co_await arc::yield();
				}

				if (!ignoreVerification) {
					if (header.loadError != LoadError::None) {
						stream.end();
						geode::queueInMainThread([postFn, checkpoints]() {
							if (ModPlayLayer* playLayer =
									 static_cast<ModPlayLayer*>(PlayLayer::get()))
								postFn(playLayer, checkpoints);
						});
						co_return;
					}
				} else {
					header.loadError = LoadError::None;
				}

				for (PersistentCheckpoint* checkpoint : checkpoints) {
					log::info("load");
					checkpoint->deserialize(stream, header);
					co_await arc::yield();
				}

				stream.end();
				geode::queueInMainThread([postFn, checkpoints]() {
					log::info("load postFn");
					if (ModPlayLayer* playLayer =
							 static_cast<ModPlayLayer*>(PlayLayer::get()))
						postFn(playLayer, checkpoints);
				});
				co_return;
			}
		);
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
	switchGhostCheckpoint(0);

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
