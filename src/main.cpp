#include "scotland2/shared/loader.hpp"
#include "hooking.hpp"
#include "logging.hpp"

#include "custom-types/shared/register.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSMLDataCache.hpp"
#include "assets.hpp"
#include "config.hpp"
#include "song-details/shared/SongDetails.hpp"
#include "song-details/shared/SongArray.hpp"

#include "Utils/SongDetails.hpp"

#include "_config.h"

BSL_EXPORT_FUNC void setup(CModInfo* info) {
    info->id = MOD_ID;
    info->version = VERSION;
    info->version_long = 0;
}

BSL_EXPORT_FUNC void late_load() {
    i2c::functions::initialize();
    BSML::Init();
    custom_types::Register::AutoRegister();

    if (!LoadConfig()) SaveConfig();

    BetterSongList::Hooking::InstallHooks();
    BetterSongList::SongDetails::Init();
}

BSML_DATACACHE(double_arrow) {
    return IncludedAssets::DoubleArrowIcon_png;
}

BSML_DATACACHE(carat_up) {
    return IncludedAssets::CaratUp_png;
}

BSML_DATACACHE(carat_down) {
    return IncludedAssets::CaratDown_png;
}
