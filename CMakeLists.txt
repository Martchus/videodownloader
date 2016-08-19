cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# meta data
set(META_PROJECT_NAME videodownloader)
set(META_PROJECT_TYPE application)
set(META_APP_NAME "Video Downloader")
set(META_APP_CATEGORIES "Utility;Network;")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "Simple video downloader with Qt GUI (currently only YouTube and Vimeo are maintained)")
set(META_VERSION_MAJOR 1)
set(META_VERSION_MINOR 3)
set(META_VERSION_PATCH 1)

# add project files
set(HEADER_FILES
    application/main.h
    application/utils.h
    cli/clidownloadinteraction.h
    cli/mainfeatures.h
    model/downloadfinderresultsmodel.h
    model/downloadmodel.h
    network/authenticationcredentials.h
    network/bitsharedownload.h
    network/download.h
    network/downloadrange.h
    network/filenukedownload.h
    network/finder/downloadfinder.h
    network/finder/groovesharksearcher.h
    network/finder/linkfinder.h
    network/finder/youtubeplaylist.h
    network/groovesharkdownload.h
    network/httpdownload.h
    network/httpdownloadwithinforequst.h
    network/misc/contentdispositionparser.h
    network/optiondata.h
    network/permissionstatus.h
    network/socksharedownload.h
    network/spotifydownload.h
    network/testdownload.h
    network/vimeodownload.h
    network/youtubedownload.h
)
set(SRC_FILES
    application/main.cpp
    application/utils.cpp
    cli/clidownloadinteraction.cpp
    cli/mainfeatures.cpp
    model/downloadfinderresultsmodel.cpp
    model/downloadmodel.cpp
    network/bitsharedownload.cpp
    network/download.cpp
    network/downloadrange.cpp
    network/filenukedownload.cpp
    network/finder/downloadfinder.cpp
    network/finder/groovesharksearcher.cpp
    network/finder/linkfinder.cpp
    network/finder/youtubeplaylist.cpp
    network/groovesharkdownload.cpp
    network/httpdownload.cpp
    network/httpdownloadwithinforequst.cpp
    network/misc/contentdispositionparser.cpp
    network/optiondata.cpp
    network/socksharedownload.cpp
    network/spotifydownload.cpp
    network/testdownload.cpp
    network/vimeodownload.cpp
    network/youtubedownload.cpp
)

set(WIDGETS_HEADER_FILES
    gui/downloadwidget.h
    gui/adddownloaddialog.h
    gui/addmultipledownloadswizard.h
    gui/downloadinteraction.h
    gui/initiate.h
    gui/mainwindow.h
    gui/setrangedialog.h
    gui/settings.h
    model/downloadfinderresultsmodel.h
    model/downloadmodel.h
)
set(WIDGETS_SRC_FILES
    gui/adddownloaddialog.cpp
    gui/addmultipledownloadswizard.cpp
    gui/downloadinteraction.cpp
    gui/downloadwidget.cpp
    gui/initiate.cpp
    gui/mainwindow.cpp
    gui/setrangedialog.cpp
    gui/settings.cpp
    itemdelegates/comboboxitemdelegate.cpp
    itemdelegates/progressbaritemdelegate.cpp
    resources/icons.qrc
)
set(WIDGETS_UI_FILES
    gui/adddownloaddialog.ui
    gui/downloadwidget.ui
    gui/setrangedialog.ui
    gui/mainwindow.ui
    gui/targetpage.ui
    gui/proxypage.ui
    gui/useragentpage.ui
)

#set(QUICK_HEADER_FILES
#)
#set(QUICK_SRC_FILES
#)

#set(TS_FILES
#    translations/${META_PROJECT_NAME}_de_DE.ts
#    translations/${META_PROJECT_NAME}_en_US.ts
#)

set(ICON_FILES
    resources/icons/hicolor/scalable/apps/${META_PROJECT_NAME}.svg
)

set(DOC_FILES
    README.md
)

set(REQUIRED_ICONS
    edit-paste
    window-close
    document-save-as
    go-previous
    document-open
    edit-copy
    preferences-other
    document-revert
    document-open-recent
    list-remove
    dialog-ok
    media-playback-start
    dialog-cancel
    edit-clear
    list-add
    dialog-ok-apply
    help-about
    application-exit
    edit-cut
    media-playback-pause
    edit-undo
    system-file-manager
    edit-select-all
    preferences-system-network
    system-run
    go-next
)

# find c++utilities
find_package(c++utilities 4.0.0 REQUIRED)
use_cpp_utilities()

# find qtutilities
find_package(qtutilities 5.0.0 REQUIRED)
use_qt_utilities()

# add Qt modules which can currently not be detected automatically
list(APPEND ADDITIONAL_QT_MODULES Network)

# add installs for JSON resource files
install(FILES resources/json/groovesharkauthenticationinfo.json
    DESTINATION share/${META_PROJECT_NAME}/json
    COMPONENT json-res
)
install(FILES resources/json/itaginfo.json
    DESTINATION share/${META_PROJECT_NAME}/json
    COMPONENT json-res
)
if(NOT TARGET install-json-res)
    add_custom_target(install-json-res
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=json-res -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# include modules to apply configuration
include(BasicConfig)
include(QtGuiConfig)
include(QtConfig)
include(WindowsResources)
include(AppTarget)
include(ShellCompletion)
include(ConfigHeader)

# create desktop file using previously defined meta data
add_desktop_file()
