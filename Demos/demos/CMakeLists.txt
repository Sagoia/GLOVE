message(STATUS "  Building demos")

set(DEMOS
    triangle2d_one_color
    triangle2d_split_colors
    circle2d_sdf
    texture2d_color
    cube3d_vertexcolors
    cube3d_textures
    render_to_texture_filter_gamma
    render_to_texture_filter_invert
    render_to_texture_filter_grayscale
    render_to_texture_filter_sobel
    render_to_texture_filter_boxblur
)

if (IOS)
    find_library(FOUNDATION Foundation)
    find_library(QUARTZCORE QuartzCore)
    find_library(IOSURFACE IOSurface)
    find_library(UIKIT UIKit)
    find_library(METAL Metal)

    file(GLOB XIB_FILES ${CMAKE_SOURCE_DIR}/Demos/demos/ios/*.storyboard)
    set_source_files_properties(${XIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    file(GLOB RES_FILES ${CMAKE_BINARY_DIR}/Demos/assets)
    set_source_files_properties(${RES_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

foreach(example ${DEMOS})
    if (IOS)
        add_executable(${example} MACOSX_BUNDLE ${example}.c
                                                ${CMAKE_SOURCE_DIR}/Demos/demos/ios/main.m
                                                ${XIB_FILES}
                                                ${RES_FILES})
        set_target_properties(${example} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Demos/demos/plist.in
                                                    XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES"
                                                    XCODE_ATTRIBUTE_VALID_ARCHS "arm64")
        target_link_libraries(${example} ${FOUNDATION})
        target_link_libraries(${example} ${QUARTZCORE})
        target_link_libraries(${example} ${IOSURFACE})
        target_link_libraries(${example} ${UIKIT})
        target_link_libraries(${example} ${METAL})
        target_link_libraries(${example} GRAPHICS_ENGINE EGLUT ${LIBS} ${VULKAN_LIBRARY})
    else()
        set_property(SOURCE ${example}.c APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_BINARY_DIR}/Demos/assets/shaders ${CMAKE_BINARY_DIR}/Demos/assets/textures)
        add_executable(${example} ${example}.c)
        target_link_libraries(${example} GRAPHICS_ENGINE EGLUT ${LIBS})
        add_dependencies(${example} GLESv2 EGL)
    endif()
endforeach()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_all_samples.sh ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
