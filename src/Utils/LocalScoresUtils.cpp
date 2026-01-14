#include "Utils/LocalScoresUtils.hpp"

#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerLevelStatsData.hpp"
#include "bsml/shared/BSML/SharedCoroutineStarter.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"

#include "logging.hpp"

#include "custom-types/shared/coroutine.hpp"
#include <atomic>
#include <string>
#include <unordered_set>

#define COROUTINE(coroutine) BSML::SharedCoroutineStarter::get_instance()->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(coroutine));

namespace BetterSongList::LocalScoresUtils {
    SafePtrUnity<GlobalNamespace::PlayerDataModel> playerDataModel;

    std::unordered_set<std::string> playedMaps;
    std::shared_mutex mutex_playedMaps;
    // Check if scores have been loaded into the map 
    std::atomic<bool> loadedScores = false;

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
        std::shared_lock<std::shared_mutex> lock(mutex_playedMaps);
        auto levelId = level->levelID;
        return levelId ? HasLocalScore(levelId) : false;
    }

    static std::atomic<bool> isLoadingScores = false;

    void Load() {
        // If aleady loading or don't have scores, skip
        if (isLoadingScores || !get_playerDataModel()) return;
        isLoadingScores = true;
        auto playerDataModel = get_playerDataModel();
        INFO("{}", fmt::ptr(playerDataModel));
        auto playerData = playerDataModel ? playerDataModel->get_playerData() : nullptr;
        INFO("{}", fmt::ptr(playerData));
        auto levelData = ListW<GlobalNamespace::PlayerLevelStatsData*>::New();
        auto stats = playerData ? playerData->get_levelsStatsData()->get_Values()->i___System__Collections__Generic__IEnumerable_1_TValue_() : nullptr;
        INFO("{}", fmt::ptr(stats));
        levelData->AddRange(stats);
    
        if (levelData) {
            il2cpp_utils::il2cpp_aware_thread([](ListW<GlobalNamespace::PlayerLevelStatsData*> levelData){
                INFO("x1");
                
                std::unordered_set<std::string> tempPlayedMaps;
                tempPlayedMaps.reserve(500);

                for (auto x : levelData) {
                    INFO("{}", fmt::ptr(x));
                    if (!x->validScore) continue;
                    auto levelId = static_cast<std::string>(x->levelID);

                    tempPlayedMaps.insert(levelId);
                }

                // Swap in the new set
                {
                    std::unique_lock<std::shared_mutex> lock(mutex_playedMaps);
                    playedMaps.swap(tempPlayedMaps);
                }

                isLoadingScores = false;
                loadedScores = true;
            }, levelData).detach();
        }
    }
}