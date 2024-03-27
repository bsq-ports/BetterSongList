#include "hooking.hpp"

#include "Patches/HookFilterClear.hpp"
#include "GlobalNamespace/LevelSearchViewController.hpp"

// from HookFilterClear
MAKE_AUTO_HOOK_MATCH(LevelSearchViewController_ResetFilter, &GlobalNamespace::LevelSearchViewController::ResetFilter, void, GlobalNamespace::LevelSearchViewController* self, bool onlyFavorites) {
    BetterSongList::Hooks::HookFilterClear::LevelSearchViewController_ResetFilter_Prefix();
    LevelSearchViewController_ResetFilter(self, onlyFavorites);
}
