#pragma once

#include <QObject>
#include <QString>

#include <filesystem>
#include <memory>
#include <string>

namespace labelprinterapp::update
{
class manager;
}

// Checks the gipson-dev/LabelPrinterApp GitHub releases for a newer version,
// downloads and extracts it in the background, and hands off to
// LabelPrinterAppLauncher.exe to apply it and relaunch the app on restart.
class AppUpdater : public QObject
{
    Q_OBJECT

public:
    explicit AppUpdater(QObject* parent = nullptr);
    ~AppUpdater() override;

    // False if the update manager could not be initialized, e.g. because
    // another instance of the app is already holding the update lock.
    bool isAvailable() const;

    // Starts a background check for a newer release. Emits exactly one of
    // upToDate(), updateReady() or checkFailed() when done. Does nothing if
    // a check is already running or the manager is unavailable.
    void checkForUpdates();

    // Restarts the app through LabelPrinterAppLauncher.exe to apply a
    // previously downloaded update. Returns true if the launcher was started,
    // in which case the caller must quit the application immediately.
    bool applyDownloadedUpdateAndRestart();

signals:
    void checkStarted();
    void upToDate();
    void updateReady(const QString& version);
    void checkFailed(const QString& error);

private:
    void finishCheck(bool upToDateResult, bool hasNewVersion,
        const QString& version, const QString& error);

    std::shared_ptr<labelprinterapp::update::manager> manager_;
    std::filesystem::path workingDirectory_;
    std::string latestDirectoryName_;
    bool checking_ = false;
};
