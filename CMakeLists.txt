cmake_minimum_required(VERSION 3.16)
project(Solitario VERSION 1.0.1 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-fat-lto-objects")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-lto")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools Multimedia)
find_package(Dtk6Core REQUIRED)
find_package(Dtk6Gui REQUIRED)
find_package(Dtk6Widget REQUIRED)

set(LANGUAGES en zh hi es fr ar bn pt ru ja ko de id tr it vi th pl)
set(TS_FILES)
foreach(LANG ${LANGUAGES})
    list(APPEND TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/solitario_${LANG}.ts")
endforeach()

add_executable(solitario
    src/main.cpp
    src/Carta.h
    src/Monton.h
    src/JuegoKlondike.h src/JuegoKlondike.cpp
    src/JuegoSpider.h src/JuegoSpider.cpp
    src/JuegoFreeCell.h src/JuegoFreeCell.cpp
    src/TableroWidget.h src/TableroWidget.cpp
    src/MainWindow.h src/MainWindow.cpp
    src/resources.qrc
)

# Compila los .ts y los embebe como recursos en :/i18n (ver main.cpp)
qt_add_translations(solitario
    TS_FILES ${TS_FILES}
    RESOURCE_PREFIX "/i18n"
)

target_link_libraries(solitario PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Multimedia
    Dtk6::Widget Dtk6::Core Dtk6::Gui
)

target_include_directories(solitario PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS solitario RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES data/solitario.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES src/logo.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME solitario.svg)
