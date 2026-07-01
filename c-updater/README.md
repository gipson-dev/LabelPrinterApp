# LabelPrinterApp C++ Updater

Header-only C++ update helper repurposed for LabelPrinterApp release delivery.
It is intended to live in the LabelPrinterApp source tree as `c-updater` and be
consumed with CMake.

## Features

- Fetches the latest LabelPrinterApp release metadata from GitHub.
- Downloads update archives over HTTPS.
- Verifies downloads with optional checksum and signature files.
- Extracts ZIP update archives on Windows.
- Maintains versioned install directories and a stable `current` directory.
- Supports a small launcher executable that can apply an update while the main
  app exits.

## CMake Usage

Add the updater repo from the LabelPrinterApp root:

```cmake
set(LABELPRINTERAPP_UPDATE_BUILD_DOWNLOADER ON CACHE BOOL "" FORCE)
add_subdirectory(c-updater)
target_link_libraries(LabelPrinterApp PRIVATE labelprinterapp_update)
```

Launcher-only targets can link the smaller manager surface:

```cmake
add_subdirectory(c-updater)
target_link_libraries(LabelPrinterAppLauncher PRIVATE labelprinterapp_update_manager)
```

Use the public headers from the `labelprinterapp/update` include path:

```cpp
#include "labelprinterapp/update/manager.hpp"
#include "labelprinterapp/update/updater.hpp"
```

The public namespace is:

```cpp
namespace labelprinterapp::update
```

## Release Archive Expectations

LabelPrinterApp update archives should be ZIP files that contain the app files
needed to run a complete updated build. The updater expects versioned releases,
for example `v1.2.3`, and can select assets whose filenames contain the matching
version string.

The app should retain local data, templates, user configuration, and installer
artifacts by configuring the manager's retained paths before applying updates.

## Dependencies

- C++17
- CMake 3.15 or newer
- OpenSSL for `labelprinterapp_update`
  - Windows auto-detects `C:/Program Files/OpenSSL-Win64` when present.
  - Otherwise pass `-DOPENSSL_ROOT_DIR=<path-to-openssl>`.
- yhirose/cpp-httplib for `labelprinterapp_update`
- nlohmann/json
- minizip-ng on Windows for `labelprinterapp_update`
  - System ZLIB is used when available; otherwise minizip-ng is allowed to fetch it.
- GoogleTest when `LABELPRINTERAPP_UPDATE_BUILD_TESTS` is enabled
