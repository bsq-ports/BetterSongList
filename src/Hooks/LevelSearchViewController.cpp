#include "hooking.hpp"
#include "config.hpp"
#include "beatsaber-hook/shared/utils.hpp"

#include "Patches/HookFilterClear.hpp"
#include "GlobalNamespace/LevelSearchViewController.hpp"
#include "GlobalNamespace/LevelFilter.hpp"

namespace {
    GlobalNamespace::LevelSearchViewController* resettingAllFilters = nullptr;
}

// Category changes reset the game's filters; preserve the BSL filter restored for that category.
MAKE_AUTO_HOOK_MATCH(LevelSearchViewController_ResetAllFilterSettings, &GlobalNamespace::LevelSearchViewController::ResetAllFilterSettings, void, GlobalNamespace::LevelSearchViewController* self, bool onlyFavorites) {
    auto* previous = resettingAllFilters;
    i2c::on_scope_exit restore([previous] { resettingAllFilters = previous; });
    resettingAllFilters = self;
    LevelSearchViewController_ResetAllFilterSettings(self, onlyFavorites);
}

// Direct option resets, including the Clear Filters button, still clear the BSL filter.
MAKE_AUTO_HOOK_MATCH(LevelSearchViewController_ResetOptionFilterSettings, &GlobalNamespace::LevelSearchViewController::ResetOptionFilterSettings, void, GlobalNamespace::LevelSearchViewController* self, bool onlyFavorites) {
    if (resettingAllFilters != self) {
        BetterSongList::Hooks::HookFilterClear::LevelSearchViewController_ResetFilter_Prefix();
    }
    LevelSearchViewController_ResetOptionFilterSettings(self, onlyFavorites);
}

MAKE_AUTO_HOOK_MATCH(LevelSearchViewController_Refresh, static_cast<void(GlobalNamespace::LevelSearchViewController::*)()>(&GlobalNamespace::LevelSearchViewController::RefreshAsync), void, GlobalNamespace::LevelSearchViewController* self) {
    if(config.get_autoFilterUnowned()) {
        self->____currentSearchFilter.songOwned = true;
        self->____currentSearchFilter.songNotOwned = false;
    }
    LevelSearchViewController_Refresh(self);
}
