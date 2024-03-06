cmake_minimum_required (VERSION 3.8)
project (simplerf)

# get rid of annoying MSVC warnings.
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set (CMAKE_CXX_STANDARD 17)
option(USE_VULKAN "Enable GPU implementation via Vulkan" OFF)

find_package(OpenMP)

if(USE_VULKAN)

  find_package(Vulkan)

  if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
  elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
  elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
  endif()

  add_subdirectory(external/volk)

  add_compile_definitions(USE_VOLK)
  add_compile_definitions(USE_VULKAN)

  include_directories(${Vulkan_INCLUDE_DIR})

  include_directories("external/LiteMath"
                      "external/vk-utils"
                      "external/volk"
                      "external/json-master")

  link_directories("../volk")
else()
  include_directories("external/LiteMath"
                      "external/json-master")
endif()

include_directories(".")
if(WIN32)
  link_directories(${ADDITIONAL_LIBRARY_DIRS})
endif()

#uncomment this to detect broken memory problems via gcc sanitizers
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined -fsanitize=bounds-strict")

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes")
  set(PLATFORM_DEPENDEPNT_LIBS ${Vulkan_LIBRARY} dl)
endif()

if(USE_VULKAN)
  add_executable(app main.cpp
                 ray_marcher/RayMarcher.cpp
                 external/LiteMath/Image2d.cpp

                 ray_marcher/RayMarcher_generated.cpp
                 ray_marcher/RayMarcher_generated_ds.cpp
                 ray_marcher/RayMarcher_generated_init.cpp

                 external/vk-utils/vk_utils.cpp
                 external/vk-utils/vk_copy.cpp
                 external/vk-utils/vk_context.cpp
                 external/vk-utils/vk_alloc_simple.cpp
                 external/vk-utils/vk_buffers.cpp
                 external/vk-utils/vk_images.cpp
                 external/vk-utils/vk_pipeline.cpp
                 external/vk-utils/vk_descriptor_sets.cpp)

  set_target_properties(app PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
  target_link_libraries(app LINK_PUBLIC OpenMP::OpenMP_CXX volk "${PLATFORM_DEPENDEPNT_LIBS}")

else()
  add_executable(app main.cpp 
                 ray_marcher/RayMarcher.cpp 
                 external/LiteMath/Image2d.cpp)

  set_target_properties(app PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

  target_link_libraries(app LINK_PUBLIC OpenMP::OpenMP_CXX)
endif()