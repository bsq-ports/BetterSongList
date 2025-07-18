#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include <string>

namespace BetterSongList::BeatmapUtils {
    std::string GetHashOfPreview(GlobalNamespace::BeatmapLevel* level);
    int GetCharacteristicFromDifficulty(GlobalNamespace::BeatmapKey diff);
    /// @brief Cleans the text by removing special characters and returning a clean string (copy of GlobalNamespace::LevelFilter::CleanText)
    /// @param text The text to clean
    /// @return A cleaned string with only alphanumeric characters, spaces, and hyphens
    std::string CleanText(std::string_view text);
}