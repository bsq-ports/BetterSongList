#pragma once

#include "beatsaber-hook/shared/hooking.hpp"

namespace BetterSongList {
    class Hooking {
    private:
        inline static std::vector<void (*)()> installFuncs;

    public:
        static void AddInstallFunc(void (*installFunc)()) {
            installFuncs.push_back(installFunc);
        }

        static void InstallHooks() {
            for (auto func : installFuncs) func();
        }
    };
}

#define BSL_AUTO_HOOK(name_, method, retval, installer, ...)                         \
    struct Auto_Hook_##name_ {                                                      \
        static void Install();                                                     \
        Auto_Hook_##name_() { BetterSongList::Hooking::AddInstallFunc(Install); }     \
    };                                                                             \
    static Auto_Hook_##name_ Auto_Hook_Instance_##name_;                             \
    struct hook_##name_ {                                                          \
        static constexpr auto cast_test = []<typename T>() { return requires { static_cast<T>(method); }; }; \
        using func_t = retval (*)(__VA_ARGS__);                                    \
        using cast_t = ::i2c::detail::method_check<cast_test, func_t>::type;          \
        static_assert(cast_test.operator()<cast_t>(), "Hook method signature does not match!"); \
        static_assert(::i2c::detail::match_hookable<static_cast<cast_t>(method)>, "Method cannot be hooked!"); \
        __INTERNAL_HOOK_STRUCT(name_, ::i2c::metadata_getter<static_cast<cast_t>(method)>::method_info(), retval, __VA_ARGS__) \
    };                                                                             \
    void Auto_Hook_##name_::Install() {                                             \
        static constexpr auto logger = Paper::ConstLoggerContext(MOD_ID);           \
        installer<hook_##name_>(logger);                                           \
    }                                                                              \
    retval hook_##name_::hook_m_##name_(__VA_ARGS__)

#define MAKE_AUTO_HOOK_MATCH(name_, method, retval, ...) \
    BSL_AUTO_HOOK(name_, method, retval, ::i2c::install_hook, __VA_ARGS__)

#define MAKE_AUTO_HOOK_ORIG_MATCH(name_, method, retval, ...) \
    BSL_AUTO_HOOK(name_, method, retval, ::i2c::install_hook_orig, __VA_ARGS__)
