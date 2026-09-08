#include "Utils/PlaylistUtils.hpp"

namespace BetterSongList::PlaylistUtils {
    bool get_hasPlaylistLib() {
        return true;
    }

    ArrayW<GlobalNamespace::BeatmapLevel*> GetLevelsForLevelCollection(GlobalNamespace::BeatmapLevelPack* levelCollection) {
        return levelCollection ? levelCollection->_beatmapLevels : ArrayW<GlobalNamespace::BeatmapLevel*>(il2cpp_array_size_t(0));
    }
}
