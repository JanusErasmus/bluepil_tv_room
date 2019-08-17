# Bluepill with Vitrual COM port

Firmware for the STM32F103 Bluepill



## Development environment
To start using this project install the following:
  - [GNU Arm Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) compiler

### Eclipse
Import as Makefile project

To get the indexer to work better, simply change the Compiler Spec Provider to **arm-none-eabi-gcc**  
 - Replace the Command to get compiler specs:  
 ( Project Properties > C/C++ General > Preprocessor Include Paths, Macro etc.)
```
arm-none-eabi-gcc ${FLAGS} ${cross_toolchain_flags}  -E -P -v -dD "${INPUTS}"
```
 - Delete all the errors in the Problems view
 - Exclude mbed-os/targets from the build
 - Navigate to targets/TARGET_STM/TARGET_STM32L1/TARGET_KAMSTRUP_WB and include to the build
 - Close the project
 - Open the project
 - Rebuild the index with freshen all files.  
 (Project -> Index -> Freshen All Files)