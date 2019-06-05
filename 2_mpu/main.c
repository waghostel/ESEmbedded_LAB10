#include <stdint.h>
#include <stdio.h>
#include "reg.h"
#include "blink.h"
#include "usart.h"
#include "asm_func.h"

void user_task(void);
void set_mpu(void);

int main(void)
{
	extern uint32_t _msp_init;

	uint32_t *msp_init = &_msp_init;
	uint32_t *psp_init = msp_init - 8 * 1024; //

	init_usart1();

	printf("[Kernel] Start in privileged thread mode (msp_init = 0x%x).\r\n\n", (unsigned int)msp_init);

	printf("[Kernel] Control: 0x%x \r\n", (unsigned int)read_ctrl());
	printf("[Kernel] SP: 0x%x \r\n", (unsigned int)read_sp());
	printf("[Kernel] MSP: 0x%x \r\n", (unsigned int)read_msp());
	printf("[Kernel] PSP: 0x%x \r\n\n", (unsigned int)read_psp());

	set_mpu();

	printf("[Kernel] Switch to unprivileged thread mode & start user task (psp_init = 0x%x).\r\n\n", (unsigned int)psp_init);

	//start user task
	start_user((uint32_t *)user_task,psp_init);

	while (1) //should not go here
	blink(LED_ORANGE);
		;
}

void user_task(void)
{
	printf("[User] Try to print something.\r\n\n");
	blink(LED_BLUE); //should not return
}

void set_mpu(void)
{
	printf("Begin set_mpu() region\n");
//set region 0: flash (0x00000000), 1MB, allow execution, full access, enable all subregion
	
	//Open and define the starting address of region 0
	REG(MPU_BASE + MPU_RBAR_OFFSET)=MPU_RBAR_VALUE(0x00000000,MPU_REGION_0); 

	//MPU_RASR_VALUE(xn, ap, type, srd, size)
	REG(MPU_BASE + MPU_RASR_OFFSET) = MPU_RASR_VALUE(MPU_XN_ENABLE,MPU_AP_FULL_ACCESS,MPU_TYPE_FLASH,MPU_SRD_ENABLE,MPU_REGION_SIZE_1MB);
	

//set region 1: sram (0x20000000), 128KB, forbid execution, full access, enable all subregion
	REG(MPU_BASE + MPU_RBAR_OFFSET) = MPU_RBAR_VALUE(0x20000000,MPU_REGION_1);
	REG(MPU_BASE + MPU_RASR_OFFSET) = MPU_RASR_VALUE(MPU_XN_DISABLE, MPU_AP_FULL_ACCESS, MPU_TYPE_SRAM, MPU_SRD_ENABLE, MPU_REGION_SIZE_128KB);

//set region 2: RCC_AHB1ENR, 32B, forbid execution, full access, enable all subregion
	REG(MPU_BASE + MPU_RBAR_OFFSET) = MPU_RBAR_VALUE(RCC_AHB1ENR_OFFSET,MPU_REGION_2);
	REG(MPU_BASE + MPU_RASR_OFFSET) = MPU_RASR_VALUE(MPU_XN_DISABLE, MPU_AP_FULL_ACCESS, MPU_TYPE_FLASH, MPU_SRD_ENABLE, MPU_REGION_SIZE_32B);
	//最小可用寬度check
	
//set region 3: GPIOD, 32B, forbid execution, full access, enable all subregion
	REG(MPU_BASE + MPU_RBAR_OFFSET) = MPU_RBAR_VALUE(GPIO_BASE(GPIO_PORTD),MPU_REGION_3);
	REG(MPU_BASE + MPU_RASR_OFFSET) = MPU_RASR_VALUE(MPU_XN_DISABLE, MPU_AP_FULL_ACCESS, MPU_TYPE_PERIPHERALS,MPU_SRD_ENABLE, MPU_REGION_SIZE_32B);
	
	
	//disable region 4 ~ 7
	for (int i=4;i<8;i++){
		REG(MPU_BASE + MPU_RBAR_OFFSET) = MPU_RBAR_VALUE(0,i);
		REG(MPU_BASE + MPU_RASR_OFFSET)= 0;
	}

	//enable the default memory map as a background region for privileged access (PRIVDEFENA)
	SET_BIT(MPU_BASE + MPU_CTRL_OFFSET, MPU_PRIVDEFENA_BIT);

	//enable mpu
	SET_BIT(MPU_BASE + MPU_CTRL_OFFSET, MPU_ENABLE_BIT);
	printf("End set_mpu() region\n");
}
