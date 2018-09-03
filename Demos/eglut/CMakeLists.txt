message(STATUS "  Building eglut library")

# Create EGLUT Lib
set(EGLUT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/eglut.c
)

if(USE_SURFACE STREQUAL "DISPLAY")
    set(EGLUT_SOURCES
        ${EGLUT_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/eglut_native.c
    )
elseif(USE_SURFACE STREQUAL "XCB")
    set(EGLUT_SOURCES
        ${EGLUT_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/eglut_x11.c
    )
elseif(USE_SURFACE STREQUAL "ANDROID")

endif()

add_library(EGLUT SHARED ${EGLUT_SOURCES})
