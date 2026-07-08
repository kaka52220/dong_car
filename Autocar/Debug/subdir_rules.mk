################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"G:/MSPM0 3507/CCS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"G:/dong_car_code/Autocar/BSP" -I"G:/dong_car_code/Autocar" -I"G:/dong_car_code/Autocar/Debug" -I"G:/MSPM0 3507/CCS/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"G:/MSPM0 3507/CCS/mspm0_sdk_2_10_00_04/source" -I"G:/dong_car_code/Autocar/OLED/csrc" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1453996038: ../empty.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"G:/MSPM0 3507/CCS/sysconfig_1.26.2/sysconfig_cli.bat" -s "G:/MSPM0 3507/CCS/mspm0_sdk_2_10_00_04/.metadata/product.json" --script "G:/dong_car_code/Autocar/empty.syscfg" -o "." --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-1453996038 ../empty.syscfg
device.opt: build-1453996038
device.cmd.genlibs: build-1453996038
ti_msp_dl_config.c: build-1453996038
ti_msp_dl_config.h: build-1453996038
Event.dot: build-1453996038

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"G:/MSPM0 3507/CCS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"G:/dong_car_code/Autocar/BSP" -I"G:/dong_car_code/Autocar" -I"G:/dong_car_code/Autocar/Debug" -I"G:/MSPM0 3507/CCS/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"G:/MSPM0 3507/CCS/mspm0_sdk_2_10_00_04/source" -I"G:/dong_car_code/Autocar/OLED/csrc" -gdwarf-3 -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: G:/MSPM0\ 3507/CCS/mspm0_sdk_2_10_00_04/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"G:/MSPM0 3507/CCS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"G:/dong_car_code/Autocar/BSP" -I"G:/dong_car_code/Autocar" -I"G:/dong_car_code/Autocar/Debug" -I"G:/MSPM0 3507/CCS/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"G:/MSPM0 3507/CCS/mspm0_sdk_2_10_00_04/source" -I"G:/dong_car_code/Autocar/OLED/csrc" -gdwarf-3 -Wall -MMD -MP -MF"startup_mspm0g350x_ticlang.d_raw" -MT"startup_mspm0g350x_ticlang.o"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


