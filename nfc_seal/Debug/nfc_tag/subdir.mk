################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../nfc_tag/ili9341_cube.c \
../nfc_tag/st25dv.c \
../nfc_tag/st25dv_reg.c \
../nfc_tag/st25ftm_common.c \
../nfc_tag/st25ftm_config.c \
../nfc_tag/st25ftm_protocol.c \
../nfc_tag/st25ftm_rx.c \
../nfc_tag/st25ftm_tx.c 

OBJS += \
./nfc_tag/ili9341_cube.o \
./nfc_tag/st25dv.o \
./nfc_tag/st25dv_reg.o \
./nfc_tag/st25ftm_common.o \
./nfc_tag/st25ftm_config.o \
./nfc_tag/st25ftm_protocol.o \
./nfc_tag/st25ftm_rx.o \
./nfc_tag/st25ftm_tx.o 

C_DEPS += \
./nfc_tag/ili9341_cube.d \
./nfc_tag/st25dv.d \
./nfc_tag/st25dv_reg.d \
./nfc_tag/st25ftm_common.d \
./nfc_tag/st25ftm_config.d \
./nfc_tag/st25ftm_protocol.d \
./nfc_tag/st25ftm_rx.d \
./nfc_tag/st25ftm_tx.d 


# Each subdirectory must supply rules for building sources it contributes
nfc_tag/%.o: ../nfc_tag/%.c nfc_tag/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F746xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"/home/george/STM32CubeIDE/workspace_1.8.0/nfc_seal/seal" -I"/home/george/STM32CubeIDE/workspace_1.8.0/nfc_seal/nfc_tag" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-nfc_tag

clean-nfc_tag:
	-$(RM) ./nfc_tag/ili9341_cube.d ./nfc_tag/ili9341_cube.o ./nfc_tag/st25dv.d ./nfc_tag/st25dv.o ./nfc_tag/st25dv_reg.d ./nfc_tag/st25dv_reg.o ./nfc_tag/st25ftm_common.d ./nfc_tag/st25ftm_common.o ./nfc_tag/st25ftm_config.d ./nfc_tag/st25ftm_config.o ./nfc_tag/st25ftm_protocol.d ./nfc_tag/st25ftm_protocol.o ./nfc_tag/st25ftm_rx.d ./nfc_tag/st25ftm_rx.o ./nfc_tag/st25ftm_tx.d ./nfc_tag/st25ftm_tx.o

.PHONY: clean-nfc_tag

