cmake_minimum_required(VERSION 3.16)
project(MemeGenerator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Qt6_DIR "/home/liborui/Qt/6.11.0/gcc_64/lib/cmake/Qt6")
list(PREPEND CMAKE_PREFIX_PATH "/home/liborui/Qt/6.11.0/gcc_64")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(SOURCES
    commands.cpp
    cyberdistressingdialog.cpp
    filterdialog.cpp
    imagecropperlabel.cpp
    main.cpp
    mainwindow.cpp
    menuconfig.cpp
    resizableitem.cpp
)

set(HEADERS
    commands.h
    cyberdistressingdialog.h
    dimoutsidecanvaseffect.h
    filterdialog.h
    imagecropperdialog.h
    imagecropperlabel.h
    mainwindow.h
    menuconfig.h
    resizableitem.h
)

set(FORMS
    filterdialog.ui
    mainwindow.ui
)

set(RESOURCES
    images.qrc
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
    logo.rc
)

# 修改为 Qt6 库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
 