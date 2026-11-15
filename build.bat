@echo off
setlocal

where cl >nul 2>&1
if errorlevel 1 (
    if exist "%USERPROFILE%\.portable-msvc\msvc\setup_x86.bat" (
        call "%USERPROFILE%\.portable-msvc\msvc\setup_x86.bat"
    )
)

if not exist "build\Release\CMakeCache.txt" (
    echo [0/2] Configuring CMake...
    cmake -S . -B build/Release -G Ninja -DCMAKE_BUILD_TYPE=Release
    if errorlevel 1 goto error
)

echo [1/2] Building gta4_reversed.asi...
cmake --build build/Release
if errorlevel 1 goto error

if defined GTA4_GAME_DIR (
    set "GTA4_GAME_DIR=%GTA4_GAME_DIR:"=%"
    if exist "%GTA4_GAME_DIR%\" (
        echo [2/2] Copying to GTA4_GAME_DIR...
        copy /Y "bin\Release\gta4_reversed.asi" "%GTA4_GAME_DIR%\gta4_reversed.asi" >nul || goto copy_error
        if exist "bin\Release\gta4_reversed.pdb" (
            copy /Y "bin\Release\gta4_reversed.pdb" "%GTA4_GAME_DIR%\gta4_reversed.pdb" >nul || goto copy_error
        )
    )
)

echo ========================================================
echo SUCCESS: gta4_reversed.asi built!
echo ========================================================
endlocal
exit /b 0

:copy_error
echo [ERROR] Failed to copy files to GTA4_GAME_DIR (game may be running).
endlocal
exit /b 1

:error
echo [ERROR] Build failed.
endlocal
exit /b 1