#include "Sorters/SortMethods.hpp"
#include "logging.hpp"
#include "config.hpp"

#include "Sorters/Models/BasicSongDetailsSorterWithLegend.hpp"
#include "Sorters/Models/FolderDateSorter.hpp"
#include "Sorters/Models/FunctionSorter.hpp"


namespace BetterSongList {
    std::string toLowercase(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return out;
    }

    static std::string joinArray(ArrayW<StringW>* array, std::string delimiter) {
        if (array->size() == 0) {
            return "";
        }

        std::string output = "";

        for (auto item : *array) {
            if (output != "") {
                output = output + delimiter;
            }

            output = output + std::string(item);
        }

        return output;
    }

    static ComparableFunctionSorterWithLegend alphabeticalSongname(
        [](auto a, auto b) -> int {
            return toLowercase(a->songName) < toLowercase(b->songName);
        },
        [](GlobalNamespace::BeatmapLevel* song) -> std::string {
            std::string songName = song->songName;
            return songName.size() > 0 ? toLowercase(songName.substr(0, 1)) : "";
        }
    );

    static ComparableFunctionSorterWithLegend alphabeticalSongAuthorName(
        [](auto a, auto b) -> int {
            std::string authorA = toLowercase(
                std::string(a->songAuthorName).empty()
                ? (a->allMappers.size() > 0 ? static_cast<std::string>(a->allMappers[0]) : "")
                : static_cast<std::string>(a->songAuthorName)
            );
            std::string authorB = toLowercase(
                std::string(b->songAuthorName).empty()
                ? (b->allMappers.size() > 0 ? static_cast<std::string>(b->allMappers[0]) : "")
                : static_cast<std::string>(b->songAuthorName)
            );

            if (authorA == authorB) {
                return toLowercase(a->songName) < toLowercase(b->songName);
            }

            return authorA < authorB;
        },
        [](GlobalNamespace::BeatmapLevel* song) -> std::string {
            std::string authorString = song->songAuthorName;

            if (authorString.empty()) {
                if (song->allMappers.size() > 0) {
                    authorString = static_cast<std::string>(song->allMappers[0]);
                } else {
                    return "";
                }
            }

            if (authorString.empty()) {
                return "";
            }

            return toLowercase(authorString.substr(0, 1));
        }
    );

    static PrimitiveFunctionSorterWithLegend bpm(
        [](auto song){
            return song->beatsPerMinute;
        },
        [](auto song){
            return fmt::format("{}", std::round(song->beatsPerMinute));
        }
    );

    static ComparableFunctionSorterWithLegend alphabeticalMapper(
        [](auto a, auto b) -> int {
            return toLowercase(joinArray(&a->allMappers, "|")) < toLowercase(joinArray(&b->allMappers, "|"));
        },
        [](auto song) -> std::string {
            std::string levelAuthor{static_cast<std::string>(song->allMappers.size() > 0 ? song->allMappers[0] : "")};
            return levelAuthor.size() > 0 ? toLowercase(levelAuthor.substr(0, 1)) : "";
        }
    );

    static FolderDateSorter downloadTime{};

    static std::optional<float> StarsProcessor(const SongDetailsCache::Song* song) {
        float ret = config.get_sortAsc() ? song->maxStarSS() : song->minStarSS();
        if (ret == 0) return std::nullopt;
        return ret;
    }

    static std::optional<float> StarsProcessorBL(const SongDetailsCache::Song* song) {
        float ret = config.get_sortAsc() ? song->maxStarBL() : song->minStarBL();
        if (ret == 0) return std::nullopt;
        return ret;
    }

    static BasicSongDetailsSorterWithLegend stars(
        [](auto song){ return StarsProcessor(song); },
        [](auto song) -> std::string {
            auto y = StarsProcessor(song);
            if (!y.has_value()) return "N/A";

            return fmt::format("{:.1f}", y.value());
        }
    );

    // TODO: If beatleader gets integrated, remove this
    static BasicSongDetailsSorterWithLegend blstars(
        [](auto song){ return StarsProcessorBL(song); },
        [](auto song) -> std::string {
            auto y = StarsProcessorBL(song);
            if (!y.has_value()) return "N/A";

            return fmt::format("{:.1f}", y.value());
        }
    );

    static const float second = 1.0f / 60.0f;

    static PrimitiveFunctionSorterWithLegend songLength(
        [](auto song){ return song->songDuration; },
        [](auto song) -> std::string {
            float duration = song->songDuration;
            if (duration < 60) return "<1";
            return fmt::format("{}", (int)std::round(duration * second));
        }
    );

    static BasicSongDetailsSorterWithLegend beatSaverDate(
        [](const SongDetailsCache::Song* song) -> std::optional<float> { 
            return song->uploadTimeUnix;
        },
        [](const SongDetailsCache::Song* song) -> std::string {
            auto uploaded = song->uploadTimeUnix;
            auto time = (time_t) uploaded;
            struct tm* tm = localtime(&time);
            // since int division is floored, we can get an index for quarter by removing 1 and dividing by 3,
            // then we just add 1 back to get from 0-3 to 1-4
            auto q = ((tm->tm_mon - 1) / 3) + 1;

            // tm year is years since 1900, if we want years since 2000 we do -100
            // ex: 2022 is 122 since 1900, so 122 - 100 = 22 which is what we want to display
            auto y = tm->tm_year - 100;
            return fmt::format("Q{:1d} {:2d}", q, y);
        }
    );

    const std::map<std::string, ISorter*>& SortMethods::get_methods() {
        return methods;
    }

    bool SortMethods::Register(ITransformerPlugin* sorter) {
        auto name = sorter->get_name();
        if (name.size() > 20) {
            name = name.substr(0, 20);
            ERROR("Name of the Transformer can not be more than 20 characters!");
        }

        if (!config.get_allowPluginSortsAndFilters()) return false;
        name = "P " + name;

        auto itr = methods.find(name);
        if (itr != methods.end()) {
            ERROR("sorter with name {} was already registered!", name);
            return false;
        }

        methods.insert({ name, sorter });
        return true;
    }

    std::map<std::string, ISorter*> SortMethods::methods{
        {"Song Name", &alphabeticalSongname},
        {"Song Author Name", &alphabeticalSongAuthorName},
        {"Mapper Name", &alphabeticalMapper},
        {"Download Date", &downloadTime},
        {"BL Stars", &blstars},
        {"SS Stars", &stars},
        {"Song Length", &songLength},
        {"BPM", &bpm},
        {"BeatSaver Date", &beatSaverDate},
        {"Default", nullptr}
    };
}
