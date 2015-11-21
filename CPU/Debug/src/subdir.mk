################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Conexion.c \
../src/GestorTiempos.c \
../src/configuracionCpu.c \
../src/cpu.c 

OBJS += \
./src/Conexion.o \
./src/GestorTiempos.o \
./src/configuracionCpu.o \
./src/cpu.o 

C_DEPS += \
./src/Conexion.d \
./src/GestorTiempos.d \
./src/configuracionCpu.d \
./src/cpu.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


