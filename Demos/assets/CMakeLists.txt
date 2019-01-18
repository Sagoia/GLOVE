message(STATUS "  Copy textures")
file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/textures
     DESTINATION ${CMAKE_BINARY_DIR}/Demos/assets)

message(STATUS "  Copy shaders")
file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/shaders
     DESTINATION ${CMAKE_BINARY_DIR}/Demos/assets)

if(WIN32 OR IOS)
    message(STATUS "  Copy textures")
    file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/textures
         DESTINATION ${CMAKE_BINARY_DIR}/Bin/assets)

    message(STATUS "  Copy shaders")
    file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/shaders
         DESTINATION ${CMAKE_BINARY_DIR}/Bin/assets)
else()
    message(STATUS "  Copy textures")
    file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/textures
         DESTINATION ${CMAKE_BINARY_DIR}/Demos/assets)

    message(STATUS "  Copy shaders")
    file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/shaders
         DESTINATION ${CMAKE_BINARY_DIR}/Demos/assets)
endif()
