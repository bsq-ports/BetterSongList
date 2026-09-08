#include "Utils/FileUtils.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>
#include <unistd.h>

namespace BetterSongList::FileUtils {
    bool WriteFileAtomic(std::string_view path, std::string_view contents) {
        const std::string destination(path);
        // A unique sibling keeps concurrent writes separate and rename on one filesystem.
        std::string temporary = destination + ".tmp.XXXXXX";
        const int fd = mkstemp(temporary.data());
        if (fd == -1) return false;

        bool success = true;
        size_t offset = 0;
        while (offset < contents.size()) {
            const auto size = std::min(contents.size() - offset, static_cast<size_t>(std::numeric_limits<ssize_t>::max()));
            const auto written = write(fd, contents.data() + offset, size);
            if (written == -1 && errno == EINTR) continue;
            if (written <= 0) {
                success = false;
                break;
            }
            offset += static_cast<size_t>(written);
        }

        if (success) {
            int result;
            do {
                result = fsync(fd);
            } while (result == -1 && errno == EINTR);
            success = result == 0;
        }

        // On Android, close must not be retried after EINTR: the descriptor is already released.
        if (close(fd) != 0) success = false;
        if (success && std::rename(temporary.c_str(), destination.c_str()) == 0) return true;

        unlink(temporary.c_str());
        return false;
    }
}
