#include "Patches/UI/BottomUI.hpp"

#include "UnityEngine/Transform.hpp"
#include "UnityEngine/WaitForEndOfFrame.hpp"
#include "bsml/shared/BSML/SharedCoroutineStarter.hpp"

#include "UI/FilterUI.hpp"

namespace BetterSongList::Hooks {
    void BottomUI::LevelCollectionNavigationController_DidActivate_Prefix(GlobalNamespace::LevelCollectionNavigationController* self, bool firstActivation) {
        auto starter = BSML::SharedCoroutineStarter::get_instance();
        auto t = self->get_transform().ptr();
        starter->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(FixPos(t)));

        if (!firstActivation) return;

        starter->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(InitDelayed(t)));
    }

    custom_types::Helpers::Coroutine BottomUI::FixPos(safe_ptr<UnityEngine::Transform*> t) {
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(UnityEngine::WaitForEndOfFrame::New_ctor());
        if (!t) co_return;
        t->set_localPosition({0, -7, 0});
        co_return;
    }

    custom_types::Helpers::Coroutine BottomUI::InitDelayed(safe_ptr<UnityEngine::Transform*> t) {
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(UnityEngine::WaitForEndOfFrame::New_ctor());
        if (!t) co_return;
        if (auto parent = t->get_parent()) FilterUI::AttachTo(parent);
        co_return;
    }
}
