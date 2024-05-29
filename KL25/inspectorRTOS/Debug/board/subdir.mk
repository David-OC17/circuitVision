################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/LCD_8BIT.c \
../board/RGB.c \
../board/board.c \
../board/clock_config.c \
../board/common.c \
../board/i2cdisplay.c \
../board/keypad.c \
../board/peripherals.c \
../board/pin_mux.c \
../board/stepper.c 

C_DEPS += \
./board/LCD_8BIT.d \
./board/RGB.d \
./board/board.d \
./board/clock_config.d \
./board/common.d \
./board/i2cdisplay.d \
./board/keypad.d \
./board/peripherals.d \
./board/pin_mux.d \
./board/stepper.d 

OBJS += \
./board/LCD_8BIT.o \
./board/RGB.o \
./board/board.o \
./board/clock_config.o \
./board/common.o \
./board/i2cdisplay.o \
./board/keypad.o \
./board/peripherals.o \
./board/pin_mux.o \
./board/stepper.o 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c board/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DSDK_OS_BAREMETAL -DFSL_RTOS_BM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"/home/david/Documents/UNI_S.4/SoC/circuitVision/KL25/inspectorRTOS/board" -I"/home/david/Documents/UNI_S.4/SoC/circuitVision/KL25/inspectorRTOS/source" -I"/home/david/Documents/UNI_S.4/SoC/circuitVision/KL25/inspectorRTOS" -I"/home/david/Documents/UNI_S.4/SoC/circuitVision/KL25/inspectorRTOS/drivers" -I"/home/david/Documents/UNI_S.4/SoC/circuitVision/KL25/inspectorRTOS/startup" -I"/home/david/Documents/UNI_S.4/SoC/circuitVision/KL25/inspectorRTOS/utilities" -I"/home/david/Documents/UNI_S.4/SoC/circuitVision/KL25/inspectorRTOS/CMSIS" -I"/home/david/Documents/UNI_S.4/SoC/circuitVision/KL25/inspectorRTOS/freertos" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-board

clean-board:
	-$(RM) ./board/LCD_8BIT.d ./board/LCD_8BIT.o ./board/RGB.d ./board/RGB.o ./board/board.d ./board/board.o ./board/clock_config.d ./board/clock_config.o ./board/common.d ./board/common.o ./board/i2cdisplay.d ./board/i2cdisplay.o ./board/keypad.d ./board/keypad.o ./board/peripherals.d ./board/peripherals.o ./board/pin_mux.d ./board/pin_mux.o ./board/stepper.d ./board/stepper.o

.PHONY: clean-board

