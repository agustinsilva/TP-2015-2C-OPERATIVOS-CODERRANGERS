################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/administradorMemoria.c \
../src/atencionPedidosCPU.c \
../src/busquedas.c \
../src/configuracionMemoria.c \
../src/gestionMemoria.c \
../src/signals.c 

OBJS += \
./src/administradorMemoria.o \
./src/atencionPedidosCPU.o \
./src/busquedas.o \
./src/configuracionMemoria.o \
./src/gestionMemoria.o \
./src/signals.o 

C_DEPS += \
./src/administradorMemoria.d \
./src/atencionPedidosCPU.d \
./src/busquedas.d \
./src/configuracionMemoria.d \
./src/gestionMemoria.d \
./src/signals.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


