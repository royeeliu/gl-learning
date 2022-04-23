#include "file_path.h"

// clang-format off
#if defined(_WIN32)
    #include <windows.h>
    #include <Shlwapi.h>
    #include <io.h> 
    #define access _access_s
#endif

#ifdef __APPLE__
    #include <libgen.h>
    #include <limits.h>
    #include <mach-o/dyld.h>
    #include <unistd.h>
#endif

#ifdef __linux__
    #include <limits.h>
    #include <libgen.h>
    #include <unistd.h>
    #if defined(__sun)
        #define PROC_SELF_EXE "/proc/self/path/a.out"
    #else
        #define PROC_SELF_EXE "/proc/self/exe"
    #endif
#endif
// clang-format on

namespace utils {

#if defined(_WIN32)
std::string GetExecutablePath()
{
    char file_name[MAX_PATH]{};
    GetModuleFileNameA(nullptr, file_name, MAX_PATH);
    return file_name;
}

std::string GetExecutableDir()
{
    char path[MAX_PATH]{};
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    PathRemoveFileSpecA(path);
    return path;
}
#endif // defined(_WIN32)

#if defined(__linux__)
std::string GetExecutablePath()
{
    char file_name[PATH_MAX]{};
    realpath(PROC_SELF_EXE, file_name);
    return file_name;
}

std::string GetExecutableDir()
{
    char file_name[PATH_MAX]{};
    realpath(PROC_SELF_EXE, file_name);
    char* dir = dirname(file_name);
    return dir;
}
#endif // defined(__linux__)

#if defined(__APPLE__)
std::string GetExecutablePath()
{
    char raw_path[PATH_MAX]{};
    char real_path[PATH_MAX]{};
    uint32_t raw_path_size = (uint32_t)sizeof(raw_path);

    if (!_NSGetExecutablePath(raw_path, &raw_path_size))
    {
        realpath(raw_path, real_path);
    }
    return real_path;
}

std::string GetExecutableDir()
{
    char raw_path[PATH_MAX]{};
    char real_path[PATH_MAX]{};
    uint32_t raw_path_size = (uint32_t)sizeof(raw_path);

    if (!_NSGetExecutablePath(raw_path, &raw_path_size))
    {
        realpath(raw_path, real_path);
    }
    return dirname(real_path);
}
#endif // defined(__APPLE__)

} // namespace utils