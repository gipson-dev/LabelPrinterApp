# Known Issues

This file tracks beta-era risks and remaining work for LabelPrinterApp.

## Current Beta Notes

- Clean-machine package testing is still required before a 1.0 release.
- Physical printer calibration is still required on both 203 DPI and 300 DPI Zebra printers.
- Preview-to-print matching was improved again in `v1.0.4`, but final vertical and horizontal offset checks still need real media validation.
- Barcode and QR scan validation should be repeated on real printed labels before production use.
- Print history can be viewed in-app from `View > Print History`, but it is read-only: exporting, clearing, and reprinting from history are still planned.
- Image/logo elements are visible in the toolbox as future work and are not implemented yet.
- The installer is still a beta convenience package; a fuller installer may be needed for non-technical users.
- The self-update flow (`Help > Check for Updates`, plus the silent startup check, download/verify, and `LabelPrinterAppLauncher.exe` apply/relaunch) has not yet been validated end to end against a real published GitHub release on a clean machine.
- The update checker requires outbound HTTPS access to GitHub; there is no offline or proxy-configured fallback yet beyond opening the releases page manually.

## Developer Setup Notes

- `nlohmann/json.hpp` is fetched by CMake into local build folders such as `build\_deps\nlohmann_json-src\include`.
- VS Code IntelliSense uses `.vscode/c_cpp_properties.json` to find that fetched include path.
- If VS Code still shows a `nlohmann/json.hpp` include error after a successful build, run `C/C++: Reset IntelliSense Database`.
