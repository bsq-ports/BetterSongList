#include "Patches/ImproveBasegameSearch.hpp"
#include "config.hpp"
#include "GlobalNamespace/LevelFilter.hpp"
#include "Utils/BeatmapUtils.hpp"
#include "Sorters/SortMethods.hpp"
#include <fmt/ranges.h>
#include "logging.hpp"

#include <map>
#include <string>
#include <vector>
namespace BetterSongList::Hooks {
    bool ImproveBasegameSearch::LevelFilter_FilterLevelByText_Prefix(System::Collections::Generic::List_1<GlobalNamespace::BeatmapLevel*>* levels, ArrayW<StringW>& searchTerms, System::Collections::Generic::List_1<GlobalNamespace::BeatmapLevel*>*& result) {
        if (!config.get_modBasegameSearch()) return false;

        auto intermediate = std::vector<std::pair<int, GlobalNamespace::BeatmapLevel*>>{};

        // Convert arrayw of StringW to std::vector<std::string>
        std::vector<std::string> searchTermsVec;
        searchTermsVec.reserve(searchTerms.size());
        for (const auto& term : searchTerms) {
            searchTermsVec.push_back(static_cast<std::string>(term));
        }
        auto preparedSearchString = BetterSongList::toLowercase(fmt::format("{}", fmt::join(searchTermsVec, " ")));
        DEBUG("Searching for: {}", preparedSearchString.c_str());

        auto count = levels->get_Count();
        for(auto i = 0; i < count; i++) {
            auto level = levels->get_Item(i);

            std::vector<std::string> allMappersVec;
            allMappersVec.reserve(level->___allMappers.size());
            for (const auto& mapper : level->___allMappers) {
                allMappersVec.push_back(static_cast<std::string>(mapper));
            }

            auto txtToClean = fmt::format("{} {} {} {}", (std::string) level->___songName, (std::string) level->___songSubName, (std::string) level->___songAuthorName, fmt::join(allMappersVec, " "));
            auto cleanedTextLowercase = BetterSongList::toLowercase(BeatmapUtils::CleanText(txtToClean));
            
            auto score = BeatmapUtils::CalculateMatchScore(cleanedTextLowercase, preparedSearchString);
            if(score > 0) {
                DEBUG("Found match for {} with score {}",
                      fmt::format("{} - {}", (std::string) level->___songName, (std::string) level->___songSubName).c_str(), score);
                intermediate.push_back({score, level});
            }
        }

        //sort the intermediate list by score
        std::sort(intermediate.begin(), intermediate.end(), [](auto& a, auto &b) {
            return a.first > b.first;
        });

        result->Clear();

        for(auto& [score, level] : intermediate) {
            result->Add(level);
        }
        return true;
    }
}
