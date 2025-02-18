#include "Patches/RestoreTableScroll.hpp"
#include "config.hpp"
#include "logging.hpp"

#include "GlobalNamespace/BeatmapLevel.hpp"
#include "System/Tuple_2.hpp"
#include "HMUI/TableView.hpp"

template <> struct fmt::formatter<std::optional<int>> : formatter<string_view> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(std::optional<int> o, FormatContext& ctx) {
        if (o.has_value()) {
            return formatter<string_view>::format(std::to_string(o.value()), ctx);
        } else {
            return formatter<string_view>::format("nullopt", ctx);
        }
    }
};


namespace BetterSongList::Hooks {
    std::optional<int> RestoreTableScroll::scrollToIndex = std::nullopt;
    bool RestoreTableScroll::doResetScrollOnNext = false;

    void RestoreTableScroll::ResetScroll() {
        scrollToIndex = 0;
        doResetScrollOnNext = true;

		INFO("RestoreTableScroll.ResetScroll()");
    }

    void RestoreTableScroll::LevelCollectionTableView_Init_Prefix(GlobalNamespace::LevelCollectionTableView* self) {
        if (self->_isInitialized && self->_tableView != nullptr && self->_beatmapLevels != nullptr && !doResetScrollOnNext)
            scrollToIndex = self->_tableView->GetVisibleCellsIdRange()->get_Item1();

        doResetScrollOnNext = false;
		INFO("LevelCollectionTableView.Init():Prefix - scrollToIndex: {}", scrollToIndex.value_or(-1));
    }

    void RestoreTableScroll::LevelCollectionTableView_SetData_PostFix(GlobalNamespace::LevelCollectionTableView* self) {
		INFO("DoTheFunnySelect -> LevelCollectionTableView.SetData():Postfix scrollToIndex: {}", RestoreTableScroll::scrollToIndex.value_or(-1));
        auto tableView = self->_tableView;
        auto previewBeatmapLevels = self->_beatmapLevels;
        auto showLevelPackHeader = self->_showLevelPackHeader;

        if (!RestoreTableScroll::scrollToIndex.has_value() || RestoreTableScroll::scrollToIndex.value() < 0) return;

		INFO("-> Scrolling to {}", RestoreTableScroll::scrollToIndex.value_or(-1));

        tableView->ScrollToCellWithIdx(
            RestoreTableScroll::scrollToIndex.value(),
            HMUI::TableView::ScrollPositionType::Beginning,
            false
        );
        INFO("Scroll done");
    }
}
