cmake_minimum_required(VERSION 3.20)
project(jarvis VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LIB_DIR "${CMAKE_SOURCE_DIR}/libs")
set(CMAKE_PREFIX_PATH "E:/Q/Qt")

set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
set(CMAKE_TOOLCHAIN_FILE 
    "E:/g++/jarvis_voice/user/vcpkg/scripts/buildsystems/vcpkg.cmake" 
    CACHE STRING "Путь к toolchain файлу vcpkg"
)

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
include_directories(include 
                    include/vosk 
                    include/src
                    build)

link_directories(${LIB_DIR})

add_library(base64 ${CMAKE_SOURCE_DIR}/libs/base64.cpp)


add_definitions(-DCURL_STATICLIB -D_WIN32)


set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)


find_package(Qt6 REQUIRED COMPONENTS UiTools Core Gui Widgets)


file(GLOB SCRIPT_SOURCES "scripts/*.cpp")
set(SOURCE_EXE main.cpp ${SCRIPT_SOURCES})

qt6_wrap_cpp(MOC_SOURCES
    include/src/animationcontroller.h
)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)

qt6_wrap_ui(UI_HEADERS main.ui)

add_executable(jarvis ${SOURCE_EXE} ${MOC_SOURCES} ${UI_HEADERS})

# if(MSVC)
#     # Копирование CRT DLL
#     get_filename_component(VC_REDIST_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64;InstallDir]" ABSOLUTE)
#     set(VCRUNTIME_DLLS "${VC_REDIST_DIR}/vcruntime140.dll" "${VC_REDIST_DIR}/vcruntime140_1.dll" "${VC_REDIST_DIR}/msvcp140.dll")
#     add_custom_command(TARGET jarvis POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#         "${LIB_DIR}/libcurl.dll"
#         "${LIB_DIR}/zlib1.dll"
#         $<TARGET_FILE_DIR:jarvis>
#     )
# endif()

target_link_libraries(jarvis
PRIVATE pv_recorder vosk nlohmann_json::nlohmann_json 
libcurl_a Normaliz.lib advapi32.lib
${CURL_LIBRARY} ws2_32 Crypt32 Wldap32 base64
sndfile user32
SDL2 SDL2_mixer
Qt::Core
Qt::Gui
Qt::Widgets
Qt::UiTools
ucrt.lib
vcruntime.lib
msvcrt.lib
legacy_stdio_definitions.lib)

include_directories(${Qt6Gui_INCLUDE_DIRS})


# add_custom_command(TARGET jarvis POST_BUILD
#     COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
#         --verbose 1
#         --no-compiler-runtime
#         --no-translations
#         "$<TARGET_FILE:jarvis>"
#     WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
# )