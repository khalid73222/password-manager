cmake_minimum_required(VERSION 3.10)
project(PasswordManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/nlohmann
)

set(SOURCES
    src/main.cpp
    src/MainFrame.cpp
    src/AddPasswordDialog.cpp
    src/PasswordDialog.cpp
    src/PasswordGeneratorDialog.cpp
    src/CategoryManagerDialog.cpp
    src/PasswordManager.cpp
    src/StorageManager.cpp
    src/CryptoUtils.cpp
    src/PasswordEntry.cpp
    src/PasswordStrengthChecker.cpp
)

set(HEADERS
    include/MainFrame.h
    include/AddPasswordDialog.h
    include/PasswordDialog.h
    include/PasswordGeneratorDialog.h
    include/CategoryManagerDialog.h
    include/PasswordManager.h
    include/StorageManager.h
    include/PasswordEntry.h
    include/CryptoUtils.h
    include/PasswordStrengthChecker.h
    include/PinDialog.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})