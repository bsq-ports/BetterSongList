#include "hooking.hpp"
#include "config.hpp"

#include "Patches/HookFilterClear.hpp"
#include "GlobalNamespace/LevelSearchViewController.hpp"
#include "GlobalNamespace/LevelFilter.hpp"

// from HookFilterClear
MAKE_AUTO_HOOK_MATCH(LevelSearchViewController_ResetOptionFilterSettings, &GlobalNamespace::LevelSearchViewController::ResetOptionFilterSettings, void, GlobalNamespace::LevelSearchViewController* self, bool onlyFavorites) {
    BetterSongList::Hooks::HookFilterClear::LevelSearchViewController_ResetFilter_Prefix();
    LevelSearchViewController_ResetOptionFilterSettings(self, onlyFavorites);
}

MAKE_AUTO_HOOK_MATCH(LevelSearchViewController_Refresh, static_cast<void(GlobalNamespace::LevelSearchViewController::*)()>(&GlobalNamespace::LevelSearchViewController::RefreshAsync), void, GlobalNamespace::LevelSearchViewController* self) {
    if(config.get_autoFilterUnowned()) {
        self->____currentSearchFilter.songOwned = true;
        self->____currentSearchFilter.songNotOwned = false;
    }
    LevelSearchViewController_Refresh(self);
}
