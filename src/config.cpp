#include "config.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "reflectcpp/include/rfl/json.hpp"
#include "logging.hpp"

Config config;

#define Save(identifier) doc[#identifier] = config.identifier

void Config::SaveConfig() {
    INFO("Saving Configuration...");
    rfl::Generic::Object doc;
    Save(lastSong);
    Save(lastPack);
    Save(lastSort);
    Save(lastFilter);
    doc["lastCategory"] = config.get_lastCategory().value__;
    Save(enableAlphabetScrollBar);
    Save(clearFiltersOnPlaylistSelect);
    Save(modBasegameSearch);
    Save(autoFilterUnowned);
    Save(extendSongScrollbar);
    Save(allowWipDelete);
    Save(showWarningIfMapHasCrouchWallsBecauseMappersThinkSprinklingThemInRandomlyIsFun);
    Save(showMapJDInsteadOfOffset);
    Save(accuracyMultiplier);
    Save(allowPluginSortsAndFilters);
    Save(sortAsc);
    Save(settingsSeenInVersion);
    Save(preferredLeaderboard);

    try {
        if (!writefile(get_config_path(MOD_ID), rfl::json::write(doc))) {
            ERROR("Failed to write configuration");
            return;
        }
    } catch (std::exception const& error) {
        ERROR("Failed to serialize configuration: {}", error.what());
        return;
    }
    INFO("Saved Configuration!");
}

#define Load(identifier, type)                                                     \
    if (auto value = doc.get(#identifier).and_then([](rfl::Generic const& field) {   \
        return rfl::from_generic<type>(field);                                     \
    })) {                                                                         \
        config.identifier = *value;                                               \
    } else {                                                                      \
        foundEverything = false;                                                  \
    }


bool Config::LoadConfig() {
    INFO("Loading Configuration...");
    bool foundEverything = true;
    auto parsed = rfl::json::read<rfl::Generic::Object>(readfile(get_config_path(MOD_ID)));
    if (!parsed) {
        ERROR("Failed to parse configuration: {}", parsed.error().what());
        return false;
    }
    auto const& doc = *parsed;

    Load(lastSong, std::string);
    Load(lastPack, std::string);
    Load(lastSort, std::string);
    Load(lastFilter, std::string);
    Load(lastCategory, int);
    Load(enableAlphabetScrollBar, bool);
    Load(clearFiltersOnPlaylistSelect, bool);
    Load(modBasegameSearch, bool);
    Load(autoFilterUnowned, bool);
    Load(extendSongScrollbar, bool);
    Load(allowWipDelete, bool);
    Load(showWarningIfMapHasCrouchWallsBecauseMappersThinkSprinklingThemInRandomlyIsFun, bool);
    Load(showMapJDInsteadOfOffset, bool);
    Load(accuracyMultiplier, float);
    Load(allowPluginSortsAndFilters, bool);
    Load(sortAsc, bool);
    Load(settingsSeenInVersion, std::string);
    Load(preferredLeaderboard, std::string);

    if (foundEverything)
        INFO("Loaded Configuration!");
    return foundEverything;
}

void SaveConfig() {
    config.SaveConfig();
}

bool LoadConfig() {
    return config.LoadConfig();
}