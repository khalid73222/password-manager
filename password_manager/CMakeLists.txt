cmake_minimum_required(VERSION 3.10)
project(PasswordManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/nlohmann
)

# Source files
set(SOURCES
    src/main.cpp
    src/MainFrame.cpp
    src/AddPasswordDialog.cpp
    src/PasswordDialog.cpp
    src/PasswordGeneratorDialog.cpp
    src/PasswordManager.cpp
    src/StorageManager.cpp
    src/CryptoUtils.cpp
)

# Header files (for IDE)
set(HEADERS
    include/MainFrame.h
    include/AddPasswordDialog.h
    include/PasswordDialog.h
    include/PasswordGeneratorDialog.h
    include/PasswordManager.h
    include/StorageManager.h
    include/PasswordEntry.h
    include/CryptoUtils.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})

# Copy nlohmann json if needed
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/nlohmann/json.hpp)
    message(STATUS "Please ensure nlohmann/json.hpp is in the nlohmann folder")
endif()