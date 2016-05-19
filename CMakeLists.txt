cmake_minimum_required(VERSION 3.5)
project(vtc-VaporWare)

set(ARMGCC "/usr/local/Cellar/gcc-arm-none-eabi/20151219")
set(CMAKE_ASM_COMPILER "${ARMGCC}/bin/arm-none-eabi-as")
set(CMAKE_C_COMPILER "${ARMGCC}/bin/arm-none-eabi-gcc")
set(CMAKE_AR "${ARMGCC}/bin/arm-none-eabi-ar")
set(CMAKE_OBJCOPY "${ARMGCC}/bin/arm-none-eabi-objcopy")
set(CMAKE_LINKER ${ARMGCC}/bin/arm-none-eabi-ld)

set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_RELEASE "")
set(CMAKE_C_FLAGS_MINSIZEREL "")
set(CMAKE_ASM_FLAGS_DEBUG "")
set(CMAKE_ASM_FLAGS_MINSIZEREL "")
set(CMAKE_ASM_FLAGS_RELEASE "")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I$ENV{EVICSDK}/include/ -I$ENV{EVICSDK}/ -I. -Werror -c -fshort-enums -Wall -mcpu=cortex-m4 -mthumb -Os -fdata-sections -ffunction-sections")

# git tag
execute_process(
        COMMAND git describe --abbrev=4 --dirty --always --tags
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(-DGIT_VERSION="${GIT_VERSION}")

# source
include_directories(
        "$ENV{EVICSDK}/nuvoton-sdk/Library/CMSIS/Include/"
        "$ENV{EVICSDK}/nuvoton-sdk/Library/Device/Nuvoton/M451Series/Include/"
        "$ENV{EVICSDK}/nuvoton-sdk/Library/StdDriver/inc/"
        "$ENV{EVICSDK}/include"
        include
        .
)

set(SOURCE_FILES
        src/globals.c
        src/materials.c
        src/settings/settings_helpers.c
        src/settings/dataflash.c
        src/utils/button.c
        src/utils/menu.c
        src/utils/communication.c
        src/utils/variabletimer.c
        font/Font_Large.c
        font/Font_Medium.c
        font/Font_Small.c
        src/utils/helper.c
        src/display/display_helper.c
        src/display/display.c
        src/settings/settings.c
        images/temperature.c
        src/modes/mode.c
        src/modes/mode_watt.c
        src/modes/mode_volt.c
        src/modes/mode_temp.c
        src/main.c
        )

# build
set(TARGET atomizer)
add_executable(${TARGET} ${SOURCE_FILES})
target_link_libraries(${TARGET} evicsdk)
add_dependencies(${TARGET} evicsdk)

# create output bin dir
add_custom_command(
        TARGET atomizer PRE_LINK
        COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/bin
        COMMENT "creating bin dir"
        VERBATIM
)

# link
set(LIBDIRS "-L${ARMGCC}/arm-none-eabi/lib/ -L${ARMGCC}/lib/gcc/arm-none-eabi/5.2.1/ -L$ENV{EVICSDK}/lib/")
set(LDSCRIPT $ENV{EVICSDK}/linker/linker.ld)
set(LINKFLAGS "${LIBDIRS} -nostdlib  -nostartfiles -T${LDSCRIPT} -gc-sections")
set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_LINKER} <OBJECTS> ${LINKFLAGS} -o ${PROJECT_SOURCE_DIR}/bin/atomizer.elf")

# convert and encrypt to a flashable binary
add_custom_command(
        TARGET atomizer POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary -j .text -j .data ${PROJECT_SOURCE_DIR}/bin/atomizer.elf ${PROJECT_SOURCE_DIR}/bin/atomizer_unencrypted.bin
        COMMAND evic convert ${PROJECT_SOURCE_DIR}/bin/atomizer_unencrypted.bin -o ${PROJECT_SOURCE_DIR}/bin/atomizer.bin
        COMMENT "converting"
        VERBATIM
)