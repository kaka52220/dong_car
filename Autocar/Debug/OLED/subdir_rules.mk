################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
OLED/%.o: ../OLED/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"G:/MSPM0 3507/CCS/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"G:/dong_car_code/Autocar/BSP" -I"G:/dong_car_code/Autocar" -I"G:/dong_car_code/Autocar/Debug" -I"G:/MSPM0 3507/CCS/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"G:/MSPM0 3507/CCS/mspm0_sdk_2_10_00_04/source" -I"G:/dong_car_code/Autocar/OLED" -I"G:/dong_car_code/Autocar/APPLICATION" -I"G:/dong_car_code/Autocar/OLED/csrc" -gdwarf-3 -Wall -MMD -MP -MF"OLED/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


