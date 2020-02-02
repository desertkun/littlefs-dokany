cmake_minimum_required(VERSION 3.5.0)
project(littlefs_dokany C ASM)

file(GLOB USER_SOURCES "src/*.c")
file(GLOB LITTLEFS_SOURCES "littlefs/*.c")

set(DOKANY_LIBS "$ENV{DokanLibrary1_LibraryPath_x64} $ENV{DokanLibrary1_LibraryPath_x86}")
message(STATUS "Dokany libs are installed here: ${DOKANY_LIBS}")

find_library(dokan1 NAMES dokan1
        PATHS $ENV{DokanLibrary1_LibraryPath_x64}
        $ENV{DokanLibrary1_LibraryPath_x86})

include_directories(
    littlefs
    dokany
    dokany/sys)

add_executable(${PROJECT_NAME} ${USER_SOURCES} ${LITTLEFS_SOURCES})

target_link_libraries(${PROJECT_NAME} ${dokan1})
