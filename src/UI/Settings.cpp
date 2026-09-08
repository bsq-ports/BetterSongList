#include "beatsaber-hook/shared/safeptr.hpp"
#include "UI/Settings.hpp"
#include "assets.hpp"
#include "Patches/UI/ExtraLevelParams.hpp"
#include "Patches/UI/ScrollEnhancement.hpp"

#include "bsml/shared/BSML.hpp"

DEFINE_TYPE(BetterSongList, Settings);

namespace BetterSongList {
    safe_ptr<Settings*, false> Settings::instance;

    Settings* Settings::get_instance() {
        if (!instance || !instance.ptr()) {
            instance.emplace(Settings::New_ctor());
        }
        return instance.ptr();
    }

    std::string Settings::get_version() {
        return "BetterSongList v" VERSION " port by RedBrumbler";
    }

    bool Settings::get_extendSongScrollbar() { return config.get_extendSongScrollbar(); }

    void Settings::set_extendSongScrollbar(bool value) {
        if (config.get_extendSongScrollbar() == value) return;
        config.set_extendSongScrollbar(value);
        Hooks::ScrollEnhancement::UpdateState();
    }

    bool Settings::get_showWarningIfMapHasCrouchWallsBecauseMappersThinkSprinklingThemInRandomlyIsFun() { return config.get_showWarningIfMapHasCrouchWallsBecauseMappersThinkSprinklingThemInRandomlyIsFun(); }

    void Settings::set_showWarningIfMapHasCrouchWallsBecauseMappersThinkSprinklingThemInRandomlyIsFun(bool value) {
        if (config.get_showWarningIfMapHasCrouchWallsBecauseMappersThinkSprinklingThemInRandomlyIsFun() == value) return;
        config.set_showWarningIfMapHasCrouchWallsBecauseMappersThinkSprinklingThemInRandomlyIsFun(value);
        Hooks::ExtraLevelParams::UpdateState();
    }

    float Settings::get_accuracyMultiplier() { return config.get_accuracyMultiplier(); }

    void Settings::set_accuracyMultiplier(float value) {
        if (config.get_accuracyMultiplier() == value) return;
        config.set_accuracyMultiplier(value);
        Hooks::ExtraLevelParams::UpdateState();
    }

    StringW Settings::get_preferredLeaderboard() { return config.get_preferredLeaderboard(); }

    void Settings::set_preferredLeaderboard(StringW value) {
        if (config.get_preferredLeaderboard() == value) return;
        config.set_preferredLeaderboard(value);
        Hooks::ExtraLevelParams::UpdateState();
    }

    void Settings::PostParse() {
        versionText->set_text(get_version());
        inited = true;
    }

    void Settings::OpenSponsorModal() {
        SettingsClosed();
        sponsorModal->Show();
        // TODO: sponsor text
    }

    void Settings::SettingsClosed() {
        settingsModal->Hide();
    }

    void Settings::Init(UnityEngine::Transform* parent) {
        if (inited && this->parent && this->parent->___m_CachedPtr.m_value) return;
        this->parent = parent;
        BSML::parse_and_construct(IncludedAssets::Settings_bsml, parent, reinterpret_cast<System::Object*>(this));
    }
}