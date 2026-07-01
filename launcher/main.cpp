#include <windows.h>
#include <shellapi.h>

#include <exception>
#include <filesystem>
#include <string>
#include <vector>

#include "labelprinterapp/update/manager.hpp"

namespace update = labelprinterapp::update;

namespace
{

// The working directory and latest-directory name are ASCII folder names
// chosen by LabelPrinterApp itself, so a plain wide-to-narrow cast is safe
// here (a general UTF-8 conversion is unnecessary for that guaranteed case).
std::string narrow(std::wstring const& wide)
{
    std::string result;
    result.reserve(wide.size());
    for (wchar_t c : wide) {
        result.push_back(static_cast<char>(c));
    }
    return result;
}

} // namespace

// LabelPrinterAppLauncher applies an update that LabelPrinterApp downloaded
// into its working directory, then relaunches LabelPrinterApp from the
// (now updated) latest directory. It is started as a copy in a temporary
// directory so it isn't locked while the directory it runs from is replaced,
// and receives its working directory and latest-directory name as arguments
// since it can no longer derive them from its own (temporary) executable path.
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == nullptr) {
        return 1;
    }
    if (argc < 3) {
        LocalFree(argv);
        return 1;
    }

    std::filesystem::path working_directory(argv[1]);
    std::string latest_directory_name = narrow(argv[2]);
    LocalFree(argv);

    try {
        update::manager manager(
            working_directory, update::version_number{ 0 }, latest_directory_name);
        manager.retain_installed_files({ "templates", "logs" });
        manager.apply_latest();
        manager.start_latest(L"LabelPrinterApp.exe");
    }
    catch (std::exception const&) {
        return 1;
    }

    return 0;
}
