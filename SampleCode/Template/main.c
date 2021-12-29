/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include "project_config.h"

#include "diskio.h"
#include "ff.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/
volatile uint32_t BitFlag = 0;
volatile uint32_t counter_tick = 0;

#define DRIVE_NUMBER    0       /* Physical drive number */
#define DRIVE_NAME      "0:"    /* The drive name string for drive number 0 */
FATFS g_FatFs[_DRIVES];     /* File system object for logical drive */
char  g_buff[512];          /* Buffer for read / write data */

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

void tick_counter(void)
{
	counter_tick++;
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void compare_buffer(uint8_t *src, uint8_t *des, int nBytes)
{
    uint16_t i = 0;	
	
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			set_flag(flag_error , ENABLE);
        }
    }

	if (!is_flag_set(flag_error))
	{
    	printf("%s finish \r\n" , __FUNCTION__);	
		set_flag(flag_error , DISABLE);
	}

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    uint8_t *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    uint8_t *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(uint8_t *pucBuff, int nBytes)
{
    uint16_t i = 0;
    
    printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }            
    }
    printf("\r\n\r\n");
}

void  dump_buffer_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        printf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            printf("%02X ", pucBuff[nIdx + i]);
        printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                printf("%c", pucBuff[nIdx + i]);
            else
                printf(".");
            nBytes--;
        }
        nIdx += 16;
        printf("\n");
    }
    printf("\n");
}

void put_rc(FRESULT rc)
{
    uint32_t i;
    const TCHAR *p =
        _T("OK\0DISK_ERR\0INT_ERR\0NOT_READY\0NO_FILE\0NO_PATH\0INVALID_NAME\0")
        _T("DENIED\0EXIST\0INVALID_OBJECT\0WRITE_PROTECTED\0INVALID_DRIVE\0")
        _T("NOT_ENABLED\0NO_FILE_SYSTEM\0MKFS_ABORTED\0TIMEOUT\0LOCKED\0")
        _T("NOT_ENOUGH_CORE\0TOO_MANY_OPEN_FILES\0");

    for (i = 0; (i != (UINT)rc) && *p; i++)
    {
        while (*p++) ;
    }

    printf(_T("rc=%u FR_%s\n"), (UINT)rc, p);
}

uint8_t Explore_Disk (char* path , uint8_t recu_level)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	char *fn;
	char tmp[30];
	
	res = f_opendir(&dir, path);
	
	if (res == FR_OK) 
	{
		printf("%s\n\r",path);
		while(1) 
		{
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0) 
				break;
			fn = fno.fname;
			strcpy(tmp, fn); 
			if(recu_level == 1)
			{
				printf(" |__%s\n\r",fno.fname);
			}
			else if(recu_level == 2)
			{
				printf("     |__%s\n\r",fno.fname);
			}
			else if(recu_level == 3)
			{
				printf("         |__%s\n\r",fno.fname);
			}
			else if(recu_level == 4)
			{
				printf("             |__%s\n\r",fno.fname);
			}
			else if(recu_level == 5)
			{
				printf("                 |__%s\n\r",fno.fname);
			}
			
			if((fno.fattrib & AM_MASK) == AM_DIR)
			{
				sprintf(tmp,"%s/%s",path,fno.fname);
			}

			if(((fno.fattrib & AM_MASK) == AM_DIR)&&(recu_level == 1))
			{
				Explore_Disk(tmp, 2);
			}
			if(((fno.fattrib & AM_MASK) == AM_DIR)&&(recu_level == 2))
			{
				Explore_Disk(tmp, 3);
			}
			if(((fno.fattrib & AM_MASK) == AM_DIR)&&(recu_level == 3))
			{
				Explore_Disk(tmp, 3);
			}
			if(((fno.fattrib & AM_MASK) == AM_DIR)&&(recu_level == 4))
			{
				Explore_Disk(tmp, 3);
			}
			if(((fno.fattrib & AM_MASK) == AM_DIR)&&(recu_level == 5))
			{
				Explore_Disk(tmp, 3);
			}


		}
	}
	return res;
}


void SD_FATFS_Init(void)
{
    FRESULT res;
    
	/* Initializes the physical disk drive */
	 res = (FRESULT)disk_initialize(DRIVE_NUMBER);
	
	 if (res != STA_OK)
	 {
		 put_rc(res);
		 printf("\n\nInitialize SD card fail.\n");
	 }
	
	 /* Registers a work area to the FatFs module */
	 res = f_mount(DRIVE_NUMBER, &g_FatFs[0]);
	
	 if (res != FR_OK)
	 {
		 put_rc(res);
		 printf("\n\nMount file system fail.\n");
	 }
	
	printf("%s finish\r\n",__FUNCTION__);

}

