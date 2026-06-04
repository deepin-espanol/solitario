cmake_minimum_required(VERSION 3.16)
project(Solitario VERSION 1.0 LANGUAGES CXX)

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
set(QM_FILES)
foreach(LANG ${LANGUAGES})
    list(APPEND TS_FILES "${CMAKE_SOURCE_DIR}/translations/solitario_${LANG}.ts")
    list(APPEND QM_FILES "${CMAKE_BINARY_DIR}/translations/solitario_${LANG}.qm")
endforeach()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/translations")

set(_qm_depends)
foreach(LANG ${LANGUAGES})
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/translations/solitario_${LANG}.qm"
        COMMAND Qt6::lrelease
            "${CMAKE_SOURCE_DIR}/translations/solitario_${LANG}.ts"
            -qm "${CMAKE_BINARY_DIR}/translations/solitario_${LANG}.qm"
        DEPENDS "${CMAKE_SOURCE_DIR}/translations/solitario_${LANG}.ts"
        COMMENT "lrelease solitario_${LANG}.qm"
    )
    list(APPEND _qm_depends "${CMAKE_BINARY_DIR}/translations/solitario_${LANG}.qm")
endforeach()

set(_qrc_content "<RCC>\n    <qresource prefix=\"/i18n\">\n")
foreach(LANG ${LANGUAGES})
    string(APPEND _qrc_content "        <file>translations/solitario_${LANG}.qm</file>\n")
endforeach()
string(APPEND _qrc_content "    </qresource>\n</RCC>\n")
file(WRITE "${CMAKE_BINARY_DIR}/translations.qrc" "${_qrc_content}")

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
    ${CMAKE_BINARY_DIR}/translations.qrc
    ${_qm_depends}
)

target_link_libraries(solitario PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Multimedia
    Dtk6::Widget Dtk6::Core Dtk6::Gui
)

target_include_directories(solitario PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS solitario DESTINATION bin)