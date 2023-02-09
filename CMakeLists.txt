cmake_minimum_required(VERSION 3.9)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(async VERSION ${PROJECT_VERSION})

add_library(async SHARED async.cpp async.h Command.cpp Controller.cpp Receiver.cpp)

add_executable(async_cli main.cpp)

set_target_properties(async_cli async PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        )
target_include_directories(async
        PRIVATE "${CMAKE_BINARY_DIR}"
        )

target_link_libraries(async_cli async)

if (MSVC)
    target_compile_options(async_cli PRIVATE
            /W4
            )
    target_compile_options(async PRIVATE
            /W4
            )
else ()
    target_compile_options(async_cli PRIVATE
            -Wall -Wextra -pedantic -Werror
            )
    target_compile_options(async PRIVATE
            -Wall -Wextra -pedantic -Werror
            )
endif()

install(TARGETS async_cli RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)
