#include "Patches/RestoreLevelSelection.hpp"
#include "Patches/RestoreTableScroll.hpp"
#include "Patches/HookLevelCollectionTableSet.hpp"

#include "UI/FilterUI.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Sprite.hpp"
#include "GlobalNamespace/BeatmapLevelPack.hpp"
#include "GlobalNamespace/BeatmapLevelPack.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/LevelSelectionNavigationController.hpp"
#include "GlobalNamespace/LevelFilteringNavigationController.hpp"
#include "GlobalNamespace/BeatmapLevelsModel.hpp"
#include "Utils/PlaylistUtils.hpp"

#include "songcore/shared/SongCore.hpp"

#include "logging.hpp"
#include "config.hpp"
#include <string>

namespace BetterSongList::Hooks {
    SafePtr<GlobalNamespace::BeatmapLevelPack> RestoreLevelSelection::restoredPack;

    GlobalNamespace::BeatmapLevelPack* RestoreLevelSelection::get_restoredPack() {
        if (!restoredPack) {
            return nullptr;
        }
        return restoredPack.ptr();
    }
    void RestoreLevelSelection::LevelSelectionFlowCoordinator_DidActivate_Prefix(GlobalNamespace::LevelSelectionFlowCoordinator* self, GlobalNamespace::LevelSelectionFlowCoordinator::State*& startState, bool firstActivation) {
        if (firstActivation) {
            BetterSongList::FilterUI::Init();
        }
       
        if (startState) {
            WARNING("Not restoring last state because we are starting off from somewhere!");
            FilterUI::SetFilter("", false, false);
            return;
        }

        // Try parse the last category        
        auto restoreCategory = config.get_lastCategory();
        auto& restoreLevel = config.get_lastSong();
        GlobalNamespace::BeatmapLevel* m = nullptr;
        
        GlobalNamespace::BeatmapLevelsModel* levelsModel = self->levelSelectionNavigationController->_levelFilteringNavigationController->_beatmapLevelsModel;
        if (!restoreLevel.empty()) {
            levelsModel->_allLoadedBeatmapLevelsRepository->TryGetBeatmapLevelById(restoreLevel, m);
        }

        LoadPackFromCollectionName(levelsModel);
        
        if (
            restoreCategory == GlobalNamespace::SelectLevelCategoryViewController::LevelCategory::All || 
            restoreCategory == GlobalNamespace::SelectLevelCategoryViewController::LevelCategory::Favorites || 
            (!restoredPack && restoreCategory == GlobalNamespace::SelectLevelCategoryViewController::LevelCategory::CustomSongs) 
        ) {
            restoredPack = SongCore::API::Loading::GetCustomLevelPack();
        }

        if (m) {
            // if the level is custom and we're trying to restore to music packs, switch to customsongs
            // TODO: Maybe check if we actually have the level in the pack?
            auto customLevel = il2cpp_utils::try_cast<SongCore::SongLoader::CustomBeatmapLevel>(m).value_or(nullptr);
            if (customLevel) {
                if (restoreCategory == GlobalNamespace::SelectLevelCategoryViewController::LevelCategory::MusicPacks) {
                    restoreCategory = GlobalNamespace::SelectLevelCategoryViewController::LevelCategory::CustomSongs;
                }
            }

            // Check if the level is favorited, if not switch to all (might never happen but just in case)
            // Can happen if you didn't load all the levels, switched to favorites and reentered the songlist
            if (restoreCategory == GlobalNamespace::SelectLevelCategoryViewController::LevelCategory::Favorites) {
                auto playerDataModel = self->___playerDataModel;
                if (playerDataModel) {
                    auto playerData = playerDataModel->get_playerData();
                    if (playerData) {
                        if (!playerData->IsLevelUserFavorite(customLevel)) {
                            restoreCategory = GlobalNamespace::SelectLevelCategoryViewController::LevelCategory::All;
                        }
                    } else {
                        WARNING("Could not get playerData when trying to check for favorite status, setting to all");
                        restoreCategory = GlobalNamespace::SelectLevelCategoryViewController::LevelCategory::All;
                    }
                } else {
                    WARNING("Could not get playerDataModel when trying to check for favorite status, setting to all");
                    restoreCategory = GlobalNamespace::SelectLevelCategoryViewController::LevelCategory::All;
                }
            }

            // Construct the new state
            auto levelCategory = System::Nullable_1<GlobalNamespace::SelectLevelCategoryViewController::LevelCategory>();
            levelCategory.value = restoreCategory;
            levelCategory.hasValue = true;
            auto state = GlobalNamespace::LevelSelectionFlowCoordinator::State::New_ctor(
                    restoredPack ? restoredPack.ptr() : nullptr,
                    static_cast<GlobalNamespace::BeatmapLevel *>(m)
            );

            state->___levelCategory = levelCategory;
            startState = state;
        } else {
            // If no level found, just restore pack and category for now
            auto levelCategory = System::Nullable_1<GlobalNamespace::SelectLevelCategoryViewController::LevelCategory>();
            levelCategory.value = restoreCategory;
            levelCategory.hasValue = true;
            auto state = GlobalNamespace::LevelSelectionFlowCoordinator::State::New_ctor(
                restoredPack ? restoredPack.ptr() : nullptr,
                nullptr
            );

            state->___levelCategory = levelCategory;
            startState = state;
        }
    }

    void RestoreLevelSelection::LoadPackFromCollectionName(GlobalNamespace::BeatmapLevelsModel* levelsModel) {
        INFO("Loading pack from name");
        if (get_restoredPack()) {
            auto packID = get_restoredPack()->packID;
            if (packID && packID == config.get_lastPack()) {
                return;
            }
        }

        // If the pack name is empty, we don't have a pack to restore
        if (config.get_lastPack().empty()) {
            restoredPack.emplace(nullptr);
            INFO("Config lastpack was empty");
            return;
        }

        restoredPack.emplace(levelsModel->____allLoadedBeatmapLevelsRepository->GetBeatmapLevelPackByPackId(config.get_lastPack()));
    }

    void RestoreLevelSelection::LevelFilteringNavigationController_ShowPacksInSecondChildController_Prefix(GlobalNamespace::LevelFilteringNavigationController* self, StringW& levelPackIdToBeSelectedAfterPresent) {
        if (levelPackIdToBeSelectedAfterPresent) return;
        auto levelsModel = self->_beatmapLevelsModel;
        LoadPackFromCollectionName(levelsModel);

        levelPackIdToBeSelectedAfterPresent = get_restoredPack() ? get_restoredPack()->packID : nullptr;
    }
}