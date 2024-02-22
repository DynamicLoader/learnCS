
# SimpleCompiler

## Introduction

## Build

Please make sure your system meets the following requirements:

- OS: Linux (Ubuntu >= 20.04 is recommended)
- Mandatory Dependencies:
    ```
        flex >= 2.6.4
        bison >= 3.5
        g++ >= 9 (Currently only g++ is supported)
        binutils
        GNU make (Ninja has been tested and does not work)
        cmake
    ```
- Optional Dependencies:
    ```
        libbz2-dev > 1.0
        asmjit
        cereal
    ```

### Initialize the Project

Make sure you clone from Git and initialize the submodules if you wish to build bzip2 or asmjit.

You can initialize submodules by executing the following command:
```bash
git submodule update --init 
```

**Note: This repo is NOT the original repo, so the submodule could not be initialized normally. All submodules are recorded in file `.gitsubmodule`, with the following version:**
```
 asmjit@1da9e4d957
 bzip2@9de658d248
 cereal@d1fcec807b
```

### Build Options

| Name        | Default | Description                                                                 |
|-------------|---------|-----------------------------------------------------------------------------|
| BUILD_BZIP2 | False   | Build Bzip2 from source. If your compile environment is equipped with libbz2-dev, the bzip2 library would be found by CMake |
| ENABLE_INTPR | False   | Enable Interpreter mode. Enabling this requires ASMJIT and cereal to be built, currently only supports x86-64 |

### Native Build

Use CMake to configure the project and run the build.

### Cross Build for Windows

Due to the convenience of developing compilers under Linux, the main platform is Linux. However, it supports cross-compiling to create executables for Windows.

To do this, you need to install target-specific C++ toolchains with at least C++17 support. Then specify the toolchain in CMake to generate a Makefile and build.

**Building BZip2 from source will help in this case unless your cross-compile toolchain already has the library!**

## License

Open source under GPL-3.
