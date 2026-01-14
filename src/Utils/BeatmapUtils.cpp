#include "Utils/BeatmapUtils.hpp"
#include "Sorters/SortMethods.hpp"
#include "logging.hpp"
#include <regex>
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"

namespace BetterSongList::BeatmapUtils {
    int indexOf(const std::string& haystack, const std::string& needle) {
        size_t pos = haystack.find(needle);
        return (pos != std::string::npos) ? static_cast<int>(pos) : -1;
    }


    // GlobalNamespace::LevelFilter::_FilterLevelByText_g__CalculateMatchScore_15_1
    int CalculateMatchScore(const std::string& levelString, const std::string& text) {
        int score = 0;

        int foundIndex = indexOf(levelString, text);
        if (foundIndex >= 0) {
            int num3 = (
                // If the found index is 0 or the character before it is a space, we add 1 to the score
                (foundIndex == 0 || std::isspace(static_cast<unsigned char>(levelString[foundIndex - 1])))
                    ? 1 : 0)
                    // we add the length of the text to the score
                + 50 * static_cast<int>(text.length());
            score += num3;
        }
        return score;
    }

    std::string GetHashOfPreview(GlobalNamespace::BeatmapLevel* level) {
        if (!level) return "";
        auto levelId = static_cast<std::string>(level->___levelID);
        if (levelId.size() < 53) return "";
        if (levelId[12] != '_') return "";
        return levelId.substr(13, 40);
    }
    
    int GetCharacteristicFromDifficulty(GlobalNamespace::BeatmapKey diff) {
        auto chara = diff.beatmapCharacteristic;
        auto d = chara ? chara->get_sortingOrder() : 0;
        if (d == 0 || d > 4) return 0;

        if (d == 3)
            d = 4;
        else if (d == 4)
            d = 3;

        return d + 1;
    }

    std::string CleanText(std::string_view text) {
      static const std::regex specialCharsRegex("[^A-Za-z0-9 -]");
      std::string result = std::regex_replace(std::string(text), specialCharsRegex, "");
      return result;
    }
}