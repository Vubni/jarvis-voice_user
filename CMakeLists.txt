cmake_minimum_required(VERSION 3.20)
project(jarvis VERSION 3.0 LANGUAGES CXX)

# Настройка стандартов и автоматизации Qt
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Настройки путей
set(LIB_DIR "${CMAKE_SOURCE_DIR}/libs")
set(CMAKE_PREFIX_PATH "E:/Q/Qt")
set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
set(CMAKE_TOOLCHAIN_FILE 
    "E:/g++/jarvis_voice/user/vcpkg/scripts/buildsystems/vcpkg.cmake" 
    CACHE STRING "Путь к toolchain файлу vcpkg"
)

# Настройки для Windows
if(WIN32)
    set(APP_RESOURCES app.rc)

    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
    
    # Добавляем манифест для правильной работы с треем
    set(MANIFEST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/app.manifest")
    if(EXISTS ${MANIFEST_FILE})
        # Создаем ресурсный файл для манифеста
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/app.rc" "CREATEPROCESS_MANIFEST_RESOURCE_ID RT_MANIFEST \"${MANIFEST_FILE}\"")
        
        # Включаем компилятор ресурсов
        enable_language(RC)
        set(RC_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
    else()
        message(WARNING "Manifest file not found: ${MANIFEST_FILE}")
    endif()
endif()

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
include_directories(include 
                    include/vosk 
                    include/src
                    build)

link_directories(${LIB_DIR})



add_definitions(-DCURL_STATICLIB -D_WIN32)


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
)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)

qt6_wrap_ui(UI_HEADERS ./design/main.ui)

add_custom_target(force_ui_update ALL
    DEPENDS ${UI_HEADERS}
)
qt_add_resources(RESOURCE_FILES "resources.qrc")
add_executable(jarvis ${SOURCE_EXE} ${MOC_SOURCES} ${UI_HEADERS} ${APP_RESOURCES})

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