################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: kernel

dependents:
	-cd /home/utnso/workspace/tp-2017-1c-Stranger-Code/shared-library/Debug && $(MAKE) all
	-cd /home/utnso/workspace/so-commons-library/src/Debug && $(MAKE) all

# Tool invocations
kernel: $(OBJS) $(USER_OBJS) /home/utnso/workspace/tp-2017-1c-Stranger-Code/shared-library/Debug/libshared-library.so /home/utnso/workspace/so-commons-library/src/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/so-commons-library/src/Debug" -L"/home/utnso/workspace/tp-2017-1c-Stranger-Code/shared-library/Debug" -o "kernel" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) kernel
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/workspace/tp-2017-1c-Stranger-Code/shared-library/Debug/libshared-library.so:
/home/utnso/workspace/so-commons-library/src/Debug/libcommons.so:

-include ../makefile.targets
