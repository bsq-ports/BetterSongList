#include "Patches/RestoreLevelSelection.hpp"
#include "Patches/RestoreTableScroll.hpp"
#include "Patches/HookLevelCollectionTableSet.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Sprite.hpp"
#include "GlobalNamespace/BeatmapLevelPack.hpp"
#include "GlobalNamespace/BeatmapLevelPack.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "Utils/PlaylistUtils.hpp"

#include "songcore/shared/SongCore.hpp"

#include "logging.hpp"
#include "config.hpp"

namespace BetterSongList::Hooks {
    std::string RestoreLevelSelection::restoredPackId;
    SafePtr<GlobalNamespace::BeatmapLevelPack> RestoreLevelSelection::restoredPack;

    GlobalNamespace::BeatmapLevelPack* RestoreLevelSelection::get_restoredPack() {
        if (!restoredPack) {
            return nullptr;
        }
        return restoredPack.ptr();
    }
    void RestoreLevelSelection::LevelSelectionFlowCoordinator_DidActivate_Prefix(GlobalNamespace::LevelSelectionFlowCoordinator::State*& startState, bool firstActivation) {
        auto startPack = startState ? startState->beatmapLevelPack : nullptr;
        auto startPackId = startPack ? startPack->packID : nullptr;
        restoredPackId = startPackId ? static_cast<std::string>(startPackId) : "";
        if (startState) {
            return;
        }

        INFO("restored Pack Id: {}", restoredPackId);
        auto restoreCategory = config.get_lastCategory();
        auto& restoreLevel = config.get_lastSong();
        GlobalNamespace::BeatmapLevel* m = nullptr;

        if (!restoreLevel.empty()) {
            m = SongCore::API::Loading::GetLevelByLevelID(restoreLevel);
        }

        LoadPackFromCollectionName();

        startState = GlobalNamespace::LevelSelectionFlowCoordinator::State::New_ctor(
            System::Nullable_1<LevelCategory>(true, restoreCategory),
            get_restoredPack(),
            m,
            nullptr
        );
    }

    void RestoreLevelSelection::LoadPackFromCollectionName() {
        INFO("Loading pack from name");
        if (get_restoredPack()) {
            auto packID = get_restoredPack()->packID;
            if (packID && packID == config.get_lastPack()) {
                return;
            }
        }

        if (config.get_lastPack().empty()) {
            restoredPack.emplace(nullptr);
            INFO("Config lastpack was empty");
            return;
        }

        restoredPack.emplace(PlaylistUtils::GetPack(config.get_lastPack()));
    }

    void RestoreLevelSelection::LevelFilteringNavigationController_ShowPacksInSecondChildController_Prefix(StringW& levelPackIdToBeSelectedAfterPresent) {
        if (levelPackIdToBeSelectedAfterPresent) return;
        LoadPackFromCollectionName();

        levelPackIdToBeSelectedAfterPresent = get_restoredPack() ? get_restoredPack()->packID : nullptr;
    }
}