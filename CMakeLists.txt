file(GLOB_RECURSE RENDERCORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
file(GLOB_RECURSE RENDERCORE_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

add_library(RenderCore STATIC
    ${RENDERCORE_SOURCES}
    ${RENDERCORE_HEADERS}
)

target_include_directories(RenderCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(RenderCore PRIVATE Core/pch.h)

include(FetchContent)
FetchContent_Declare(
  DirectXTex
  GIT_REPOSITORY https://github.com/microsoft/DirectXTex.git
  GIT_TAG        mar2026
)
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_SAMPLE OFF CACHE BOOL "" FORCE)
set(BUILD_DX12 OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(DirectXTex)

target_link_libraries(RenderCore PUBLIC
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
    DirectXTex
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RENDERCORE_SOURCES} ${RENDERCORE_HEADERS})

if (MSVC)
    target_compile_definitions(RenderCore PRIVATE _LIB UNICODE _UNICODE)
endif()