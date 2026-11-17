cmake_minimum_required(VERSION 3.20)
project(gta4_reversed LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")

add_library(gta4_reversed SHARED
    source/dllmain.cpp
    source/Core/Hook.cpp
    source/Core/Log.cpp
    source/Core/Vector.cpp
    source/Core/Matrix.cpp
    source/Core/Pool.cpp
    source/Game/Timer.cpp
    source/Game/Clock.cpp
    source/Game/Weather.cpp
    source/Game/Cheat.cpp
    source/Game/Stats.cpp
    source/Game/General.cpp
    source/Game/Entity.cpp
    source/Game/DynamicEntity.cpp
    source/Game/Physical.cpp
    source/Game/Ped.cpp
    source/Game/Vehicle.cpp
    source/Game/Object.cpp
    source/Game/LoadingScreens.cpp
    source/Game/World.cpp
    source/Game/WeaponInfo.cpp
    source/Game/Cam.cpp
    source/Game/Camera.cpp
    source/Game/FileMgr.cpp
    source/Game/DummyObject.cpp
    source/Game/IplStore.cpp
    source/Game/Font.cpp
    source/Rage/fiStream.cpp
    source/Game/PlayerInfo.cpp
    source/Game/Wanted.cpp
    source/Game/Building.cpp
    source/Game/ModelInfo.cpp
    source/Game/TxdStore.cpp
    source/Game/Streaming.cpp
    source/Game/Pad.cpp
    source/Rage/phBound.cpp
    source/Rage/phBoundComposite.cpp
    source/Rage/phBoundBVH.cpp
    source/Rage/phArchetype.cpp
    source/Rage/phInst.cpp
    source/Rage/phCollider.cpp
    source/Rage/phArticulatedCollider.cpp
    source/Rage/phConstraint.cpp
    source/Rage/phLevelNew.cpp
    source/Rage/phSimulator.cpp
)

set_target_properties(gta4_reversed PROPERTIES
    OUTPUT_NAME "gta4_reversed"
    SUFFIX ".asi"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_include_directories(gta4_reversed PRIVATE
    "${CMAKE_SOURCE_DIR}/source"
)

target_precompile_headers(gta4_reversed PRIVATE
    source/StdInc.h
)

target_compile_definitions(gta4_reversed PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
)

target_compile_options(gta4_reversed PRIVATE
    /W4
    /MP
    /Zi
)

target_link_options(gta4_reversed PRIVATE
    /DEBUG
)
