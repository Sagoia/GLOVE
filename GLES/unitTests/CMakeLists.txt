message(STATUS "  Building GLES Unit Tests")

set(SOURCES
    utils/arrays_tests.cpp
    resources/refObject_test.cpp
)

set(LIBS
    gtest
    gtest_main
    pthread
    GLESv2
)

include_directories(${GLES_PATH}/source
                    ${GLES_PATH}/include
                    ${EGL_PATH}/include
                    ${GTEST_PATH}/include
                    ${Vulkan_INCLUDE_DIR}
                    ${CMAKE_INSTALL_FULL_INCLUDEDIR})

link_directories(${GTEST_PATH}/lib
                 ${CMAKE_BINARY_DIR}/GLES/source)

if(WIN32)
    add_library(gtest_main STATIC IMPORTED)
    set_target_properties(gtest_main PROPERTIES IMPORTED_LOCATION ${GTEST_PATH}/lib/gtest_main.lib)
    add_library(gtest STATIC IMPORTED)
    set_target_properties(gtest PROPERTIES IMPORTED_LOCATION ${GTEST_PATH}/lib/gtest.lib)
else()
    add_library(gtest_main STATIC IMPORTED)
    set_target_properties(gtest_main PROPERTIES IMPORTED_LOCATION ${GTEST_PATH}/lib/libgtest_main.a)
    add_library(gtest STATIC IMPORTED)
    set_target_properties(gtest PROPERTIES IMPORTED_LOCATION ${GTEST_PATH}/lib/libgtest.a)
endif()

add_executable(gles_unit_tests ${SOURCES})
target_link_libraries(gles_unit_tests ${LIBS})
add_dependencies(gles_unit_tests GLESv2)
