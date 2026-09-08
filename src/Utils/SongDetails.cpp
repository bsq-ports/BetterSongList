#include "Utils/SongDetails.hpp"
#include "GlobalNamespace/BeatmapCharacteristicExtensions.hpp"
#include "logging.hpp"
#include "songcore/shared/SongCore.hpp"

#include <thread>

namespace BetterSongList::SongDetails {
    static SongDetailsCache::SongDetails* songDetails = nullptr;
    SongDetailsCache::SongDetails* get_songDetails() {
        return songDetails;
    }

    bool get_isAvailable() {
        return CheckAvailable();
    }

    bool CheckAvailable() {
        if (songDetails == nullptr)
            return false;

        return  songDetails->songs.get_isDataAvailable();
    }

    static bool finishedInitAttempt = false;
    bool get_finishedInitAttempt() {
        return finishedInitAttempt;
    }
    
    static bool attemptedToInit = false;
    bool get_attemptedToInit() {
        return attemptedToInit;
    }

    std::string GetUnavailabilityReason() {
        if (finishedInitAttempt && (!songDetails || !songDetails->songs.get_isDataAvailable() || songDetails->songs.size() == 0)) {
            return "Initialization failed";
        }
        return "";
    }

    void Init() {
        if (attemptedToInit) return;
        attemptedToInit = true;
        std::thread([](){
            DEBUG("Getting songdetails");
            songDetails = SongDetailsCache::SongDetails::Init().get();
            DEBUG("Got songdetails");


            if (!songDetails->songs.get_isDataAvailable()) {
                finishedInitAttempt = true;
                DEBUG("BSL Failed");
            } else {
                DEBUG("BSL Not failed Songs size:{}", songDetails->songs.size());
                finishedInitAttempt = true;
            }
        }).detach();
    }

        /// @brief Gets the song_data_core::BeatStarCharacteristics from a passed serialized char name
        /// @param serializedName the name to check for
        /// @return song_data_core::BeatStarCharacteristic of the name, or Unknown for invalid
        SongDetailsCache::MapCharacteristic StringToBeatStarCharacteristics(std::string_view serializedName)
        {
            if (serializedName.empty()) return SongDetailsCache::MapCharacteristic::Custom;
            if (serializedName == "90Degree") return SongDetailsCache::MapCharacteristic::NinetyDegree;
            if (serializedName == "360Degree") return SongDetailsCache::MapCharacteristic::ThreeSixtyDegree;

            switch(serializedName.data()[0])
            {
                case 's': [[fallthrough]];
                case 'S': return SongDetailsCache::MapCharacteristic::Standard;
                case 'o': [[fallthrough]];
                case 'O': return SongDetailsCache::MapCharacteristic::OneSaber;
                case 'n': [[fallthrough]];
                case 'N': return SongDetailsCache::MapCharacteristic::NoArrows;
                case 'd': [[fallthrough]];
                case 'D':
                    if (serializedName.size() > 6 && serializedName[6] == '9') return SongDetailsCache::MapCharacteristic::NinetyDegree;
                    else return SongDetailsCache::MapCharacteristic::ThreeSixtyDegree;
                case 'l': [[fallthrough]];
                case 'L': {
                    if (serializedName.size() > 1 && (serializedName[1] == 'a' || serializedName[1] == 'A')) return SongDetailsCache::MapCharacteristic::Lawless;
                    else return SongDetailsCache::MapCharacteristic::LightShow;
                }
                default: return SongDetailsCache::MapCharacteristic::Custom;
            }
        }

    std::string BeatmapCharacteristicToString(GlobalNamespace::BeatmapCharacteristic characteristic) {
        using Characteristic = GlobalNamespace::BeatmapCharacteristic;
        switch (characteristic.value__) {
            case Characteristic::Standard.value__:
            case Characteristic::OneSaber.value__:
            case Characteristic::NoArrows.value__:
            case Characteristic::Degree90.value__:
            case Characteristic::Degree360.value__:
            case Characteristic::Legacy.value__:
                return GlobalNamespace::BeatmapCharacteristicExtensions::SerializedName(characteristic);
        }
        auto custom = SongCore::API::Characteristics::GetCharacteristic(characteristic);
        return custom ? custom->serializedName : "";
    }

    SongDetailsCache::MapCharacteristic BeatmapCharacteristicToBeatStarCharacteristic(GlobalNamespace::BeatmapCharacteristic characteristic) {
        if (characteristic == GlobalNamespace::BeatmapCharacteristic::Legacy) return SongDetailsCache::MapCharacteristic::Custom;
        return StringToBeatStarCharacteristics(BeatmapCharacteristicToString(characteristic));
    }
}
