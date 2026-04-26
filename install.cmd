@echo off
title RemoteDesk - Full Installer
echo ============================================
echo    REMOTEDESK - FULL INSTALLER
echo ============================================
echo.

:: ====== CHECK GIT ======
echo [1/5] Checking Git...
where git >nul 2>nul
if %errorlevel% neq 0 (
    echo Git not found! Downloading...
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/git-for-windows/git/releases/download/v2.47.0.windows.1/Git-2.47.0-64-bit.exe' -OutFile 'git-installer.exe'"
    echo Installing Git...
    git-installer.exe /VERYSILENT /NORESTART
    del git-installer.exe
    echo Git installed!
) else (
    echo Git OK
)

:: ====== CHECK CMAKE ======
echo.
echo [2/5] Checking CMake...
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo CMake not found! Downloading...
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/Kitware/CMake/releases/download/v3.31.0/cmake-3.31.0-windows-x86_64.msi' -OutFile 'cmake-installer.msi'"
    echo Installing CMake...
    msiexec /i cmake-installer.msi /quiet /norestart
    del cmake-installer.msi
    echo CMake installed!
) else (
    echo CMake OK
)

:: ====== CHECK QT ======
echo.
echo [3/5] Checking Qt...
if not exist "C:\Qt" (
    echo Qt not found!
    echo.
    echo ============================================
    echo    IMPORTANT!
    echo ============================================
    echo.
    echo You need to install Qt manually:
    echo 1. Go to https://www.qt.io/download-open-source
    echo 2. Download Qt Online Installer
    echo 3. Install Qt 6.x with MinGW compiler
    echo 4. Use default path C:\Qt
    echo.
    echo After Qt is installed, run this script again.
    echo.
    pause
) else (
    echo Qt OK
)

:: ====== INSTALL VCPKG ======
echo.
echo [4/5] Checking vcpkg...
if not exist "C:\vcpkg" (
    echo Installing vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
    cd C:\vcpkg
    bootstrap-vcpkg.bat
    echo.
    echo Installing libraries...
    vcpkg install protobuf:x64-windows libsodium:x64-windows zlib:x64-windows
    cd %~dp0
    echo vcpkg installed!
) else (
    echo vcpkg OK
)

:: ====== BUILD ======
echo.
echo [5/5] Building RemoteDesk...
if exist "build" rmdir /s /q build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
cd ..

echo.
echo ============================================
echo    DONE! Starting RemoteDesk...
echo ============================================
start build\Release\RemoteDesk.exe
pause
