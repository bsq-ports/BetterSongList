#include "beatsaber-hook/shared/threading.hpp"
#include "beatsaber-hook/shared/listw.hpp"
#include "beatsaber-hook/shared/safeptr.hpp"
#include "Utils/LocalScoresUtils.hpp"

#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerLevelStatsData.hpp"
#include "bsml/shared/BSML/SharedCoroutineStarter.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "hooking.hpp"
#include "logging.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "custom-types/shared/coroutine.hpp"
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_set>

#define COROUTINE(coroutine) BSML::SharedCoroutineStarter::get_instance()->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(coroutine));

namespace BetterSongList::LocalScoresUtils {
    safe_ptr<GlobalNamespace::PlayerDataModel*> playerDataModel;

    std::unordered_set<std::string> playedMaps;
    std::shared_mutex mutex_playedMaps;
    // Check if scores have been loaded into the map 
    std::atomic<bool> loadedScores = false;
    static std::mutex loadMutex;
    static std::shared_future<void> loadFuture;

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


    std::shared_future<void> Load() {
        std::lock_guard lock(loadMutex);
        // Share an in-flight attempt, but allow retrying a completed failure.
        if (loadFuture.valid() && (loadedScores || loadFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready)) {
            return loadFuture;
        }
        auto completion = std::make_shared<std::promise<void>>();
        loadFuture = completion->get_future().share();

        try {
            BSML::MainThreadScheduler::Schedule([completion] {
                try {
                    auto* model = get_playerDataModel();
                    auto* playerData = model ? model->_playerData : nullptr;
                    if (!playerData) throw std::runtime_error("Player data is not available");

                    il2cpp_thread([completion, playerData = safe_ptr<GlobalNamespace::PlayerData*>(playerData)] {
                        try {
                            // Get all level stats data
                            auto levelData = ListW<GlobalNamespace::PlayerLevelStatsData*>::New();
                            auto* levelStats = playerData->get_levelsStatsData();
                            if (!levelStats) throw std::runtime_error("Local score data is not available");
                            auto stats = levelStats->get_Values()->i___System__Collections__Generic__IEnumerable_1_TValue_();
                            levelData->AddRange(stats);
                
                            // Populate playedMaps
                            std::unique_lock<std::shared_mutex> lock(BetterSongList::LocalScoresUtils::mutex_playedMaps);
                            playedMaps.reserve(500);
                            for (auto x : levelData) {
                                if (!x->_validScore) continue;
                                auto levelId = static_cast<std::string>(x->_levelID);
                                playedMaps.insert(levelId);
                            }

                            loadedScores = true;
                            completion->set_value();
                        } catch (...) {
                            ERROR("LocalScoresUtils::Load() => Exception during loading local scores");
                            completion->set_exception(std::current_exception());
                        }
                    }).detach();
                } catch (...) {
                    completion->set_exception(std::current_exception());
                }
            });
        } catch (...) {
            completion->set_exception(std::current_exception());
        }
        return loadFuture;
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
