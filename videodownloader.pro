projectname = videodownloader
appname = "Video Downloader"
appauthor = Martchus
appurl = "https://github.com/$${appauthor}/$${projectname}"
VERSION = 1.0.7

# include ../../common.pri when building as part of a subdirs project; otherwise include general.pri
!include(../../common.pri) {
    !include(./general.pri) {
        error("Couldn't find the common.pri or the general.pri file!")
    }
}

TEMPLATE = app

QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

testdownload {
    DEFINES += CONFIG_TESTDOWNLOAD
}

underconstruction {
    DEFINES += UNDER_CONSTRUCTION
}

!notrayicon {
    DEFINES += CONFIG_USE_TRAY_ICON
    usetrayiconalways {
        DEFINES += CONFIG_USE_TRAY_ICON_ALWAYS
    }
}

SOURCES += application/main.cpp \
    network/bitsharedownload.cpp \
    network/download.cpp \
    model/downloadmodel.cpp \
    network/downloadrange.cpp \
    network/filenukedownload.cpp \
    network/finder/downloadfinder.cpp \
    network/finder/groovesharksearcher.cpp \
    network/finder/youtubeplaylist.cpp \
    model/downloadfinderresultsmodel.cpp \
    network/finder/linkfinder.cpp \
    network/groovesharkdownload.cpp \
    network/httpdownload.cpp \
    network/httpdownloadwithinforequst.cpp \
    network/misc/contentdispositionparser.cpp \
    network/socksharedownload.cpp \
    network/youtubedownload.cpp \
    gui/adddownloaddialog.cpp \
    gui/addmultipledownloadswizard.cpp \
    gui/downloadwidget.cpp \
    gui/mainwindow.cpp \
    gui/setrangedialog.cpp \
    application/utils.cpp \
    itemdelegates/comboboxitemdelegate.cpp \
    itemdelegates/progressbaritemdelegate.cpp \
    gui/downloadinteraction.cpp \
    gui/settings.cpp \
    network/optiondata.cpp \
    gui/initiate.cpp \
    cli/mainfeatures.cpp \
    cli/clidownloadinteraction.cpp

HEADERS += application/main.h \
    network/bitsharedownload.h \
    network/download.h \
    model/downloadmodel.h \
    network/downloadrange.h \
    network/filenukedownload.h \
    network/finder/downloadfinder.h \
    network/finder/groovesharksearcher.h \
    network/finder/youtubeplaylist.h \
    model/downloadfinderresultsmodel.h \
    network/finder/linkfinder.h \
    network/groovesharkdownload.h \
    network/httpdownload.h \
    network/httpdownloadwithinforequst.h \
    network/misc/contentdispositionparser.h \
    network/socksharedownload.h \
    network/youtubedownload.h \
    gui/adddownloaddialog.h \
    gui/addmultipledownloadswizard.h \
    gui/downloadwidget.h \
    gui/mainwindow.h \
    gui/setrangedialog.h \
    application/utils.h \
    itemdelegates/comboboxitemdelegate.h \
    itemdelegates/progressbaritemdelegate.h \
    gui/downloadinteraction.h \
    gui/settings.h \
    network/authenticationcredentials.h \
    network/permissionstatus.h \
    network/optiondata.h \
    application/main.h \
    gui/initiate.h \
    cli/mainfeatures.h \
    cli/clidownloadinteraction.h

testdownload {
    SOURCES += network/testdownload.cpp
    HEADERS += network/testdownload.h
}

underconstruction {
SOURCES += \
    network/spotifydownload.cpp

HEADERS += \
    network/spotifydownload.h
}

FORMS += gui/adddownloaddialog.ui \
    gui/downloadwidget.ui \
    gui/setrangedialog.ui \
    gui/mainwindow.ui \
    gui/targetpage.ui \
    gui/proxypage.ui \
    gui/useragentpage.ui

RESOURCES += resources/icons.qrc \
    resources/json.qrc

win32:include(windowsicon.pri)

OTHER_FILES += \
    README.md \
    LICENSE

CONFIG(debug, debug|release) {
    LIBS += -lc++utilitiesd
    !no-gui {
        LIBS += -lqtutilitiesd
    }
} else {
    LIBS += -lc++utilities
    !no-gui {
        LIBS += -lqtutilities
    }
}

# installs
target.path = $$(INSTALL_ROOT)/bin
INSTALLS += target
!mingw-w64-install {
    icon.path = $$(INSTALL_ROOT)/share/icons/hicolor/scalable/apps/
    icon.files = $${PWD}/resources/icons/hicolor/scalable/apps/$${projectname}.svg
    INSTALLS += icon
    menu.path = $$(INSTALL_ROOT)/share/applications/
    menu.files = $${PWD}/resources/desktop/applications/$${projectname}.desktop
    INSTALLS += menu
}
json.path = $$(INSTALL_ROOT)/share/$${projectname}/json/
json.files = $${PWD}/resources/json/groovesharkauthenticationinfo.json
INSTALLS += json
