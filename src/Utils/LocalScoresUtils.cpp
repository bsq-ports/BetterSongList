#include "Utils/LocalScoresUtils.hpp"

#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerLevelStatsData.hpp"
#include "bsml/shared/BSML/SharedCoroutineStarter.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "hooking.hpp"
#include "logging.hpp"

#include "custom-types/shared/coroutine.hpp"
#include <atomic>
#include <mutex>
#include <string>
#include <unordered_set>

#define COROUTINE(coroutine) BSML::SharedCoroutineStarter::get_instance()->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(coroutine));

namespace BetterSongList::LocalScoresUtils {
    SafePtrUnity<GlobalNamespace::PlayerDataModel> playerDataModel;

    std::unordered_set<std::string> playedMaps;
    std::shared_mutex mutex_playedMaps;
    // Check if scores have been loaded into the map 
    std::atomic<bool> loadedScores = false;
    // Is loading scores right now
    static std::atomic<bool> isLoadingScores = false;

    /**
     * @brief Get the playerDataModel object and cache it
     * 
     * @return GlobalNamespace::PlayerDataModel* 
     */
    GlobalNamespace::PlayerDataModel* get_playerDataModel() {
        if (!playerDataModel || !playerDataModel.ptr()) {
            playerDataModel = UnityEngine::Object::FindObjectOfType<GlobalNamespace::PlayerDataModel*>();
        }
        return playerDataModel.ptr();
    }

    bool get_hasScores() {
        return loadedScores;
    }

    bool HasLocalScore(std::string levelId) {
        std::shared_lock<std::shared_mutex> lock(mutex_playedMaps);
        if (playedMaps.find(levelId) != playedMaps.end()) {
            return true;
        }
        return false;
    }

    bool HasLocalScore(GlobalNamespace::BeatmapLevel* level) {
        if (!level) return false;
        auto levelId = level->___levelID;
        return levelId ? HasLocalScore(levelId) : false;
    }


    void Load() {
        // If aleady loading or don't have scores, skip
        // get_playerDataModel() will set playerDataModel if not set, it i
        if (isLoadingScores || !get_playerDataModel()) return;
        isLoadingScores = true;
        
        il2cpp_utils::il2cpp_aware_thread([](){
            try {
                auto playerData = playerDataModel ? playerDataModel->_playerData : nullptr;
                if (!playerData) {
                    WARNING("LocalScoresUtils::Load() => No player data found, cannot load local scores");
                    isLoadingScores = false;
                    return;
                }

                // Get all level stats data
                auto levelData = ListW<GlobalNamespace::PlayerLevelStatsData*>::New();
                auto stats = playerData ? playerData->get_levelsStatsData()->get_Values()->i___System__Collections__Generic__IEnumerable_1_TValue_() : nullptr;
                levelData->AddRange(stats);
                
                // Populate playedMaps
                std::unique_lock<std::shared_mutex> lock(BetterSongList::LocalScoresUtils::mutex_playedMaps);
                playedMaps.reserve(500);
                for (auto x : levelData) {
                    if (!x->_validScore) continue;
                    auto levelId = static_cast<std::string>(x->_levelID);
                    playedMaps.insert(levelId);
                }

                isLoadingScores = false;
                loadedScores = true;
            } catch (...) {
                ERROR("LocalScoresUtils::Load() => Exception during loading local scores");
                isLoadingScores = false;
                return;
            }
        }).detach();
    }
}

MAKE_AUTO_HOOK_MATCH(PlayerLevelStatsData_UpdateScoreData, &GlobalNamespace::PlayerLevelStatsData::UpdateScoreData,  void, GlobalNamespace::PlayerLevelStatsData* self, int32_t score, int32_t maxCombo, bool fullCombo, ::GlobalNamespace::RankModel_Rank rank) {
    // Will become valid after this call
    if (!self->_validScore) {
        std::unique_lock<std::shared_mutex> lock(BetterSongList::LocalScoresUtils::mutex_playedMaps);
        BetterSongList::LocalScoresUtils::playedMaps.insert(static_cast<std::string>(self->_levelID));
    }
    PlayerLevelStatsData_UpdateScoreData(self, score, maxCombo, fullCombo, rank);
};
