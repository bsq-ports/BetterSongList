#pragma once

#include "beatsaber-hook/shared/safeptr.hpp"

#include "custom-types/shared/macros.hpp"
#include "bsml/shared/macros.hpp"
#include "bsml/shared/BSML/Components/ModalView.hpp"
#include "bsml/shared/BSML/Components/Backgroundable.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/Transform.hpp"
#include "config.hpp"

DECLARE_CLASS_CODEGEN(BetterSongList, Settings, Il2CppObject) {
    DECLARE_INSTANCE_METHOD(void, SettingsClosed);
    DECLARE_INSTANCE_METHOD(void, PostParse);
    DECLARE_INSTANCE_METHOD(void, OpenSponsorModal);
    DECLARE_INSTANCE_METHOD(void, Init, UnityEngine::Transform* parent);
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, parent);
    DECLARE_INSTANCE_FIELD(BSML::ModalView*, settingsModal);
    DECLARE_INSTANCE_FIELD(BSML::ModalView*, sponsorModal);
    DECLARE_INSTANCE_FIELD(BSML::Backgroundable*, settingsTitle);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, versionText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, sponsorText);
    DECLARE_INSTANCE_FIELD(bool, inited);

    BSML_PROPERTY_DEFINITION(bool, allowWipDelete);
    BSML_PROPERTY_DEFINITION(bool, autoFilterUnowned);
    DECLARE_INSTANCE_METHOD(bool, get_showWarningIfMapHasCrouchWallsBecauseMappersThinkSprinklingThemInRandomlyIsFun);
    DECLARE_INSTANCE_METHOD(void, set_showWarningIfMapHasCrouchWallsBecauseMappersThinkSprinklingThemInRandomlyIsFun, bool value);
    BSML_PROPERTY_DEFINITION(bool, clearFiltersOnPlaylistSelect);
    BSML_PROPERTY_DEFINITION(bool, modBasegameSearch);
    DECLARE_INSTANCE_METHOD(bool, get_extendSongScrollbar);
    DECLARE_INSTANCE_METHOD(void, set_extendSongScrollbar, bool value);
    BSML_PROPERTY_DEFINITION(bool, showMapJDInsteadOfOffset);
    DECLARE_INSTANCE_METHOD(float, get_accuracyMultiplier);
    DECLARE_INSTANCE_METHOD(void, set_accuracyMultiplier, float value);
    BSML_OPTIONS_LIST_OBJECT(preferredLeaderboardChoices, "ScoreSaber", "BeatLeader");
    DECLARE_INSTANCE_METHOD(StringW, get_preferredLeaderboard);
    DECLARE_INSTANCE_METHOD(void, set_preferredLeaderboard, StringW value);

    DECLARE_DEFAULT_CTOR();
    public:
        static std::string get_version();
        static Settings* get_instance();
    private:
        static safe_ptr<Settings*, false> instance;
};
