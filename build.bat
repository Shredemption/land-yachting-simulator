@echo off
setlocal

set BUILD_DIR="build"
set BUILD_TYPE="Release"

set CLEAN_BUILD=false
if "%1"=="/clean" (
    set CLEAN_BUILD=true
)

if %CLEAN_BUILD%==true (
  rmdir /s /q %BUILD_DIR%
  mkdir %BUILD_DIR%
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

marama.exe

endlocal