#include "Filters/Models/RequirementsFilter.hpp"
#include "logging.hpp"

#include "songcore/shared/SongCore.hpp"
#include "songcore/shared/CustomJSONData.hpp"

#include "System/Threading/Tasks/Task_1.hpp"
#include "Utils/BeatmapUtils.hpp"

#include "songcore/shared/SongLoader/CustomBeatmapLevel.hpp"


namespace BetterSongList {
    bool RequirementsFilter::inited = false;

    RequirementsFilter::RequirementsFilter(bool invert) 
        : IFilter(), invert(invert) {}

    bool RequirementsFilter::get_isReady() const {
        return SongCore::API::Loading::AreSongsLoaded();
    }

    std::future<void> RequirementsFilter::Prepare() {
        return std::async(std::launch::async, [this](){
            while (!SongCore::API::Loading::AreSongsLoaded()) std::this_thread::yield();
        });
    }

    bool RequirementsFilter::GetValueFor(GlobalNamespace::BeatmapLevel* level) {
        auto customLevel = i2c::try_cast<SongCore::SongLoader::CustomBeatmapLevel*>(level);
        if (!customLevel) {
            DEBUG("Level was not custom level!");
            return invert;
        }

        auto customData = customLevel->get_CustomSaveDataInfo();
        if (!customData) {
            DEBUG("Level had no custom save data!");
            return invert;
        }

        auto levelDetails = customData->get().TryGetBasicLevelDetails();
        if (!levelDetails) return invert;

        for (const auto& [characteristic, set] : levelDetails->get().characteristicNameToBeatmapDetailsSet) {
            for (const auto& [difficulty, details] : set.difficultyToDifficultyBeatmapDetails) {
                if (!details.requirements.empty()) return !invert;
            }
        }
        return invert;
    }
}
