cmake_minimum_required(VERSION 3.16)

project(
  QuteSkatGame
  VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui DBus)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Include the src directory for header files
include_directories(src)

set(PROJECT_SOURCES
    src/main.cpp
    src/table.cpp
    src/table.h
    src/table.ui
    src/game.cpp
    src/game.h
    src/card.h
    src/card.cpp
    src/cardvec.h
    src/cardvec.cpp
    src/game.h
    src/main.cpp
    src/game.cpp
    src/player.h
    src/player.cpp
    src/definitions.h
    README.md
    res/resources.qrc)

qt_add_executable(quteSkatGame MANUAL_FINALIZATION ${PROJECT_SOURCES})

target_link_libraries(quteSkatGame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                           Qt6::DBus)

# Qt for iOS/macOS settings
if(QT_VERSION_MAJOR EQUAL 6 AND QT_VERSION_MINOR VERSION_LESS 1)
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

# Find GoogleTest and Qt6
set(GTEST_DIR /usr/share/googletest)
set(GMOCK_DIR /usr/share/googletest/googlemock)

find_package(Qt6 REQUIRED COMPONENTS Core Test Widgets Gui)

# Include GoogleTest and GoogleMock
include_directories(${GTEST_DIR}/googletest/include ${GMOCK_DIR}/include)

# Add GoogleTest
add_subdirectory(${GTEST_DIR} ${CMAKE_BINARY_DIR}/googletest-build)

# Define test executable Define sources without main.cpp
set(TEST_SOURCES ${PROJECT_SOURCES})
list(REMOVE_ITEM TEST_SOURCES src/main.cpp)

# Define test executable
add_executable(runTests tests/tst_reizen.cpp tests/tst_bieten.cpp
                        ${TEST_SOURCES})

# Link GoogleTest and Qt6
target_link_libraries(runTests gtest gtest_main Qt6::Core Qt6::Test
                      Qt6::Widgets)

# Register the tests
add_test(NAME CardTests COMMAND runTests)
