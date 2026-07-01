#include "core/AppUpdater.h"

#include <QCoreApplication>
#include <QDir>
#include <QPointer>
#include <QtGlobal>

#include <exception>
#include <regex>
#include <thread>

#include "core/AppVersion.h"

#include "labelprinterapp/update/manager.hpp"
#include "labelprinterapp/update/updater.hpp"
#include "labelprinterapp/update/detail/github.h"
#include "labelprinterapp/update/detail/operations.h"

namespace update = labelprinterapp::update;

AppUpdater::AppUpdater(QObject* parent) : QObject(parent)
{
    QDir appDir(QCoreApplication::applicationDirPath());
    latestDirectoryName_ = appDir.dirName().toStdString();
    appDir.cdUp();
    workingDirectory_ = std::filesystem::path(appDir.absolutePath().toStdWString());

    try {
        update::version_number currentVersion{ LABELPRINTERAPP_VERSION_MAJOR,
            LABELPRINTERAPP_VERSION_MINOR, LABELPRINTERAPP_VERSION_PATCH };
        manager_ = std::make_shared<update::manager>(
            workingDirectory_, currentVersion, latestDirectoryName_);
        manager_->retain_installed_files({ "templates", "logs" });
        manager_->set_launcher(std::make_unique<update::launcher>(
            std::filesystem::path("LabelPrinterAppLauncher.exe"),
            std::vector<std::filesystem::path>{}));
    }
    catch (std::exception const& e) {
        manager_.reset();
        qWarning("AppUpdater: update manager unavailable: %s", e.what());
    }
}

AppUpdater::~AppUpdater() = default;

bool AppUpdater::isAvailable() const
{
    return manager_ != nullptr;
}

void AppUpdater::checkForUpdates()
{
    if (checking_) {
        return;
    }
    if (!manager_) {
        emit checkFailed("The update manager is unavailable.");
        return;
    }

    checking_ = true;
    emit checkStarted();

    auto manager = manager_;
    QPointer<AppUpdater> self(this);
    std::thread([manager, self]() {
        bool upToDateResult = false;
        bool hasNewVersion = false;
        std::string newVersion;
        QString errorMessage;
        try {
            update::updater upd(manager);
            upd.update_source(
                update::github_api_latest_retriever("gipson-dev", "LabelPrinterApp"));
            upd.archive_type(update::archive_type::zip);
            upd.filename_contains_version(false);
            upd.download_filename_pattern(
                std::regex(R"(LabelPrinterApp_Portable\.zip)"));
            upd.add_content_operation(update::operations::flatten_extracted_directory());

            auto info = upd.get_latest();
            switch (info.state()) {
            case update::state::up_to_date:
            case update::state::latest_is_older:
                upToDateResult = true;
                break;
            case update::state::update_already_installed:
                hasNewVersion = true;
                newVersion = info.version().string();
                break;
            case update::state::new_version_available:
                upd.update(info);
                hasNewVersion = true;
                newVersion = info.version().string();
                break;
            }
            if (!hasNewVersion) {
                // No update in progress, safe to prune old version
                // directories and stale launcher temp copies.
                manager->prune();
            }
        }
        catch (std::exception const& e) {
            errorMessage = QString::fromStdString(e.what());
        }

        auto version = QString::fromStdString(newVersion);
        QMetaObject::invokeMethod(
            qApp,
            [self, upToDateResult, hasNewVersion, version, errorMessage]() {
                if (!self) {
                    return;
                }
                self->finishCheck(upToDateResult, hasNewVersion, version, errorMessage);
            },
            Qt::QueuedConnection);
    }).detach();
}

void AppUpdater::finishCheck(bool upToDateResult, bool hasNewVersion,
    const QString& version, const QString& error)
{
    checking_ = false;
    if (!error.isEmpty()) {
        emit checkFailed(error);
    } else if (hasNewVersion) {
        emit updateReady(version);
    } else if (upToDateResult) {
        emit upToDate();
    }
}

bool AppUpdater::applyDownloadedUpdateAndRestart()
{
    if (!manager_) {
        return false;
    }
    try {
        std::vector<std::wstring> launcherArguments{
            workingDirectory_.wstring(),
            QString::fromStdString(latestDirectoryName_).toStdWString(),
        };
        return manager_->launch_latest(launcherArguments);
    }
    catch (std::exception const&) {
        return false;
    }
}
