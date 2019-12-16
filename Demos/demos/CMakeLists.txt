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

if (APPLE)
    set(CMAKE_MACOS_RPATH TRUE)
    find_library(FOUNDATION Foundation)
    find_library(QUARTZCORE QuartzCore)
    find_library(COCOA      Cocoa)
    find_library(APPKIT APPKit)
    find_library(METAL Metal)

    #file(GLOB XIB_FILES LIST_DIRECTORIES false $(CMAKE_SOURCE_DIR)/Demos/demos/macOS/*.storyboard)
    set(XIB_FILES   ${CMAKE_SOURCE_DIR}/Demos/demos/macOS/Main.storyboard)
    set_source_files_properties(${XIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    file(GLOB RES_FILES ${CMAKE_BINARY_DIR}/Demos/assets)
    set_source_files_properties(${RES_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

foreach(example ${DEMOS})
    if (APPLE)
        add_executable(${example} MACOSX_BUNDLE ${example}.c
                                                ${CMAKE_SOURCE_DIR}/Demos/demos/macOS/main.m
                                                ${XIB_FILES}
                                                ${RES_FILES})
        set_target_properties(${example} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Demos/demos/plist.in
                                                    XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES"
                                                    MACOSX_RPATH TRUE
                                                    SKIP_BUILD_RPATH FALSE
                                                    XCODE_ATTRIBUTE_VALID_ARCHS "x86_64")
        target_link_libraries(${example} ${FOUNDATION})
        target_link_libraries(${example} ${QUARTZCORE})
        target_link_libraries(${example} ${COCOA})
        target_link_libraries(${example} ${APPKIT})
        target_link_libraries(${example} ${METAL})
        target_link_libraries(${example} GRAPHICS_ENGINE EGLUT ${LIBS} ${VULKAN_LIBRARY})
    else()
        set_property(SOURCE ${example}.c APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_BINARY_DIR}/Demos/assets/shaders ${CMAKE_BINARY_DIR}/Demos/assets/textures)
        add_executable(${example} ${example}.c)
        target_link_libraries(${example} GRAPHICS_ENGINE EGLUT ${LIBS})
        add_dependencies(${example} GLESv2 EGL)
    endif()
endforeach()

if(WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_all_samples.bat ${CMAKE_BINARY_DIR}/bin COPYONLY)
elseif(APPLE)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_all_samples_mac.sh ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
else()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_all_samples.sh ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endif()
