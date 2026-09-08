#include "Filters/Models/PlayedFilter.hpp"
#include "Utils/LocalScoresUtils.hpp"

namespace BetterSongList {
    PlayedFilter::PlayedFilter(bool unplayed) 
        : IFilter(), intendedPlayedState(!unplayed) {
        
    }

    bool PlayedFilter::get_isReady() const { 
        return LocalScoresUtils::get_hasScores(); 
    }

    std::future<void> PlayedFilter::Prepare() {
        return std::async(std::launch::deferred, [load = LocalScoresUtils::Load()] {
            load.get();
        });
    }

    bool PlayedFilter::GetValueFor(GlobalNamespace::BeatmapLevel* level) { 
        if (!get_isReady())
            return true;

        return LocalScoresUtils::HasLocalScore(level) == intendedPlayedState;
    }
}
