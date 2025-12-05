cmake_minimum_required(VERSION 3.19)
project(RedisInstall LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()

qt_add_executable(RedisInstall
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    serviceconfig.cpp
    serviceconfig.h
    servicemanager.cpp
    servicemanager.h
    portchecker.cpp
    portchecker.h
    redismanager.cpp
    redismanager.h
)

target_link_libraries(RedisInstall
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

# Windows specific libraries
if(WIN32)
    target_link_libraries(RedisInstall PRIVATE iphlpapi ws2_32)
endif()

include(GNUInstallDirs)

install(TARGETS RedisInstall
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET RedisInstall
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
