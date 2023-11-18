cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(VulkanTutorial LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

option(AUTO_LOCATE_VULKAN "AUTO_LOCATE_VULKAN" ON)

if(AUTO_LOCATE_VULKAN)
    find_package(Vulkan)
    
    # Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
    if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
        set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
        STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
    endif()
         
    if(NOT Vulkan_FOUND)
        message(STATUS "Failed to locate Vulkan SDK, retrying again...")
        if(EXISTS "${VULKAN_PATH}")
            message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
        else()
            message("Error: Unable to locate Vulkan SDK. Please turn off auto locate option by specifying 'AUTO_LOCATE_VULKAN' as 'OFF'")
            message("and specify manually path using 'VULKAN_SDK' and 'VULKAN_VERSION' variables in the CMakeLists.txt.")
            return()
        endif()
    endif()
else()
    message(STATUS "Attempting to locate Vulkan SDK using manual path......")
    set(VULKAN_SDK "C:/VulkanSDK")
    set(VULKAN_VERSION "1.3.211.0")
    set(VULKAN_PATH "${VULKAN_SDK}/${VULKAN_VERSION}")
    message(STATUS "Using manual specified path: ${VULKAN_PATH}")

    # Check if manual set path exists
    if(NOT EXISTS "${VULKAN_PATH}")
        message("Error: Unable to locate this Vulkan SDK path VULKAN_PATH: ${VULKAN_PATH}, please specify correct path.
        For more information on correct installation process, please refer to subsection 'Getting started with Lunar-G SDK'
        and 'Setting up first project with CMake' in Chapter 3, 'Shaking hands with the device' in this book 'Learning Vulkan', ISBN - 9781786469809.")
       return()
    endif()
endif()

file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR} "src/*.h" )

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR} "src/*.cpp" "src/*.h" "src/*.hpp" "Libraries/SPIRV-Reflect/spirv_reflect.c")

message(STATUS "SRC LIST: = ${SRC_FILES}")

add_executable(maintest  ${SRC_FILES})

INCLUDE_DIRECTORIES(${Vulkan_INCLUDE_DIRS})
#message(STATUS "Vulkan_INCLUDE_DIRS: = ${Vulkan_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libraries/glfw/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libraries/tinyobjloader)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libraries/assimp/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libraries/assimp/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libraries/SPIRV-Reflect/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libraries/SPIRV-Reflect)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(Libraries/glfw)
add_subdirectory(Libraries/glm)
add_subdirectory(Libraries/tinyobjloader)
add_subdirectory(Libraries/assimp)
add_subdirectory(Libraries/SPIRV-Reflect)

target_link_libraries(maintest 
    PUBLIC
    Vulkan::Vulkan
    glfw
    tinyobjloader
    assimp
    )
