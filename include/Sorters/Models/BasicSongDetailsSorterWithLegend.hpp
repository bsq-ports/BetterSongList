#pragma once

#include "ISorter.hpp"
#include "Utils/IAvailabilityCheck.hpp"
#include "song-details/shared/Data/Song.hpp"
#include "song-details/shared/SongDetails.hpp"

namespace BetterSongList {
    class BasicSongDetailsSorterWithLegend : public ISorterWithLegend, public ISorterCustom, public IAvailabilityCheck {
        public:
            using ValueGetterFunc = std::function<std::optional<double>(const SongDetailsCache::Song*)>;
            using LegendGetterFunc = std::function<std::string(const SongDetailsCache::Song*)>;
            BasicSongDetailsSorterWithLegend(ValueGetterFunc sortFunc);
            BasicSongDetailsSorterWithLegend(ValueGetterFunc sortFunc, LegendGetterFunc legendFunc);
            virtual bool get_isReady() const override;
            virtual std::future<void> Prepare() override;
            virtual Legend BuildLegend(ArrayW<GlobalNamespace::BeatmapLevel*> levels) const override;
            std::optional<double> GetValueFor(GlobalNamespace::BeatmapLevel* level) const;
            virtual void DoSort(ArrayW<GlobalNamespace::BeatmapLevel*>& levels, bool ascending) const override;
            virtual std::string GetUnavailableReason() const override;
            std::string DefaultLegendGetter(const SongDetailsCache::Song* song) const;
        private:
            ValueGetterFunc sortValueGetter;
            LegendGetterFunc legendValueGetter;
    };
}