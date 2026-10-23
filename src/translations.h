#pragma once

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

// Vega Qt ships en-US as the source language, plus pt-BR and es-ES catalogs
// embedded under :/i18n. Any Portuguese or Spanish variant uses the matching
// catalog; every other language falls back to English.
inline void installVegaTranslations(QCoreApplication &app)
{
    static QTranslator vega;
    static QTranslator qtBase;
    for (const QString &language : QLocale().uiLanguages()) {
        QString catalog;
        if (language.startsWith(QLatin1String("pt")))
            catalog = QStringLiteral("vega-qt_pt_BR");
        else if (language.startsWith(QLatin1String("es")))
            catalog = QStringLiteral("vega-qt_es_ES");
        else if (language.startsWith(QLatin1String("en")))
            return;
        else
            continue;
        if (vega.load(catalog, QStringLiteral(":/i18n")))
            app.installTranslator(&vega);
        if (qtBase.load(QLocale(), QStringLiteral("qtbase"), QStringLiteral("_"),
                        QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
            app.installTranslator(&qtBase);
        return;
    }
}
