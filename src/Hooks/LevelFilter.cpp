#include "hooking.hpp"

#include "Patches/ImproveBasegameSearch.hpp"
#include "logging.hpp"
#include "GlobalNamespace/LevelFilter.hpp"

// from ImproveBasegameSearch
MAKE_AUTO_HOOK_ORIG_MATCH(LevelFilter_FilterLevelByText, &GlobalNamespace::LevelFilter::FilterLevelByText, System::Collections::Generic::List_1<GlobalNamespace::BeatmapLevel*>*, System::Collections::Generic::List_1<GlobalNamespace::BeatmapLevel*>* levels, ArrayW<::StringW> searchTexts) {
    System::Collections::Generic::List_1<GlobalNamespace::BeatmapLevel*>* result = nullptr;
    if (BetterSongList::Hooks::ImproveBasegameSearch::LevelFilter_FilterLevelByText_Prefix(levels, searchTexts, result)) {
        if (!result) {
            DEBUG("ImproveBasegameSearch: No results found, returning original filter method");
        } else {
            DEBUG("ImproveBasegameSearch: Found {} results", result->get_Count());
            // Output the results for debugging
            for (int i = 0; i < result->get_Count(); ++i) {
                auto level = result->get_Item(i);
                DEBUG("Result {}: {} - {}", i + 1, static_cast<std::string>(level->___songName), static_cast<std::string>(level->___songSubName));
            }
        }
        return result;
    }

    return LevelFilter_FilterLevelByText(levels, searchTexts);
}