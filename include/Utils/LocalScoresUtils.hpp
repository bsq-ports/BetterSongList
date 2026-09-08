#pragma once

#include "GlobalNamespace/BeatmapLevel.hpp"
#include <future>
namespace BetterSongList::LocalScoresUtils {
    bool get_hasScores();
    bool HasLocalScore(GlobalNamespace::BeatmapLevel* level);
    std::shared_future<void> Load();
}
