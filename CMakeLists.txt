cmake_minimum_required(VERSION 3.20)
project(ProiectPOO)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

# ============ GLFW ============
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ============ IMGUI ============
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.91.0
)
FetchContent_MakeAvailable(imgui)

# ============ SQLITE3 ============
FetchContent_Declare(
        sqlite3
        URL https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip
)
FetchContent_MakeAvailable(sqlite3)

# ============ EXECUTABLE ============
add_executable(ProiectPOO
        main.cpp
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${sqlite3_SOURCE_DIR}/sqlite3.c
        models/Produs.h
        models/Categorie.h
        models/Furnizor.h
        models/Zona.h
        models/Tranzactie.h
        models/User.h
        data/sha256.h
        data/Database.h
        core/Depozit.h
        core/AlertaManager.h
        core/Raport.h
        ui/LoginUI.h
        ui/MainUI.h
        tests/TestRunner.h
        models/EntitateDepozit.h
)

target_include_directories(ProiectPOO PRIVATE
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${sqlite3_SOURCE_DIR}
        ${glfw_SOURCE_DIR}/include
)

target_link_libraries(ProiectPOO PRIVATE
        glfw
        opengl32
)