void SD_FATFS_Demo(void)
{
	uint32_t freeCluster;
	FATFS *fs; 	 /* Pointer to file system object */
	DIR dir;		 /* Directory object */
	FRESULT res;
	char  *ptr2 = NULL;

	FIL file;
	UINT count;


	/* Opens an exsisting directory and creates the directory object for subsequent calls */
	put_rc(f_opendir(&dir, DRIVE_NAME));
		
	if (f_getfree(ptr2, (DWORD *)&freeCluster, &fs) == FR_OK)
	 printf(",	%10u bytes free\n", freeCluster * fs->csize * 512);
	
	/* File read / write case */
	/* Open file and set it to readable / writable mode */
	res = f_open(&file, "test.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
	
	 if (res == FR_OK)
	 {
		 /* Move file pointer to end of file (offset = file.fsize) */
		 f_lseek(&file, file.fsize);
		 /* Write something to file pointer */
		 f_write(&file, "hello ", 6, &count);
		 /* Move file pointer to begin of file (offset = 0) */
		 f_lseek(&file, 0);
		 /* Read something from file pointer */
		 f_read(&file, g_buff, file.fsize, &count);
		 printf("Read data from file test.txt : \n[%s]\n", g_buff);
		 /* Close file */
		 f_close(&file);
	 }
	 else
		 printf("Open file fail. Return %d\n", res);    


	printf("%s finish\r\n",__FUNCTION__);

}


void TMR1_IRQHandler(void)
{
//	static uint32_t LOG = 0;

	
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        TIMER_ClearIntFlag(TIMER1);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
//        	printf("%s : %4d\r\n",__FUNCTION__,LOG++);
			PH0 ^= 1;
		}

		if ((get_tick() % 50) == 0)
		{

		}	
    }
}


void TIMER1_Init(void)
{
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);	
    TIMER_Start(TIMER1);
}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART0);

	if (res == 'x' || res == 'X')
	{
		NVIC_SystemReset();
	}

	if (res > 0x7F)
	{
		printf("invalid command\r\n");
	}
	else
	{
		switch(res)
		{
			case '1':
				break;

			case 'X':
			case 'x':
			case 'Z':
			case 'z':
				NVIC_SystemReset();		
				break;
		}
	}
}


void UART0_IRQHandler(void)
{
    if(UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAINT_Msk | UART_INTSTS_RXTOINT_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART0) == 0)
        {
			UARTx_Process();
        }
    }

    if(UART0->FIFOSTS & (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk))
    {
        UART_ClearIntFlag(UART0, (UART_INTSTS_RLSINT_Msk| UART_INTSTS_BUFERRINT_Msk));
    }	
}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);

	/* Set UART receive time-out */
	UART_SetTimeoutCnt(UART0, 20);

	UART0->FIFO &= ~UART_FIFO_RFITL_4BYTES;
	UART0->FIFO |= UART_FIFO_RFITL_8BYTES;

	/* Enable UART Interrupt - */
	UART_ENABLE_INT(UART0, UART_INTEN_RDAIEN_Msk | UART_INTEN_TOCNTEN_Msk | UART_INTEN_RXTOIEN_Msk);
	
	NVIC_EnableIRQ(UART0_IRQn);

	#if (_debug_log_UART_ == 1)	//debug
	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetPCLK0Freq());
	printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());	

//    printf("Product ID 0x%8X\n", SYS->PDID);
	
	#endif
}

void Custom_Init(void)
{
	SYS->GPH_MFPL = (SYS->GPH_MFPL & ~(SYS_GPH_MFPL_PH0MFP_Msk)) | (SYS_GPH_MFPL_PH0MFP_GPIO);
	SYS->GPH_MFPL = (SYS->GPH_MFPL & ~(SYS_GPH_MFPL_PH1MFP_Msk)) | (SYS_GPH_MFPL_PH1MFP_GPIO);
	SYS->GPH_MFPL = (SYS->GPH_MFPL & ~(SYS_GPH_MFPL_PH2MFP_Msk)) | (SYS_GPH_MFPL_PH2MFP_GPIO);

	//EVM LED
	GPIO_SetMode(PH,BIT0,GPIO_MODE_OUTPUT);
	GPIO_SetMode(PH,BIT1,GPIO_MODE_OUTPUT);
	GPIO_SetMode(PH,BIT2,GPIO_MODE_OUTPUT);
	
}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);

    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(FREQ_192MHZ);
    /* Set PCLK0/PCLK1 to HCLK/2 */
    CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV2 | CLK_PCLKDIV_APB1DIV_DIV2);

    /* Enable UART clock */
    CLK_EnableModuleClock(UART0_MODULE);
    /* Select UART clock source from HXT */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk);
    SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);

    CLK_EnableModuleClock(TMR1_MODULE);
    CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HIRC, 0);
	
    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}

/*
 * This is a template project for M480 series MCU. Users could based on this project to create their
 * own application without worry about the IAR/Keil project settings.
 *
 * This template application uses external crystal as HCLK source and configures UART0 to print out
 * "Hello World", users may need to do extra system configuration based on their system design.
 */

int main()
{
    SYS_Init();

	UART0_Init();
	Custom_Init();	
	TIMER1_Init();

	SD_FATFS_Init();
	SD_FATFS_Demo();

    /* Got no where to go, just loop forever */
    while(1)
    {


    }
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
