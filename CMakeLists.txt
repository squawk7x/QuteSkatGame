cmake_minimum_required(VERSION 3.16)

project(
  quteSkatGame
  VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES res/quteSkatGame_en_US.ts res/quteSkatGame_en_US.ts)

# Include the src directory for header files
include_directories(src)

set(PROJECT_SOURCES
    src/main.cpp
    # src/xmain.cpp
    src/table.cpp
    src/table.h
    src/table.ui
    src/game.cpp
    src/game.h
    src/card.h
    src/card.cpp
    src/cardvec.h
    src/cardvec.cpp
    src/player.h
    src/player.cpp
    ${TS_FILES})

# Add main executable add_executable(skat src/skat.cpp src/card.cpp src/card.h
# src/cardvec.cpp src/cardvec.h)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  qt_add_executable(
    quteSkatGame
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    src/main.cpp
    src/game.h
    src/game.cpp
    res/cards/10_of_clubs.png
    res/cards/10_of_diamonds.png
    res/cards/10_of_hearts.png
    res/cards/10_of_spades.png
    res/cards/6_of_clubs.png
    res/cards/6_of_diamonds.png
    res/cards/6_of_hearts.png
    res/cards/6_of_spades.png
    res/cards/7_of_clubs.png
    res/cards/7_of_diamonds.png
    res/cards/7_of_hearts.png
    res/cards/7_of_spades.png
    res/cards/8_of_clubs.png
    res/cards/8_of_diamonds.png
    res/cards/8_of_hearts.png
    res/cards/8_of_spades.png
    res/cards/9_of_clubs.png
    res/cards/9_of_diamonds.png
    res/cards/9_of_hearts.png
    res/cards/9_of_spades.png
    res/cards/ace_of_clubs.png
    res/cards/ace_of_diamonds.png
    res/cards/ace_of_hearts.png
    res/cards/ace_of_spades.png
    res/cards/backside_blue.png
    res/cards/backside_red.png
    res/cards/jack_of_clubs.png
    res/cards/jack_of_diamonds.png
    res/cards/jack_of_hearts.png
    res/cards/jack_of_spades.png
    res/cards/king_of_clubs.png
    res/cards/king_of_diamonds.png
    res/cards/king_of_hearts.png
    res/cards/king_of_spades.png
    res/cards/queen_of_clubs.png
    res/cards/queen_of_diamonds.png
    res/cards/queen_of_hearts.png
    res/cards/queen_of_spades.png)

  qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
  if(ANDROID)
    add_library(quteSkatGame SHARED ${PROJECT_SOURCES})
  else()
    add_executable(quteSkatGame ${PROJECT_SOURCES})
  endif()

  qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(quteSkatGame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS/macOS settings
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.quteSkatGame)
endif()
set_target_properties(
  quteSkatGame
  PROPERTIES ${BUNDLE_ID_OPTION} MACOSX_BUNDLE_BUNDLE_VERSION
             ${PROJECT_VERSION} MACOSX_BUNDLE_SHORT_VERSION_STRING
             ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} MACOSX_BUNDLE
             TRUE WIN32_EXECUTABLE
             TRUE)

include(GNUInstallDirs)
install(
  TARGETS quteSkatGame
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(QT_VERSION_MAJOR EQUAL 6)
  qt_finalize_executable(quteSkatGame)
endif()

# ---------------------------------------------------------

# Enable testing
enable_testing()

# GoogleTest & GoogleMock Setup
set(GTEST_DIR /usr/share/googletest)
set(GMOCK_DIR /usr/share/googletest/googlemock)

# Include GoogleTest and GoogleMock
include_directories(${GTEST_DIR}/googletest/include ${GMOCK_DIR}/include)

# Specify the binary directory for GoogleTest
add_subdirectory(${GTEST_DIR} ${CMAKE_BINARY_DIR}/googletest-build)

# Add the test executable Add test executable
add_executable(runTests tests/test_card.cpp src/card.cpp)

# Link GoogleTest properly
target_link_libraries(runTests gtest gtest_main)

# Register the tests
add_test(NAME CardTests COMMAND runTests)
