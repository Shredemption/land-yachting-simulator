@echo off
setlocal

if "%~1"=="" (
    set "BUILD_TYPE=Release"
) else (
    set "BUILD_TYPE=%~1"
)

set "BUILD_DIR=build"

set "CLEAN_BUILD=false"
if /i "%~2"=="/clean" (
    set "CLEAN_BUILD=true"
)

if /i "%CLEAN_BUILD%"=="true" (
    echo Cleaning build directory: %BUILD_DIR%
    rmdir /s /q "%BUILD_DIR%"
    mkdir "%BUILD_DIR%"
)

if not exist %BUILD_DIR% mkdir %BUILD_DIR%

pushd %BUILD_DIR%

conan install .. ^
    --output-folder=. ^
    --build=missing ^
    --settings=build_type=%BUILD_TYPE% ^
    --settings=compiler=msvc ^
    --settings=compiler.runtime=dynamic ^
    --settings=compiler.cppstd=20

cmake .. ^
    -G "Visual Studio 17 2022" ^
    -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

cmake --build . --config %BUILD_TYPE%

popd

if not exist %BUILD_TYPE%\resources mkdir %BUILD_TYPE%\resources

robocopy resources %BUILD_TYPE%\resources /MIR /NFL /NDL /NJH /NJS /NC /NS /NP >nul
robocopy thirdparty\Ultralight\resources %BUILD_TYPE%\resources /E /XO /NFL /NDL /NJH /NJS >nul

echo Resource copy complete.

if /i not "%BUILD_TYPE%"=="Debug" (
    pushd %BUILD_TYPE%
    marama.exe
    popd
)

endlocal