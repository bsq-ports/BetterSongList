#include "Patches/HookSelectedCollection.hpp"
#include "config.hpp"
#include "logging.hpp"

#include "GlobalNamespace/BeatmapLevelPack.hpp"

#include "System/Collections/Generic/IReadOnlyList_1.hpp"

#include "songcore/shared/SongCore.hpp"

#include "UI/FilterUI.hpp"

namespace BetterSongList::Hooks {
    SafePtr<GlobalNamespace::BeatmapLevelPack> HookSelectedCollection::lastSelectedCollection;

    GlobalNamespace::BeatmapLevelPack* HookSelectedCollection::get_lastSelectedCollection() {
        if (!lastSelectedCollection) {
            return nullptr;
        }
        return lastSelectedCollection.ptr();
    }

    // same as CollectionSet
    void HookSelectedCollection::AnnotatedBeatmapLevelCollectionsViewController_HandleDidSelectAnnotatedBeatmapLevelCollection_Prefix(GlobalNamespace::BeatmapLevelPack* beatmapLevelCollection) {
        // Save the collection we're on for reselection purposes
        if (beatmapLevelCollection) {
            INFO("Setting last selected pack");
            auto name = beatmapLevelCollection->packID;
            config.set_lastPack(name ? static_cast<std::string>(name) : "");
            INFO("It is now '{}'", config.get_lastPack());
        }

        INFO("AnnotatedBeatmapLevelCollectionsViewController.HandleDidSelectAnnotatedBeatmapLevelCollection(): {0}", beatmapLevelCollection ? static_cast<std::string>(beatmapLevelCollection->packName) : "NULL");
        auto pack = SongCore::API::Loading::GetCustomLevelPack();

        // If its a playlist we want to start off with no sorting and filtering - Requested by Pixel
        auto instance = FilterUI::get_instance();
        if (beatmapLevelCollection && config.get_clearFiltersOnPlaylistSelect() && beatmapLevelCollection != pack) {
            instance->SetSort("", false, false);
            instance->SetFilter("", false, false);
        } else if (get_lastSelectedCollection()) {
            instance->SetSort(config.get_lastSort(), false, false);
            instance->SetFilter(config.get_lastFilter(), false, false);
        }

        lastSelectedCollection.emplace(beatmapLevelCollection);
        instance->UpdateTransformerOptionsAndDropdowns();
        DEBUG("EOF HandleDidSelectAnnotatedBeatmapLevelCollection:Prefix");
    }

    // HookLevelCollectionUnset
    void HookSelectedCollection::LevelFilteringNavigationController_UpdateSecondChildControllerContent_Prefix(GlobalNamespace::SelectLevelCategoryViewController::LevelCategory levelCategory) {
        if (levelCategory != GlobalNamespace::SelectLevelCategoryViewController::LevelCategory::CustomSongs) {
            AnnotatedBeatmapLevelCollectionsViewController_HandleDidSelectAnnotatedBeatmapLevelCollection_Prefix(nullptr);
        }
    }

    // HookLevelCollectionInit
    void HookSelectedCollection::AnnotatedBeatmapLevelCollectionsViewController_SetData_PostFix(GlobalNamespace::AnnotatedBeatmapLevelCollectionsViewController* self) {
        AnnotatedBeatmapLevelCollectionsViewController_HandleDidSelectAnnotatedBeatmapLevelCollection_Prefix(self->_annotatedBeatmapLevelCollections->get_Item(self->_selectedItemIndex));
    }
}
