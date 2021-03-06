/* This file was generated by ecm_create_qm_loader().
 * It was edited for SeExpr.
 *
 * Building this file in a library ensures translations are automatically loaded
 * when an application makes use of the library.
 *
 *
 * SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>
 * SPDX-FileCopyrightText: 2015 Alex Merry <alex.merry@kde.org>
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <Debug.h>

#include <QCoreApplication>
#include <QDir>
#include <QLocale>
#include <QObject>
#include <QStandardPaths>
#include <QThread>
#include <QTranslator>

namespace {

    bool loadTranslation(const QString &localeDirName)
    {
        QString subPath = QStringLiteral("locale/") + localeDirName + QStringLiteral("/LC_MESSAGES/seexpr2_qt.qm");

        dbgSeExpr << "Attempting to load: " << subPath;

#if defined(Q_OS_ANDROID)
        const QString fullPath = QStringLiteral("assets:/share/") + subPath;
        if (!QFile::exists(fullPath)) {
            return false;
        }
#else
        const QString fullPath = QStandardPaths::locate(QStandardPaths::DataLocation, subPath);
        if (fullPath.isEmpty()) {
            return false;
        }
#endif
        QTranslator *translator = new QTranslator(QCoreApplication::instance());
        if (!translator->load(fullPath)) {
            delete translator;
            return false;
        }
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        dbgSeExpr << "Installing translation for: " << fullPath << "(" << translator->language() << ")";
#else
        dbgSeExpr << "Installing translation for: " << fullPath;
#endif
        dbgSeExpr << "Test: " << translator->translate("ExprControlCollection", "Add new variable");

        QCoreApplication::instance()->installTranslator(translator);
        return true;
    }

    void load()
    {
#if defined(Q_OS_ANDROID)
        const auto paths = QStringLiteral("assets:/share/");
#else
        const auto paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
#endif
        dbgSeExpr << "Base paths for translations: " << paths;

        dbgSeExpr << "Qt UI languages: " << QLocale::system().uiLanguages() << qgetenv("LANG");

        // The way Qt translation system handles plural forms makes it necessary to
        // have a translation file which contains only plural forms for `en`. That's
        // why we load the `en` translation unconditionally, then load the
        // translation for the current locale to overload it.
        loadTranslation(QStringLiteral("en"));

        // Amy: use the default locale, not the system() one.
        // Krita changes the default at startup.
        for (const auto &locale : {QLocale::system(), QLocale()}) {
            dbgSeExpr << "Attempting to load translations for locale: " << locale.name();
            if (!loadTranslation(locale.name())) {
                if (!loadTranslation(locale.bcp47Name())) {
                    const int i = locale.name().indexOf(QLatin1Char('_'));
                    if (i > 0) {
                        loadTranslation(locale.name().left(i));
                    }
                }
            }
        }

        dbgSeExpr << "Test: " << QCoreApplication::translate("ExprControlCollection", "Add new variable");
    }

    // Helper to call load() on the main thread.
    //
    // Calling functions on another thread without using moc is non-trivial in
    // Qt until 5.4 (when some useful QTimer::singleShot overloads were added).
    //
    // Instead, we have to use QEvents. Ideally, we'd use a custom QEvent, but
    // there's a chance this could cause trouble with applications that claim
    // QEvent codes themselves, but don't register them with Qt (and we also
    // want to avoid registering a new QEvent code for every plugin that might
    // be loaded). We use QTimer because it's unlikely to be filtered by
    // applications, and is also unlikely to cause Qt to do something it
    // shouldn't.
    class Loader : public QObject
    {
    protected:
        void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE
        {
            load();
            this->deleteLater();
        }
    };

    void loadOnMainThread()
    {
        // If this library is loaded after the QCoreApplication instance is
        // created (eg: because it is brought in by a plugin), there is no
        // guarantee this function will be called on the main thread.
        // QCoreApplication::installTranslator needs to be called on the main
        // thread, because it uses QCoreApplication::sendEvent.
        if (QThread::currentThread() == QCoreApplication::instance()->thread()) {
            load();
        } else {
            // QObjects inherit their parent object's thread
            Loader *loader = new Loader();
            loader->moveToThread(QCoreApplication::instance()->thread());
            QCoreApplication::instance()->postEvent(loader, new QTimerEvent(0), Qt::HighEventPriority);
        }
    }
}

Q_COREAPP_STARTUP_FUNCTION(loadOnMainThread)
