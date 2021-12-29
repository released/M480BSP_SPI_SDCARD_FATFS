/****************************************************************************//**
 * @file    sdcard.h
 * @brief
 *          SD Card driver header file
 * @note
 * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"
#include "sdcard.h"

/****************************************************************************/
/* Define                                                                   */
/****************************************************************************/

#define SPI_SPEED_LOW			(300000)
#define SPI_SPEED_HIGH			(2000000)	//(2000000)

#define Set_SD_CS       		SPI_SET_SS_HIGH(SPI1)   // CS state is low
#define Clr_SD_CS       		SPI_SET_SS_LOW(SPI1)    // CS state is high
#define SD_SPI_SpeedLow    	 	SPI_Open(SPI1, SPI_MASTER, SPI_MODE_0, 8, SPI_SPEED_LOW)
#define SD_SPI_SpeedHigh    	SPI_Open(SPI1, SPI_MASTER, SPI_MODE_0, 8, SPI_SPEED_HIGH)

/****************************************************************************/
/* Global variables                                                         */
/****************************************************************************/

unsigned char  SD_Type = 0;

/****************************************************************************/
/* Functions                                                                */
/****************************************************************************/

void SD_SPI_Init(void)
{
	/*
		SPI1 :
		PB2 : SS
		PB3: CLK
		PB4 : MOSI
		PB5 : MISO
	*/
    /* Unlock protected registers */
    SYS_UnlockReg();
	
    /* Setup SPI1 multi-function pins */
    SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk| SYS_GPB_MFPL_PB3MFP_Msk| SYS_GPB_MFPL_PB2MFP_Msk);	
    SYS->GPB_MFPL |= SYS_GPB_MFPL_PB4MFP_SPI1_MOSI | SYS_GPB_MFPL_PB5MFP_SPI1_MISO | SYS_GPB_MFPL_PB3MFP_SPI1_CLK | SYS_GPB_MFPL_PB2MFP_SPI1_SS;

    /* Enable SPI1 clock pin (PB3) schmitt trigger */
    PB->SMTEN |= GPIO_SMTEN_SMTEN3_Msk;

    /* Enable SPI1 I/O high slew rate */
    GPIO_SetSlewCtl(PB, 0xF, GPIO_SLEWCTL_HIGH);

    /* Enable SPI1 peripheral clock */
    CLK_EnableModuleClock(SPI1_MODULE);

    /* Select PCLK1 as the clock source of SPI1 */
    CLK_SetModuleClock(SPI1_MODULE, CLK_CLKSEL2_SPI1SEL_HIRC, MODULE_NoMsk);

    /* Lock protected registers */
    SYS_LockReg(); 

    /* Configure SPI1 as a master, SPI clock rate 2 MHz,
       clock idle low, 32-bit transaction, drive output on falling clock edge and latch input on rising edge. */
    SPI_Open(SPI1, SPI_MASTER, SPI_MODE_0, 8, SPI_SPEED_LOW);

    /* Disable the automatic hardware slave select function. Select the SS pin and configure as low-active. */
    SPI_DisableAutoSS(SPI1);
    Set_SD_CS;

	printf("%s\r\n",__FUNCTION__);
    
}

unsigned char SD_SPI_ReadWriteByte(unsigned char u32Data)
{
    SPI_WRITE_TX(SPI1, u32Data);

    while (SPI_IS_BUSY(SPI1));

    return SPI_READ_RX(SPI1);
}

void SD_DisSelect(void)
{
    Set_SD_CS;
    SD_SPI_ReadWriteByte(0xff);
}

unsigned char SD_Select(void)
{
    Clr_SD_CS;

    if (SD_WaitReady() == 0)
        return 0;

    SD_DisSelect();
    return 1;
}

unsigned char SD_WaitReady(void)
{
    unsigned int t = 0;

    do
    {
        if (SD_SPI_ReadWriteByte(0xFF) == 0xFF)
            return 0;   // OK

        t++;
    } while (t < 0xFFFFFF);

    return 1;   // Fail
}

