#include "Filters/Models/BasicSongDetailsFilter.hpp"

#include "Utils/SongDetails.hpp"
#include "Utils/BeatmapUtils.hpp"
#include "song-details/shared/SongArray.hpp"
#include "song-details/shared/SongDetails.hpp"

#include <stdexcept>

namespace BetterSongList {
    BasicSongDetailsFilter::BasicSongDetailsFilter(const std::function<bool(const SongDetailsCache::Song*)>& func) 
        : IFilter(), IAvailabilityCheck(), filterValueTransformer(func) {

    }

    bool BasicSongDetailsFilter::get_isReady() const { 
        return SongDetails::get_finishedInitAttempt() && SongDetails::GetUnavailabilityReason().empty();
    }

    std::future<void> BasicSongDetailsFilter::Prepare() {
        return std::async(std::launch::async, []{
            SongDetails::Init();

            while(!SongDetails::get_finishedInitAttempt()) {
                std::this_thread::yield();
            }
            if (auto reason = SongDetails::GetUnavailabilityReason(); !reason.empty()) {
                throw std::runtime_error(reason);
            }
        });
    }
    
    bool BasicSongDetailsFilter::GetValueFor(GlobalNamespace::BeatmapLevel* level) {
        if (!SongDetails::get_songDetails()->songs.get_isDataAvailable()) {
            return false;
        }
        if (SongDetails::get_songDetails()->songs.size()==0) {
            return false;
        }

        const std::string h = BeatmapUtils::GetHashOfPreview(level);
        if (h.empty()) return false;

        auto &song = SongDetails::get_songDetails()->songs.FindByHash(h);
        if (song == SongDetailsCache::Song::none) return false;

        return filterValueTransformer(&song);
    }

    std::string BasicSongDetailsFilter::GetUnavailableReason() const { 
        return SongDetails::GetUnavailabilityReason(); 
    }
}