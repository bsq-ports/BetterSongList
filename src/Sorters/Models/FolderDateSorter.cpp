#include "Sorters/Models/FolderDateSorter.hpp"
#include "logging.hpp"

#include "Utils/SongListLegendBuilder.hpp"

#include "GlobalNamespace/BeatmapLevelPack.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include <mutex>
#include <shared_mutex>
#include <sys/stat.h>
#include <time.h>
#include <unordered_set>

static const float MONTH_SECS = 1.0f / (60 * 60 * 24 * 30.4f);

namespace BetterSongList {
    std::map<std::string, std::time_t> FolderDateSorter::songTimes;

    std::shared_mutex FolderDateSorter::songTimesMutex;

    std::atomic_bool FolderDateSorter::isLoading = false;
    bool FolderDateSorter::eventsMapped = false;

    FolderDateSorter::FolderDateSorter() : ISorterWithLegend(), ISorterPrimitive() { }

    FolderDateSorter::~FolderDateSorter() {
        if (this->eventsMapped) {
            SongCore::API::Loading::GetSongsLoadedEvent() -= {&FolderDateSorter::OnSongsLoaded, this};
            SongCore::API::Loading::GetSongWillBeDeletedEvent() -= {&FolderDateSorter::OnSongWillBeDeleted, this};
        }
    }

    bool FolderDateSorter::get_isReady() const {
        std::shared_lock<std::shared_mutex> lock(songTimesMutex);
        return !songTimes.empty();
    }

    std::future<void> FolderDateSorter::Prepare() {
        return Prepare(false);
    }

    void FolderDateSorter::OnSongsLoaded(std::span<SongCore::SongLoader::CustomBeatmapLevel* const> customLevels) {
        FolderDateSorter::GatherFolderInfoThread(false);
    }

    void FolderDateSorter::OnSongWillBeDeleted(SongCore::SongLoader::CustomBeatmapLevel* customLevel) {
        std::unique_lock<std::shared_mutex> lock(songTimesMutex);
        songTimes.erase(customLevel->___levelID);
    }

    void FolderDateSorter::GatherFolderInfoThread(bool fullReload) {
        auto levels = SongCore::API::Loading::GetAllLevels();

        // Build a set of known song IDs to avoid re-checking them
        std::unordered_set<std::string> knownSongIds;
        if (!fullReload) {
            std::shared_lock<std::shared_mutex> lock(songTimesMutex);
            knownSongIds.reserve(songTimes.size());
            for (const auto& songTime : songTimes) {
                knownSongIds.insert(songTime.first);
            }
        }

        // Gather folder info for new songs
        std::map<std::string, std::time_t> gatheredSongTimes;
        for (auto level : levels) {
            if (!level) continue;

            auto levelIdValue = level->___levelID;
            auto customLevelPath = level->get_customLevelPath();
            if (!levelIdValue || customLevelPath.empty()) continue;

            std::string levelID = levelIdValue;
            std::string filePath(customLevelPath);
            if (levelID.empty()) continue;
            if (knownSongIds.contains(levelID)) continue;

            struct stat fileStat = {0};
            if (stat(filePath.c_str(), &fileStat) == 0) {
                gatheredSongTimes[levelID] = fileStat.st_mtim.tv_sec;
            }
        }

        {
            std::unique_lock<std::shared_mutex> lock(songTimesMutex);
            for (const auto& [levelId, timestamp] : gatheredSongTimes) {
                songTimes[levelId] = timestamp;
            }
        }

    }

    std::future<void> FolderDateSorter::Prepare(bool fullReload) {
        bool expected = false;
        if (!isLoading.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
            return std::async(std::launch::deferred, []{});
        }

        return il2cpp_utils::il2cpp_async(std::launch::async, [fullReload, this](){
            il2cpp_utils::threading::OnScopeExit resetLoading([] {
                isLoading.store(false, std::memory_order_release);
            });

            if (this->eventsMapped == false) {
                SongCore::API::Loading::GetSongsLoadedEvent() += {&FolderDateSorter::OnSongsLoaded, this};
                SongCore::API::Loading::GetSongWillBeDeletedEvent() += {&FolderDateSorter::OnSongWillBeDeleted, this};

                this->eventsMapped = true;
            }

            auto hasLoaded = SongCore::API::Loading::AreSongsLoaded();
            while(!hasLoaded) std::this_thread::yield();
            
            this->FolderDateSorter::GatherFolderInfoThread(fullReload);
        });
    }

    std::optional<float> FolderDateSorter::GetValueFor(GlobalNamespace::BeatmapLevel* level) const {
		std::string levelId = level ? level->___levelID : "";
        if (levelId.empty()) return std::nullopt;
        std::shared_lock<std::shared_mutex> lock(songTimesMutex);
        auto itr = songTimes.find(levelId);
        if (itr != songTimes.end()) return itr->second;
        return std::nullopt;
    }

    ISorterWithLegend::Legend FolderDateSorter::BuildLegend(ArrayW<GlobalNamespace::BeatmapLevel*> levels) const {
        time_t now = time(NULL);
        return SongListLegendBuilder::BuildFor(levels, [now](GlobalNamespace::BeatmapLevel* level) -> std::string {
            std::string levelId = level ? level->___levelID : "";
            if (levelId.empty()) return "";
            std::shared_lock<std::shared_mutex> lock(songTimesMutex);
            auto itr = songTimes.find(levelId);
            if (itr == songTimes.end()) return "";
            auto months = (now - itr->second) * MONTH_SECS;

            if (months < 1.0f) return "<1 M";

            return fmt::format("{} M", (int)months);
        });
    }
}
