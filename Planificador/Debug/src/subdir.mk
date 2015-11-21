################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/configuracionPlanificador.c \
../src/consola.c \
../src/planificador.c \
../src/servidor.c 

OBJS += \
./src/configuracionPlanificador.o \
./src/consola.o \
./src/planificador.o \
./src/servidor.o 

C_DEPS += \
./src/configuracionPlanificador.d \
./src/consola.d \
./src/planificador.d \
./src/servidor.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


