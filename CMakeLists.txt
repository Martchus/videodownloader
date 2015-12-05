cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# meta data
set(META_PROJECT_NAME videodownloader)
set(META_APP_NAME "Video Downloader")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META APP_DESCRIPTION "A video downloader with Qt GUI (currently only YouTube and Vimeo are maintained).")
set(META_VERSION_MAJOR 1)
set(META_VERSION_MINOR 2)
set(META_VERSION_PATCH 1)

# define project
project(${META_PROJECT_NAME})

# stringification of meta data
set(META_PROJECT_NAME_STR "\"${META_PROJECT_NAME}\"")
set(META_APP_NAME_STR "\"${META_APP_NAME}\"")
set(META_APP_AUTHOR_STR "\"${META_APP_AUTHOR}\"")
set(META_APP_URL_STR "\"${META_APP_URL}\"")
set(APP_DESCRIPTION_STR "\"${APP_DESCRIPTION}\"")
set(META_APP_VERSION_STR "\"${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}\"")

# add configuration header
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)
include_directories("${PROJECT_BINARY_DIR}")

# add source and header files
file(GLOB_RECURSE SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "application/*.cpp" "cli/*.cpp" "model/*.cpp" "network/*.cpp" "resources/json.qrc")
file(GLOB_RECURSE WIDGETS_SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "gui/*.cpp" "itemdelegates/*.cpp" "resources/icons.qrc")
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "application/*.h" "cli/*.h" "model/*.h" "network/*.h")
file(GLOB_RECURSE WIDGETS_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "gui/*.h" "itemdelegates/*.h")

# enable only Qt Widgets GUI
add_definitions(-DGUI_QTWIDGETS -DMODEL_UNDO_SUPPORT)

# check required Qt 5 modules
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

# enable moc, uic and rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# executable and linking
add_executable(${META_PROJECT_NAME} ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_HEADER_FILES} ${WIDGETS_SRC_FILES})
target_link_libraries(${META_PROJECT_NAME} c++utilities qtutilities Qt5::Core Qt5::Widgets Qt5::Network)

# enable C++11
set_property(TARGET ${META_PROJECT_NAME} PROPERTY CXX_STANDARD 11)

# add install target
install(TARGETS ${META_PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(FILES resources/icons/hicolor/scalable/apps/${META_PROJECT_NAME}.svg
    DESTINATION share/icons/hicolor/scalable/apps
)
install(FILES resources/desktop/applications/${META_PROJECT_NAME}.desktop
    DESTINATION share/applications
)
install(FILES resources/json/groovesharkauthenticationinfo.json
    DESTINATION share/${META_PROJECT_NAME}/json
)
install(FILES resources/json/itaginfo.json
    DESTINATION share/${META_PROJECT_NAME}/json
)
