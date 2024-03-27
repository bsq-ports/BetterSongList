#pragma once

#include "GlobalNamespace/LevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/LevelFilteringNavigationController.hpp"
#include "GlobalNamespace/BeatmapLevelPack.hpp"

namespace BetterSongList::Hooks {
    class RestoreLevelSelection {
        public:
            /// @brief no prio
            static void LevelSelectionFlowCoordinator_DidActivate_Prefix(GlobalNamespace::LevelSelectionFlowCoordinator::State*& startState, bool firstActivation);
            /// @brief prio int minvalue
            static void LevelFilteringNavigationController_ShowPacksInSecondChildController_Prefix(StringW& levelPackIdToBeSelectedAfterPresent);
            static GlobalNamespace::BeatmapLevelPack* get_restoredPack();
        private:
            static void LoadPackFromCollectionName();
            static std::string restoredPackId;
            static SafePtr<GlobalNamespace::BeatmapLevelPack> restoredPack;
    };
}