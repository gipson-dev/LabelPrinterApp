# Contributing

Thanks for your interest in contributing to LabelPrinterApp.

## How to Contribute

1. Open an issue to describe a bug, question, or proposed improvement.
2. Fork the repository and create a focused branch for your change.
3. Keep commits small and descriptive.
4. Open a pull request using the repository template.

## Pull Request Checklist

- The change is focused and easy to review.
- Documentation is updated when behavior changes.
- Tests or manual verification steps are included when applicable.
- No secrets, credentials, or local-only files are committed.

## Development Notes

LabelPrinterApp is a Windows C++17 Qt 6 Widgets application built with CMake. See
[README.md](README.md#building-from-source) for setup requirements and
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for the project structure.

Build and test locally with:

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.8.3\msvc2022_64"
.\scripts\build-and-test.ps1 -Config Release
```

This configures and builds the app and `c-updater`, then runs `ctest`. There is no
separate formatting step; match the existing code style in the file you are editing.
