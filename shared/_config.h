#pragma once

#define BSL_EXPORT __attribute__((visibility("default")))
#ifdef __cplusplus
#define BSL_EXPORT_FUNC extern "C" BSL_EXPORT
#else
#define BSL_EXPORT_FUNC BSL_EXPORT
#endif
