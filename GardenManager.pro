QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    src/gui/addeventdialog.cpp \
    src/gui/addplantdialog.cpp \
    src/gui/addbatchdialog.cpp \
    src/gui/addgroupdialog.cpp \
    src/gui/addproductdialog.cpp \
    src/gui/addspeciedialog.cpp \
    src/gui/addvarietydialog.cpp \
    src/gui/mainwindow.cpp \
    src/database/databasemanager.cpp \
    src/gui/settingsdialog.cpp

HEADERS += \
    src/gui/addeventdialog.h \
    src/gui/addplantdialog.h \
    src/gui/addbatchdialog.h \
    src/gui/addgroupdialog.h \
    src/gui/addproductdialog.h \
    src/gui/addspeciedialog.h \
    src/gui/addvarietydialog.h \
    src/gui/mainwindow.h \
    src/database/databasemanager.h \
    src/gui/settingsdialog.h

FORMS += \
    src/gui/addeventdialog.ui \
    src/gui/addplantdialog.ui \
    src/gui/addbatchdialog.ui \
    src/gui/addgroupdialog.ui \
    src/gui/addproductdialog.ui \
    src/gui/addspeciedialog.ui \
    src/gui/addvarietydialog.ui \
    src/gui/mainwindow.ui \
    src/gui/settingsdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
TRANSLATIONS += \
    res/translations/garden_manager_it.ts

RESOURCES += \
    res/resources.qrc