unsigned char SD_GetResponse(unsigned char Response)
{
    unsigned int Count = 0xFFFF;

    while ((SD_SPI_ReadWriteByte(0xFF) != Response) && Count)
        Count--;

    if (Count == 0)
        return MSD_RESPONSE_FAILURE;
    else
        return MSD_RESPONSE_NO_ERROR;
}

unsigned char SD_RecvData(unsigned char *buf, unsigned int len)
{
    if (SD_GetResponse(0xFE))
        return 1;

    while (len--)
    {
        *buf = SD_SPI_ReadWriteByte(0xFF);
        buf++;
    }

    SD_SPI_ReadWriteByte(0xFF);
    SD_SPI_ReadWriteByte(0xFF);
    return 0;
}

unsigned char SD_SendBlock(unsigned char *buf, unsigned char cmd)
{
    unsigned int t;

    if (SD_WaitReady())
        return 1;

    SD_SPI_ReadWriteByte(cmd);

    if (cmd != 0xFD)
    {
        for (t = 0; t < 512; t++)
            SD_SPI_ReadWriteByte(buf[t]);

        SD_SPI_ReadWriteByte(0xFF);
        SD_SPI_ReadWriteByte(0xFF);
        t = SD_SPI_ReadWriteByte(0xFF);

        if ((t & 0x1F) != 0x05)
            return 2;
    }

    return 0;
}

unsigned char SD_SendCmd(unsigned char cmd, unsigned int arg, unsigned char crc)
{
    unsigned char r1;
    unsigned int Retry = 0;

    SD_DisSelect();

    if (SD_Select())
        return 0xFF;

    SD_SPI_ReadWriteByte(cmd | 0x40);
    SD_SPI_ReadWriteByte(arg >> 24);
    SD_SPI_ReadWriteByte(arg >> 16);
    SD_SPI_ReadWriteByte(arg >> 8);
    SD_SPI_ReadWriteByte(arg);
    SD_SPI_ReadWriteByte(crc);

    if (cmd == CMD12)
        SD_SPI_ReadWriteByte(0xff); // Skip a stuff byte when stop reading

    Retry = 0x1F;

    do
    {
        r1 = SD_SPI_ReadWriteByte(0xFF);
    } while ((r1 & 0x80) && Retry--);

    return r1;
}

unsigned char SD_GetCID(unsigned char *cid_data)
{
    unsigned char r1;

    r1 = SD_SendCmd(CMD10, 0, 0x01);

    if (r1 == 0x00)
    {
        r1 = SD_RecvData(cid_data, 16);
    }

    SD_DisSelect();

    if (r1)
        return 1;
    else
        return 0;
}

unsigned char SD_GetCSD(unsigned char *csd_data)
{
    unsigned char r1;
    r1 = SD_SendCmd(CMD9, 0, 0x01);

    if (r1 == 0)
    {
        r1 = SD_RecvData(csd_data, 16);
    }

    SD_DisSelect();

    if (r1)
        return 1;
    else
        return 0;
}

unsigned int SD_GetSectorCount(void)
{
    unsigned char csd[16];
    unsigned int Capacity;
    unsigned char n;
    unsigned int csize;

    if (SD_GetCSD(csd) != 0)
        return 0;

    if ((csd[0] & 0xC0) == 0x40)
    {
        csize = csd[9] + ((unsigned int)csd[8] << 8) + 1;
        Capacity = csize << 10;
    }
    else
    {
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((unsigned int)csd[7] << 2) + ((unsigned int)(csd[6] & 3) << 10) + 1;
        Capacity = csize << (n - 9);
    }

    return Capacity;
}

