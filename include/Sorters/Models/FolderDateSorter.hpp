#pragma once

#include "ISorter.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include <atomic>
#include <ctime>
#include <map>
#include <shared_mutex>

#include "songcore/shared/SongCore.hpp"
#include "songcore/shared/SongLoader/CustomBeatmapLevel.hpp"

namespace BetterSongList {
    class FolderDateSorter : public ISorterWithLegend, public ISorterPrimitive {
        public:
            FolderDateSorter();
            ~FolderDateSorter();
            virtual bool get_isReady() const override;
            virtual std::future<void> Prepare() override;
            std::future<void> Prepare(bool fullReload);
            virtual std::optional<float> GetValueFor(GlobalNamespace::BeatmapLevel* level) const override;
            virtual Legend BuildLegend(ArrayW<GlobalNamespace::BeatmapLevel*> levels) const override;
        private:
            void OnSongsLoaded(std::span<SongCore::SongLoader::CustomBeatmapLevel* const> customLevels);
            void OnSongWillBeDeleted(SongCore::SongLoader::CustomBeatmapLevel* customLevel);
            void GatherFolderInfoThread(bool fullReload = false);
            static std::map<std::string, std::time_t> songTimes;
            static std::shared_mutex songTimesMutex;
            static bool hasScanned;
            static std::atomic_bool isLoading;
            static bool eventsMapped;
    };
}
