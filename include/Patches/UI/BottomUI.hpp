#pragma once

#include "GlobalNamespace/LevelCollectionNavigationController.hpp"
#include "UnityEngine/Transform.hpp"
#include "beatsaber-hook/shared/safeptr.hpp"
#include "custom-types/shared/coroutine.hpp"

namespace BetterSongList::Hooks {
    class BottomUI {
        public:
            /// @brief prio int max value
            static void LevelCollectionNavigationController_DidActivate_Prefix(GlobalNamespace::LevelCollectionNavigationController* self, bool firstActivation);
        
        private:
            static custom_types::Helpers::Coroutine FixPos(safe_ptr<UnityEngine::Transform*> t);
            static custom_types::Helpers::Coroutine InitDelayed(safe_ptr<UnityEngine::Transform*> t);
    };
}
