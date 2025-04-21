cmake_minimum_required(VERSION 3.16)

project(GOL VERSION 0.1.0 LANGUAGES CXX)

#set(CMAKE_CXX_COMPILER /opt/homebrew/Cellar/llvm/20.1.2/bin/clang++)
set(CMAKE_CXX_STANDARD 23)
option(vendored/SDL "Use vendored libraries" ON)
set(SDL_STATIC ON)

if(vendored/SDL)
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
else()
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-static)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp" "include/*.h")
add_executable(GOL WIN32 ${SOURCES})
install(TARGETS GOL
        RUNTIME DESTINATION bin)

target_link_libraries(GOL SDL3::SDL3)


