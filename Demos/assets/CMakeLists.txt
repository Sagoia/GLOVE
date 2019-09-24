message(STATUS "  Copy textures")
file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/textures
     DESTINATION ${CMAKE_BINARY_DIR}/Demos/assets)

message(STATUS "  Copy shaders")
file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/shaders
     DESTINATION ${CMAKE_BINARY_DIR}/Demos/assets)

if(WIN32)
    message(STATUS "  Copy textures")
    file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/textures
         DESTINATION ${CMAKE_BINARY_DIR}/assets)

    message(STATUS "  Copy shaders")
    file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/shaders
         DESTINATION ${CMAKE_BINARY_DIR}/assets)
else()
    message(STATUS "  Copy textures")
    file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/textures
         DESTINATION ${CMAKE_BINARY_DIR}/assets)

    message(STATUS "  Copy shaders")
    file(COPY        ${CMAKE_SOURCE_DIR}/Demos/assets/shaders
         DESTINATION ${CMAKE_BINARY_DIR}/assets)
endif()
