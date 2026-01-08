cmake_minimum_required(VERSION 3.20)
project(jarvis VERSION 4.0.0 LANGUAGES CXX DESCRIPTION "Голосовой помощник для управления компьютером с помощью искусственного интеллекта")

set(PROJECT_AUTHOR "Егор Иманаев")
set(PROJECT_COMPANY "Vubni")
string(TIMESTAMP CURRENT_YEAR "%Y")

set(FILE_VERSION "${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0")
set(PRODUCT_VERSION "${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0")
set(FILE_VERSION_STRING "${PROJECT_VERSION}")
set(PRODUCT_VERSION_STRING "${PROJECT_VERSION}")

# Настройка стандартов и автоматизации Qt
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Настройки путей
set(LIB_DIR "${CMAKE_SOURCE_DIR}/libs")
set(CMAKE_PREFIX_PATH "E:/Q/Qt") # Путь к установленному Qt
set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
set(CMAKE_TOOLCHAIN_FILE 
    "./vcpkg/scripts/buildsystems/vcpkg.cmake" 
    CACHE STRING "Путь к toolchain файлу vcpkg"
)

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
include_directories(include 
                    include/vosk 
                    include/src
                    build)

link_directories(${LIB_DIR})



add_definitions(-DCURL_STATICLIB -D_WIN32)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/app.rc"
    "${CMAKE_CURRENT_SOURCE_DIR}/app_generated.rc"
    @ONLY
)
set(APP_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/app_generated.rc")

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)


find_package(Qt6 REQUIRED COMPONENTS UiTools Core Gui Svg Widgets)


file(GLOB SCRIPT_SOURCES "scripts/*.cpp")
set(SOURCE_EXE main.cpp ${SCRIPT_SOURCES})

qt6_wrap_cpp(MOC_SOURCES
    include/src/animationcontroller.h
    include/src/TraySystem.h
    include/src/CustomNotification.h
    include/src/mainwindow.h
    include/src/switch.h
)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)

qt6_wrap_ui(UI_HEADERS ./design/main.ui)

add_custom_target(force_ui_update ALL
    DEPENDS ${UI_HEADERS}
)
qt_add_resources(RESOURCE_FILES "resources.qrc")
add_executable(jarvis ${SOURCE_EXE} ${MOC_SOURCES} ${UI_HEADERS} ${APP_RESOURCES})

if(WIN32)
    set_target_properties(jarvis PROPERTIES
        VS_GLOBAL_EnableManagedIncrementalBuild "false"
        VS_GLOBAL_MinimalRebuild "false"
        LINK_FLAGS "/MANIFEST:NO"
    )
    
    # Добавляем манифест отдельно
    if(EXISTS ${MANIFEST_FILE})
        add_custom_command(TARGET jarvis POST_BUILD
            COMMAND mt.exe -nologo -manifest "${MANIFEST_FILE}" "-outputresource:$<TARGET_FILE:jarvis>;#1"
            COMMENT "Embedding manifest..."
        )
    endif()
endif()

target_sources(jarvis PRIVATE ${RESOURCE_FILES})

target_link_libraries(jarvis
PRIVATE pv_recorder vosk nlohmann_json::nlohmann_json 
libcurl_a Normaliz.lib advapi32.lib
${CURL_LIBRARY} ws2_32 Crypt32 Wldap32
sndfile user32
SDL2 SDL2_mixer
Qt::Core
Qt::Gui
Qt::Widgets
Qt::UiTools
Qt::Svg
ucrt.lib
vcruntime.lib
msvcrt.lib
legacy_stdio_definitions.lib
ole32.lib
shlwapi.lib)

include_directories(${Qt6Gui_INCLUDE_DIRS})