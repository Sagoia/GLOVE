message(STATUS "  Building eglut library")

# Create EGLUT Lib
set(EGLUT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/eglut.c
)

set(EGLUT_HEADS
    ${CMAKE_CURRENT_SOURCE_DIR}/eglut.h
    ${CMAKE_CURRENT_SOURCE_DIR}/eglutint.h
)

if(${USE_SURFACE} STREQUAL DISPLAY)
    set(EGLUT_SOURCES
        ${EGLUT_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/eglut_native.c
    )
elseif(${USE_SURFACE} STREQUAL XCB)
    set(EGLUT_SOURCES
        ${EGLUT_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/eglut_x11.c
    )
elseif(${USE_SURFACE} STREQUAL WINDOWS)
    set(EGLUT_SOURCES
        ${EGLUT_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/eglut_windows.c
    )
elseif(${USE_SURFACE} STREQUAL IOS)
    set(EGLUT_HEADS
        ${EGLUT_HEADS}
        ${CMAKE_CURRENT_SOURCE_DIR}/eglut_ios_app.h
    )
    set(EGLUT_SOURCES
        
        ${EGLUT_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/eglut_ios.m
        ${CMAKE_CURRENT_SOURCE_DIR}/eglut_ios_app.m
    )
elseif(${USE_SURFACE} STREQUAL ANDROID)

endif()

if(WIN32 OR IOS)
    add_library(EGLUT STATIC ${EGLUT_HEADS} ${EGLUT_SOURCES})
    target_link_libraries(EGLUT ${LIBS})
    if (IOS)
        set_target_properties(EGLUT PROPERTIES  XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES"
                                                XCODE_ATTRIBUTE_VALID_ARCHS "arm64")
    endif()
else()
    add_library(EGLUT SHARED ${EGLUT_SOURCES})
endif()
