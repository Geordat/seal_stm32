################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../seal/ckks_asym.c \
../seal/ckks_common.c \
../seal/ckks_sym.c \
../seal/fft.c \
../seal/fileops.c \
../seal/fips202.c \
../seal/intt.c \
../seal/keccakf1600.c \
../seal/modulus.c \
../seal/network.c \
../seal/ntt.c \
../seal/parameters.c \
../seal/polymodmult.c \
../seal/rng.c \
../seal/sample.c \
../seal/seal_embedded.c \
../seal/timer.c \
../seal/uint_arith.c 

OBJS += \
./seal/ckks_asym.o \
./seal/ckks_common.o \
./seal/ckks_sym.o \
./seal/fft.o \
./seal/fileops.o \
./seal/fips202.o \
./seal/intt.o \
./seal/keccakf1600.o \
./seal/modulus.o \
./seal/network.o \
./seal/ntt.o \
./seal/parameters.o \
./seal/polymodmult.o \
./seal/rng.o \
./seal/sample.o \
./seal/seal_embedded.o \
./seal/timer.o \
./seal/uint_arith.o 

C_DEPS += \
./seal/ckks_asym.d \
./seal/ckks_common.d \
./seal/ckks_sym.d \
./seal/fft.d \
./seal/fileops.d \
./seal/fips202.d \
./seal/intt.d \
./seal/keccakf1600.d \
./seal/modulus.d \
./seal/network.d \
./seal/ntt.d \
./seal/parameters.d \
./seal/polymodmult.d \
./seal/rng.d \
./seal/sample.d \
./seal/seal_embedded.d \
./seal/timer.d \
./seal/uint_arith.d 


# Each subdirectory must supply rules for building sources it contributes
seal/%.o: ../seal/%.c seal/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F746xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"/home/george/STM32CubeIDE/workspace_1.8.0/nfc_seal/seal" -I"/home/george/STM32CubeIDE/workspace_1.8.0/nfc_seal/nfc_tag" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-seal

clean-seal:
	-$(RM) ./seal/ckks_asym.d ./seal/ckks_asym.o ./seal/ckks_common.d ./seal/ckks_common.o ./seal/ckks_sym.d ./seal/ckks_sym.o ./seal/fft.d ./seal/fft.o ./seal/fileops.d ./seal/fileops.o ./seal/fips202.d ./seal/fips202.o ./seal/intt.d ./seal/intt.o ./seal/keccakf1600.d ./seal/keccakf1600.o ./seal/modulus.d ./seal/modulus.o ./seal/network.d ./seal/network.o ./seal/ntt.d ./seal/ntt.o ./seal/parameters.d ./seal/parameters.o ./seal/polymodmult.d ./seal/polymodmult.o ./seal/rng.d ./seal/rng.o ./seal/sample.d ./seal/sample.o ./seal/seal_embedded.d ./seal/seal_embedded.o ./seal/timer.d ./seal/timer.o ./seal/uint_arith.d ./seal/uint_arith.o

.PHONY: clean-seal

