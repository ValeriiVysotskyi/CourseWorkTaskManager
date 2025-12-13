cmake_minimum_required(VERSION 3.16)

project(CourseWork VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Sql)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appCourseWork
    main.cpp
)

qt_add_qml_module(appCourseWork
    URI CourseWork
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES dbManager.h
        SOURCES dbManager.cpp
        SOURCES dbSync.h dbSync.cpp
        SOURCES AuthorizationController.h
        QML_FILES TaskCard.qml
        QML_FILES TaskCreateDlg.qml
        RESOURCES resources.qrc
        SOURCES AuthorizationController.cpp
        SOURCES authorizationController.h authorizationController.cpp
)

qt_add_resources(appCourseWork "dlgWindows"
    PREFIX "/"
    FILES TaskCreateDlg.qml
    FILES MemeWatchingDlg.qml
    FILES MemeVideo.mp4
)
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appCourseWork PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appCourseWork
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appCourseWork
    PRIVATE Qt6::Quick
    PRIVATE Qt6::Sql
)

include(GNUInstallDirs)
install(TARGETS appCourseWork
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