unsigned char SD_Initialize(void)
{
    unsigned char r1;
    unsigned int retry;
    unsigned char buf[4];
    unsigned char i;

    SD_SPI_Init();
    SD_SPI_SpeedLow;

    for (i = 0; i < 10; i++)
        SD_SPI_ReadWriteByte(0xFF);

    retry = 20;

    do
    {
        r1 = SD_SendCmd(CMD0, 0, 0x95);
    } while ((r1 != 0x01) && retry--);

    SD_Type = 0;

    if (r1 == 0x01)
    {
        if (SD_SendCmd(CMD8, 0x1AA, 0x87) == 1) // SD V2.0
        {
            for (i = 0; i < 4; i++)
                buf[i] = SD_SPI_ReadWriteByte(0xFF);    // Get trailing return value of R7 resp

            if ((buf[2] == 0x01) && (buf[3] == 0xAA))
            {
                retry = 0xFFFE;

                do
                {
                    SD_SendCmd(CMD55, 0, 0x01);
                    r1 = SD_SendCmd(CMD41, 0x40000000, 0x01);
                } while (r1 && retry--);

                if (retry && SD_SendCmd(CMD58, 0, 0x01) == 0)
                {
                    for (i = 0; i < 4; i++)
                        buf[i] = SD_SPI_ReadWriteByte(0xFF);

                    if (buf[0] & 0x40)
                        SD_Type = SD_TYPE_V2HC;
                    else
                        SD_Type = SD_TYPE_V2;
                }
            }
        }
        else
        {
            SD_SendCmd(CMD55, 0, 0x01);
            r1 = SD_SendCmd(CMD41, 0, 0x01);

            if (r1 <= 1)
            {
                SD_Type = SD_TYPE_V1;
                retry = 0xFFFE;

                do
                {
                    SD_SendCmd(CMD55, 0, 0x01);
                    r1 = SD_SendCmd(CMD41, 0, 0x01);
                } while (r1 && retry--);
            }
            else
            {
                SD_Type = SD_TYPE_MMC;
                retry = 0xFFFE;

                do
                {
                    r1 = SD_SendCmd(CMD1, 0, 0x01);
                } while (r1 && retry--);
            }

            if ((retry == 0) || (SD_SendCmd(CMD16, 512, 0x01) != 0))
                SD_Type = SD_TYPE_ERR;
        }
    }

    SD_DisSelect();
    SD_SPI_SpeedHigh;

    if (SD_Type)
        return 0;
    else if (r1)
        return r1;

    return 0xaa;
}

unsigned char SD_ReadDisk(unsigned char *buf, unsigned int sector, unsigned char cnt)
{
    unsigned char r1;

    if (SD_Type != SD_TYPE_V2HC)
        sector <<= 9;

    if (cnt == 1)
    {
        r1 = SD_SendCmd(CMD17, sector, 0x01);

        if (r1 == 0)
        {
            r1 = SD_RecvData(buf, 512);
        }
    }
    else
    {
        r1 = SD_SendCmd(CMD18, sector, 0x01);

        do
        {
            r1 = SD_RecvData(buf, 512);
            buf += 512;
        } while (--cnt && r1 == 0);

        SD_SendCmd(CMD12, 0, 0x01);
    }

    SD_DisSelect();
    return r1;
}

unsigned char SD_WriteDisk(unsigned char *buf, unsigned int  sector, unsigned char cnt)
{
    unsigned char r1;

    if (SD_Type != SD_TYPE_V2HC)
        sector *= 512;

    if (cnt == 1)
    {
        r1 = SD_SendCmd(CMD24, sector, 0x01);

        if (r1 == 0)
        {
            r1 = SD_SendBlock(buf, 0xFE);
        }
    }
    else
    {
        if (SD_Type != SD_TYPE_MMC)
        {
            SD_SendCmd(CMD55, 0, 0x01);
            SD_SendCmd(CMD23, cnt, 0x01);
        }

        r1 = SD_SendCmd(CMD25, sector, 0x01);

        if (r1 == 0)
        {
            do
            {
                r1 = SD_SendBlock(buf, 0xFC);
                buf += 512;
            } while (--cnt && r1 == 0);

            r1 = SD_SendBlock(0, 0xFD);
        }
    }

    SD_DisSelect();
    return r1;
}

unsigned char SD_CRC_OFF(void)
{
    unsigned char r1;
    r1 = SD_SendCmd(CRC_ON_OFF, 0, 0x25);

    if (r1 != 0x00)
    {
        printf("crc off error\n\r");
    }

    SD_DisSelect();

    if (r1)
        return 1;
    else
        return 0;
}

/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/
