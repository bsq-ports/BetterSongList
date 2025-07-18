#include "hooking.hpp"

#include "Patches/ImproveBasegameSearch.hpp"
#include "logging.hpp"
#include "GlobalNamespace/LevelFilter.hpp"

// from ImproveBasegameSearch
MAKE_AUTO_HOOK_ORIG_MATCH(LevelFilter_FilterLevelByText, &GlobalNamespace::LevelFilter::FilterLevelByText, System::Collections::Generic::List_1<GlobalNamespace::BeatmapLevel*>*, System::Collections::Generic::List_1<GlobalNamespace::BeatmapLevel*>* levels, ArrayW<::StringW> searchTexts) {
    // WARNING quest version of it is not using the return value, it uses the original by reference so we havve to modify it in place
    if (BetterSongList::Hooks::ImproveBasegameSearch::LevelFilter_FilterLevelByText_Prefix(levels, searchTexts, levels)) {
        return levels;
    }

    return LevelFilter_FilterLevelByText(levels, searchTexts);
}