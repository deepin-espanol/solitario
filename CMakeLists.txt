cmake_minimum_required(VERSION 3.16)
project(Solitario VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Dtk6Core REQUIRED)
find_package(Dtk6Gui REQUIRED)
find_package(Dtk6Widget REQUIRED)

add_executable(solitario
    src/main.cpp
    src/Carta.h
    src/Monton.h
    src/JuegoKlondike.h src/JuegoKlondike.cpp
    src/TableroWidget.h src/TableroWidget.cpp
    src/MainWindow.h src/MainWindow.cpp
)

target_link_libraries(solitario PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
    Dtk6::Widget Dtk6::Core Dtk6::Gui
)
