[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white)](https://github.com/pre-commit/pre-commit)

| **`Windows`** | **`Linux(x86/s390x)`** |
|:-----------------:|:-----------------:|
[![Build status](https://ci.appveyor.com/api/projects/status/pijhpvbd7l36x800/branch/develop?svg=true)](https://ci.appveyor.com/project/rokoDev/buffer/branch/develop)|[![CircleCI](https://dl.circleci.com/status-badge/img/gh/rokoDev/buffer/tree/develop.svg?style=shield)](https://dl.circleci.com/status-badge/redirect/gh/rokoDev/buffer/tree/develop)|

# buffer

## Common prerequisites
  * Installed CMake 3.15.7 or newer.
  * Installed Git.

## MacOS

  1. Install Xcode.
  2. `git clone https://github.com/rokoDev/buffer.git`
  3. `cd buffer`
  4. Generate Xcode project: `cmake -GXcode -DCMAKE_TOOLCHAIN_FILE=deps_content/cmake-toolchains/clang.cmake -B $(pwd)/build/Xcode`
  5. `cd build/Xcode`
  6. Build unit tests(if you need you can replace `Debug` with `Rlease`): `cmake --build . --config Debug`
  7. Run unit tests(you must use `Release` instead of `Debug` if you did so in previous step): `ctest -I ,,1 -C Debug --output-on-failure`
  8. If you need to open Xcode project run: `cmake --open .`

## Windows

  1. Install `Visual Studio 2019`
  2. `git clone https://github.com/rokoDev/buffer.git`
  3. `cd buffer`
  4. `cmake -E make_directory build`
  5. `cd build`
  6. Generate Visual Studio project: `cmake -G "Visual Studio 16 2019" -DCMAKE_TOOLCHAIN_FILE=deps_content/cmake-toolchains/clang-cl.cmake ..`
  7. Build unit tests(if you need you can replace `Debug` with `Rlease`): `cmake --build . --config Debug`
  8. Run unit tests(you must use `Release` instead of `Debug` if you did so in previous step): `ctest -I ,,1 -C Debug --output-on-failure`
  9. If you need to open Visual Studio project run: `cmake --open .`

## Linux
  It is desirable to have installed one of the latest versions of the `clang` or `gcc` compilers.
  `Note: If you use clang compiler you need to set environment variables like so:
         PATH=/path/to/clang/bin:/path/to/cmake-install/bin:$PATH
         CC=/path/to/clang/bin/clang
         CXX=/path/to/clang/bin/clang++
         LD_LIBRARY_PATH=/path/to/clang/lib:$LD_LIBRARY_PATH`

  Then follow this instructions:
  1. Install `Ninja`: `apt-get install -y ninja-build`
  2. `git clone https://github.com/rokoDev/buffer.git`
  3. `cd buffer`
  4. Generate Ninja project: `cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=deps_content/cmake-toolchains/gnu.cmake -DCMAKE_BUILD_TYPE=Debug -S . -B ./build/Ninja/gnu/Debug`(or run `cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=deps_content/cmake-toolchains/clang.cmake -DCMAKE_BUILD_TYPE=Debug -S . -B ./build/Ninja/clang/Debug` if you use clang compiler)
  5. `cd build/Ninja/gnu/Debug`(or run `cd build/Ninja/clang/Debug` if you use clang compiler)
  6. Build unit tests: `cmake --build . --config Debug`
  7. Run unit tests: `ctest -I ,,1 -C Debug --output-on-failure`

## Commits with proper formatting, json validation and other checks

  Install `pre-commit` package. For instructions see: [pre-commit installation](https://pre-commit.com/#install)

  In order to be able to make commits with proper formatting and other checks you should run this commands after clonning the repo:
  1. `cd buffer`
  2. `pre-commit install`
  3. `pre-commit install --hook-type prepare-commit-msg`
