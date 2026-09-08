#pragma once

#include <string_view>

namespace BetterSongList::FileUtils {
    // Replace the destination only after a complete write, sync, and close succeed.
    bool WriteFileAtomic(std::string_view path, std::string_view contents);
}
