# Known Issues

This file tracks beta-era risks and remaining work for LabelPrinterApp.

## Current Beta Notes

- Clean-machine package testing is still required before a 1.0 release.
- Physical printer calibration is still required on both 203 DPI and 300 DPI Zebra printers.
- Preview-to-print matching is improved, but final vertical and horizontal offset checks still need real media validation.
- Barcode and QR scan validation should be repeated on real printed labels before production use.
- The print history is currently a CSV file at `logs\print_history.csv`; an in-app viewer is still planned.
- Image/logo elements are visible in the toolbox as future work and are not implemented yet.
- The installer is still a beta convenience package; a fuller installer may be needed for non-technical users.

## Developer Setup Notes

- `nlohmann/json.hpp` is fetched by CMake into local build folders such as `build\_deps\nlohmann_json-src\include`.
- VS Code IntelliSense uses `.vscode/c_cpp_properties.json` to find that fetched include path.
- If VS Code still shows a `nlohmann/json.hpp` include error after a successful build, run `C/C++: Reset IntelliSense Database`.
