cmake_minimum_required(VERSION 3.20)
project(jarvis VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_language(CUDA)

set(LIB_DIR "${CMAKE_SOURCE_DIR}/libs")
set(SOURCE_EXE main.cpp)
set(GGML_CUDA_ARCHITECTURES 52 61 70 86) 
set(WHISPER_CPP_DIR "${PROJECT_SOURCE_DIR}/whisper-cpp-cuda")

find_package(CUDAToolkit REQUIRED)
set(SDL2_PATH "E:/SDL2")
find_package(SDL2 REQUIRED PATHS ${SDL2_PATH} NO_DEFAULT_PATH)

add_definitions(-DWHISPER_CUDA=1)
add_definitions(-DGGML_USE_CUBLAS=1)

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
include_directories(include include/vosk ${WHISPER_CPP_DIR})

# Источники
file(GLOB WHISPER_SOURCES
    ${WHISPER_CPP_DIR}/*.c
    ${WHISPER_CPP_DIR}/ggml-cuda/*.c
    ${WHISPER_CPP_DIR}/*.h
    ${WHISPER_CPP_DIR}/ggml-cuda/*.h
    ${WHISPER_CPP_DIR}/whisper.h
    ${WHISPER_CPP_DIR}/whisper.cpp
)

# CUDA-исходники
file(GLOB WHISPER_CUDA_SOURCES 
    ${WHISPER_CPP_DIR}/ggml-cuda/*.cu
    ${WHISPER_CPP_DIR}/*.cu)

# Основная библиотека с поддержкой CUDA
add_library(whisper STATIC
    ${WHISPER_SOURCES}
    ${WHISPER_CUDA_SOURCES}
)

# Настройка CUDA
set_property(TARGET whisper PROPERTY CUDA_ARCHITECTURES ${GGML_CUDA_ARCHITECTURES})
set_property(TARGET whisper PROPERTY CUDA_SELECT_NVCC_ARCH_FLAGS "Auto")

target_compile_definitions(whisper PRIVATE
    WHISPER_CUDA
    GGML_USE_CUDA
)

target_include_directories(whisper PUBLIC
    ${WHISPER_CPP_DIR}
    ${WHISPER_CPP_DIR}/include
)


link_directories(${LIB_DIR})

add_library(base64 ${CMAKE_SOURCE_DIR}/libs/base64.cpp)

add_definitions(-DCURL_STATICLIB)
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)

add_executable(jarvis ${SOURCE_EXE} command_processor.cpp http_client.cpp)

target_link_libraries(jarvis
PRIVATE pv_recorder vosk nlohmann_json::nlohmann_json libcurl_a
${CURL_LIBRARY} ws2_32 Crypt32 Wldap32 base64
PRIVATE whisper
PRIVATE CUDA CUDA::cudart CUDA::cublas CUDA::cublasLt CUDA::cufft SDL2::SDL2
sndfile)


add_custom_command(TARGET jarvis
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIB_DIR} ${CMAKE_BINARY_DIR}
)