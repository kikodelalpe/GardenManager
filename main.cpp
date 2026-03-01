#include "src/gui/mainwindow.h"
#include "src/database/databasemanager.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("DMsw");
    QCoreApplication::setApplicationName("GardenManager");
    QSettings settings("MyGardenApp", "GardenManager");
    QString langCode = settings.value("language", "").toString();
    if(langCode.isEmpty()){
        QLocale systemLocale = QLocale::system();
        langCode = systemLocale.name().section('_', 0, 0);
        qInfo() << "No configuration found. Using system language: " << langCode;
    }
    else qInfo() << "Language loaded from settings: " << langCode;
    
    QTranslator qtTranslator;
    if(qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) a.installTranslator(&qtTranslator);
    QTranslator myTranslator;
    if(langCode != "en"){
        QString translationFile = ":/translations/garden_manager_" + langCode;
        QFileInfo checkFile(translationFile + ".qm");
        qInfo() << "Searching translation file " << langCode << " in: " << checkFile.absoluteFilePath();
        if (myTranslator.load(translationFile)){
            a.installTranslator(&myTranslator);
            qInfo() << "Translation file loaded succesfully.";
        }
        else qWarning() << "Attenzione: File di traduzione non caricato.";
    }
    else qInfo() << "English language selected, no translation loaded.";
    
    if(!DatabaseManager::instance().connectToDatabase()){
        qCritical() << "ERROR: Unable to connect to database";
        return -1;
    }
    qInfo() << "System check: Database OK.";
    MainWindow w;
    w.show();
    return a.exec();
}