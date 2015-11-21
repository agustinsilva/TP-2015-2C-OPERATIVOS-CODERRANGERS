################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/administradorSwap.c \
../src/conexion.c \
../src/configuracionSwap.c \
../src/particion.c 

OBJS += \
./src/administradorSwap.o \
./src/conexion.o \
./src/configuracionSwap.o \
./src/particion.o 

C_DEPS += \
./src/administradorSwap.d \
./src/conexion.d \
./src/configuracionSwap.d \
./src/particion.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


