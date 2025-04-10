
#Auto Generated Root CMake file by aura CLI
#None

#====================Basic Configuration===================

cmake_minimum_required(VERSION 3.6...3.31)
project(tervinal VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(STATIC_LINK "Enable static linking" ON)
option(ENABLE_TESTS "GTests" OFF)
if(STATIC_LINK)
  if (WIN32)
      set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  else()
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
  endif()
endif()

set(COMPANY "None")
string(TIMESTAMP CURRENT_YEAR "%Y")
set(COPYRIGHT "Copyright(c) ${CURRENT_YEAR} ${COMPANY}.")

include_directories(${CMAKE_BINARY_DIR})
configure_file(res/config.h.in tervinalconfig.h)

#@find Warning: Do not remove this line
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Enabling secure coding features for Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic        # General warnings
        -Wshadow -Wold-style-cast       # Detect potential issues
        -Wcast-align -Wnull-dereference # Runtime safety
        -Wformat=2 -Wformat-security    # Secure formatting
        -fstack-protector-strong        # Stack protection
        -D_FORTIFY_SOURCE=2             # Buffer security
        -fno-common                     # Avoid common symbol issues
        #-Werror                         # Treat warnings as errors
    )
endif()


#====================Configuration===================

if(NOT ENABLE_TESTS)
    add_executable(${PROJECT_NAME} "src/main.cpp")# Add your Source Files here
    #@link Warning: Do not remove this line
    target_link_libraries(${PROJECT_NAME}
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
    target_link_libraries(${PROJECT_NAME} PRIVATE $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)
else()
    message(STATUS "Tests are enabled")
    find_package(GTest)
    enable_testing()
    add_executable(tests "tests/main.cpp")
    target_link_libraries(tests GTest::GTest GTest::Main)
endif()
