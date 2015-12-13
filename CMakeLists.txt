cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

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
#set(QUICK_HEADER_FILES
#)
#set(QUICK_SRC_FILES
#)
#set(TS_FILES
#    translations/${META_PROJECT_NAME}_de_DE.ts
#    translations/${META_PROJECT_NAME}_en_US.ts
#)

# meta data
set(META_PROJECT_NAME videodownloader)
set(META_APP_NAME "Video Downloader")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "A video downloader with Qt GUI (currently only YouTube and Vimeo are maintained).")
set(META_VERSION_MAJOR 1)
set(META_VERSION_MINOR 2)
set(META_VERSION_PATCH 1)

# stringification of meta data
set(META_PROJECT_NAME_STR "\"${META_PROJECT_NAME}\"")
set(META_APP_NAME_STR "\"${META_APP_NAME}\"")
set(META_APP_AUTHOR_STR "\"${META_APP_AUTHOR}\"")
set(META_APP_URL_STR "\"${META_APP_URL}\"")
set(META_APP_DESCRIPTION_STR "\"${META_APP_DESCRIPTION}\"")
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})
set(META_APP_VERSION_STR "\"${META_APP_VERSION}\"")

# define project
project(${META_PROJECT_NAME})

# add configuration header
configure_file(
    "${PROJECT_SOURCE_DIR}/resources/config.h.in"
    "${PROJECT_BINARY_DIR}/resources/config.h"
)
include_directories("${PROJECT_BINARY_DIR}")

# set "GUI-type" to WIN32 to hide console under windows
if(WIN32)
    set(GUI_TYPE WIN32)
endif(WIN32)

# add windows resource file
if(MINGW)
    # creation windows icon from png with ffmpeg if available
    find_program(FFMPEG_BIN ffmpeg avconv)
    if(FFMPEG_BIN)
        set(PNG_ICON_PATH "${PROJECT_SOURCE_DIR}/resources/icons/hicolor/128x128/apps/${META_PROJECT_NAME}.png")
        if(EXISTS ${PNG_ICON_PATH})
            set(WINDOWS_ICON_PATH "${PROJECT_BINARY_DIR}/${META_PROJECT_NAME}.ico")
            set(WINDOWS_ICON_RC_ENTRY "IDI_ICON1   ICON    DISCARDABLE \"${WINDOWS_ICON_PATH}\"")
            add_custom_command(
                OUTPUT "${WINDOWS_ICON_PATH}"
                COMMAND ${FFMPEG_BIN} -y -i "${PNG_ICON_PATH}" -vf crop=iw-20:ih-20:10:10,scale=64:64 "${WINDOWS_ICON_PATH}"
            )
        endif()
    endif(FFMPEG_BIN)
    # create windows rc file from template
    set(WINDOWS_EXT "exe")
    configure_file(
        "${PROJECT_SOURCE_DIR}/resources/windows.rc.in"
        "${PROJECT_BINARY_DIR}/resources/windows.rc"
    )
    # set windres as resource compiler
    set(RES_FILES "${PROJECT_BINARY_DIR}/resources/windows.rc")
    set(CMAKE_RC_COMPILER_INIT windres)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
    enable_language(RC)
endif(MINGW)

# enable Qt Widgets GUI
# disable new ABI (can't catch ios_base::failure with new ABI)
add_definitions(
    -DGUI_QTWIDGETS
    -DMODEL_UNDO_SUPPORT
    -D_GLIBCXX_USE_CXX11_ABI=0
)

# check required Qt 5 modules
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Network REQUIRED)

# enable moc, uic and rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# enable lrelease
qt5_add_translation(QM_FILES ${TS_FILES})
ADD_CUSTOM_TARGET(translations ALL DEPENDS ${QM_FILES})

# executable and linking
add_executable(${META_PROJECT_NAME} ${GUI_TYPE} ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_HEADER_FILES} ${WIDGETS_SRC_FILES} ${QM_FILES} ${RES_FILES} ${WINDOWS_ICON_PATH})
target_link_libraries(${META_PROJECT_NAME} c++utilities qtutilities Qt5::Core Qt5::Widgets Qt5::Network)
set_target_properties(${META_PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
)

# add install target
install(TARGETS ${META_PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT binary
)
install(FILES resources/icons/hicolor/scalable/apps/${META_PROJECT_NAME}.svg
    DESTINATION share/icons/hicolor/scalable/apps
    COMPONENT desktop
)
install(FILES resources/desktop/applications/${META_PROJECT_NAME}.desktop
    DESTINATION share/applications
    COMPONENT desktop
)
install(FILES ${QM_FILES}
    DESTINATION share/${META_PROJECT_NAME}/translations
    COMPONENT localization
)
install(FILES resources/json/groovesharkauthenticationinfo.json
    DESTINATION share/${META_PROJECT_NAME}/json
    COMPONENT config
)
install(FILES resources/json/itaginfo.json
    DESTINATION share/${META_PROJECT_NAME}/json
    COMPONENT config
)
add_custom_target(install-binary
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-desktop
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=desktop -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-localization
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=localization -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-config
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=config -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-mingw-w64
    DEPENDS install-binary install-localization install-config
)
add_custom_target(install-binary-strip
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_DO_STRIP=1 -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-mingw-w64-strip
    DEPENDS install-binary-strip install-localization install-config
)
