Bootloader - small piece of code stored in MCU flash or ROM to act as an application loader as well as a mechanism to update the application whenever required

Memory organization (STM32F446XX example):
- internal Flash (also called as Embedded Flash) (512KB) - begins @0x0800_0000, ends @0x0807_FFFF, stores application code and read only data, non volatile
- internal SRAM1 (112KB) - begins @0x2000_0000, ends @0x2001_BFFF, stores apllication global data and static variables, stack and heap, volatile, can store code to execute
- internal SRAM2 (16KB) - begins @0x2001_C000, ends @0x2001_FFFF
- System Memory (ROM) (30KB) - begins @0x1FFF_0000, ends @0x1FFF_77FF, read only memory, stores ST Bootloader
- OTP memory (528 bytes) - begins @0x1FFF_7800, ends @0x1FFF_7A0F
- option bytes (16 bytes) - begins @0x1FFF_C000, ends @0x1FFF_C00F
- backup RAM (4KB)

Flash memory is divided into sectors of different size (Flash memory can be erased only by full sector).

Reset sequence of ARM Cortex-M microcontroller:
- the PC of the processor is loaded with the value 0x0000_0000
- the processor reads the value @0x0000_0000 into MSP (Main Stack Pointer register) - stack initialization
- the processor reads the value @0x0000_0004 into PC - address of the Reset Handler
- PC jumps to the Reset Handler

In STM32 microcontroller:
- MSP value is stored in Flash @0x0800_0000
- address of the Reset Handler is stored @0x0800_0004

Memory aliasing - allows to map memory to 0x0000_0000 (it can be Flash memory, RAM or System Memory), aliased memory region depends on boot mode that is selected by boot pins