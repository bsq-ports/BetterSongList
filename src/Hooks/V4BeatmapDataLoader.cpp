#include "hooking.hpp"

#include "Patches/HookBeatmapDataLoader.hpp"

#include "BeatmapDataLoaderVersion4/BeatmapDataLoader.hpp"
#include "BeatmapSaveDataVersion4/BeatmapSaveData.hpp"
#include "BeatmapSaveDataVersion4/BeatmapBeatIndex.hpp"
#include "BeatmapSaveDataVersion4/ChainBeatIndex.hpp"
#include "GlobalNamespace/BeatmapDataBasicInfo.hpp"
#include "UnityEngine/JsonUtility.hpp"

// from UI/SongDeleteButton
// from UI/ExtraLevelParams
MAKE_AUTO_HOOK_MATCH(BeatmapDataLoaderVersion4_BeatmapDataLoader_GetBeatmapDataBasicInfoFromSaveDataJson,
                    &BeatmapDataLoaderVersion4::BeatmapDataLoader::GetBeatmapDataBasicInfoFromSaveDataJson,
                    GlobalNamespace::BeatmapDataBasicInfo*, StringW beatmapJson)
{
    if (!beatmapJson)
    {
        return nullptr;
    }
    BeatmapSaveDataVersion4::BeatmapSaveData* beatmapSaveData = UnityEngine::JsonUtility::FromJson<BeatmapSaveDataVersion4::BeatmapSaveData*>(beatmapJson);
    if (beatmapSaveData == nullptr)
    {
        return nullptr;
    }
    int count = beatmapSaveData->___colorNotes.size();
    int count2 = 0;

    for (auto chainBeatIndex : beatmapSaveData->___chains)
    {
        count2 += beatmapSaveData->___chainsData[chainBeatIndex->ci].c - 1;
    }

    count2 += count;

    int count3 = beatmapSaveData->___obstacles.size();
    int count4 = beatmapSaveData->___bombNotes.size();

    std::vector<BetterSongList::Hooks::SimpleObstacle> obstacles;
    for (int i = 0; i < count3; i++)
    {
        auto beat = beatmapSaveData->___obstacles[i];
        auto obstacleData = beatmapSaveData->___obstaclesData[i];
        obstacles.push_back(BetterSongList::Hooks::SimpleObstacle(obstacleData.x, obstacleData.y, obstacleData.w, obstacleData.h, obstacleData.d, beat->b));
    }

    auto mark = BetterSongList::Hooks::HookBeatmapDataLoader::GetMarkStatus(obstacles);
    auto beatmapDataBasicInfo = GlobalNamespace::BeatmapDataBasicInfo::New_ctor(4, count, count2, count3 * (mark ? -1 : 1), count4);

    return beatmapDataBasicInfo;
}
