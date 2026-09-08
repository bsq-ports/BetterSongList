#pragma once

#include <cstdint>

#include "beatsaber-hook/shared/safeptr.hpp"

#include "custom-types/shared/coroutine.hpp"
#include "GlobalNamespace/LevelCollectionTableView.hpp"
#include "UnityEngine/GameObject.hpp"
#include "HMUI/TableView.hpp"

namespace BetterSongList::Hooks {
    class ScrollEnhancement {
        public:
            /// @brief no prio
            static void LevelCollectionTableView_Init_Prefix(GlobalNamespace::LevelCollectionTableView* self, bool isInitialized, HMUI::TableView* tableView);
            static void UpdateState();
            static void GameRestart();
        private:
            static UnityEngine::Transform* BuildButton(UnityEngine::Transform* baseButton, StringW Icon, float vOffs, float rotation, std::function<void()> cb);
            static custom_types::Helpers::Coroutine SetupExtraScrollButtons(safe_ptr<HMUI::TableView*> table, safe_ptr<UnityEngine::Transform*> a, std::uint64_t generation);
            static std::array<safe_ptr<UnityEngine::GameObject*>, 4> buttons;
            
    };
}
