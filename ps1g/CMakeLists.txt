include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)

FetchContent_Declare(
  nfd
  URL https://github.com/btzy/nativefiledialog-extended/archive/refs/tags/v1.2.0.zip
)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.92.6
)

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(nfd)
FetchContent_MakeAvailable(imgui)

find_package(OpenGL REQUIRED)

file(GLOB_RECURSE PS1G_SOURCES "src/*.cpp")

add_executable (ps1g ${PS1G_SOURCES} "ps1g.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ps1g PROPERTY CXX_STANDARD 20)
endif()

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR} 
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw)

target_include_directories(ps1g PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(ps1g PRIVATE imgui OpenGL::GL nfd)
