#include "Patches/ImproveBasegameSearch.hpp"
#include "config.hpp"

namespace BetterSongList::Hooks {

    bool filterLiver(GlobalNamespace::BeatmapLevel* level, ArrayW<StringW>& searchTexts) {
        bool result = false;
        
        for (auto s : searchTexts) {
            auto str = static_cast<std::u16string_view>(s);
            if (str.size() > 2) {
                auto authorName = static_cast<std::u16string_view>(level->songAuthorName);

                // try to find the string in author name
                auto match = std::search(authorName.begin(), authorName.end(), str.begin(), str.end(), [](char16_t c1, char16_t c2) {
                    return std::toupper(c1) == std::toupper(c2);
                });

                // if match == first end, it was not found, otherwise it was found
                if (match != authorName.end()) {
                    result = true;
                    return false;
                }
            }
        }

        if (searchTexts.size() > 1) {
            searchTexts = ArrayW<StringW>(il2cpp_array_size_t(1));
            //searchTexts[0] = Il2CppString::Join(" ", searchTexts.convert());
        }

        return result;
    }
    bool ImproveBasegameSearch::LevelFilter_FilterLevelByText_Prefix(System::Collections::Generic::List_1<GlobalNamespace::BeatmapLevel*>* levels, ArrayW<StringW>& searchTexts, System::Collections::Generic::List_1<GlobalNamespace::BeatmapLevel*>*& result) {
        if (!config.get_modBasegameSearch()) return true;

        ListW<GlobalNamespace::BeatmapLevel*> resultLevels = ListW<GlobalNamespace::BeatmapLevel*>();
        
        for (size_t i = 0; i < levels->Count; i++)
        {
            auto level = levels->get_Item(i);
            if (filterLiver(level, searchTexts)) {
                resultLevels->Add(level);
            }
        }
        
        result = resultLevels;

        return true;
    }
}