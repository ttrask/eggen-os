################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../proj1.o 

C_SRCS += \
../forkManyProcs.c \
../proj1.c \
../semdemo.c \
../seminit.c 

OBJS += \
./forkManyProcs.o \
./proj1.o \
./semdemo.o \
./seminit.o 

C_DEPS += \
./forkManyProcs.d \
./proj1.d \
./semdemo.d \
./seminit.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


