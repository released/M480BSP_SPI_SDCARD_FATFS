/**************************************************************************//**
 * @file     fmc.c
 * @version  V1.00
 * @brief    M480 series FMC driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2016-2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>

#include "NuMicro.h"


/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup FMC_Driver FMC Driver
  @{
*/


/** @addtogroup FMC_EXPORTED_FUNCTIONS FMC Exported Functions
  @{
*/

int32_t  g_FMC_i32ErrCode;


/**
  * @brief Disable FMC ISP function.
  * @return None
  */
void FMC_Close(void)
{
    FMC->ISPCTL &= ~FMC_ISPCTL_ISPEN_Msk;
}

/**
  * @brief     Config XOM Region
  * @param[in] u32XomNum    The XOM number(0~3)
  * @param[in] u32XomBase   The XOM region base address.
  * @param[in] u8XomPage   The XOM page number of region size.
  *
  * @retval   0   Success
  * @retval   1   XOM is has already actived.
  * @retval   -1  Program failed.
  * @retval   -2  Invalid XOM number.
  *
  * @details  Program XOM base address and XOM size(page)
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Program failed or program time-out
  *           -2  Invalid XOM number.
  */
int32_t FMC_ConfigXOM(uint32_t u32XomNum, uint32_t u32XomBase, uint8_t u8XomPage)
{
    int32_t   tout;
    int32_t   ret;

    g_FMC_i32ErrCode = 0;

    if (u32XomNum >= 4UL)
    {
        g_FMC_i32ErrCode = -2;
        return -2;
    }

    ret = FMC_GetXOMState(u32XomNum);
    if (ret != 0)
        return ret;

    FMC->ISPCMD = FMC_ISPCMD_PROGRAM;
    FMC->ISPADDR = FMC_XOM_BASE + (u32XomNum * 0x10u);
    FMC->ISPDAT = u32XomBase;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
    tout = FMC_TIMEOUT_WRITE;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPSTS & FMC_ISPSTS_ISPFF_Msk)
    {
        FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    FMC->ISPCMD = FMC_ISPCMD_PROGRAM;
    FMC->ISPADDR = FMC_XOM_BASE + (u32XomNum * 0x10u + 0x04u);
    FMC->ISPDAT = u8XomPage;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
    tout = FMC_TIMEOUT_WRITE;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if(FMC->ISPSTS & FMC_ISPSTS_ISPFF_Msk)
    {
        FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    FMC->ISPCMD = FMC_ISPCMD_PROGRAM;
    FMC->ISPADDR = FMC_XOM_BASE + (u32XomNum * 0x10u + 0x08u);
    FMC->ISPDAT = 0u;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
    tout = FMC_TIMEOUT_WRITE;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if(FMC->ISPSTS & FMC_ISPSTS_ISPFF_Msk)
    {
        FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    return 0;
}

/**
  * @brief Execute FMC_ISPCMD_PAGE_ERASE command to erase a flash page. The page size is 4096 bytes.
  * @param[in]  u32PageAddr Address of the flash page to be erased.
  *             It must be a 4096 bytes aligned address.
  * @return ISP page erase success or not.
  * @retval   0  Success
  * @retval   -1  Erase failed
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Erase failed or erase time-out
  */
int32_t FMC_Erase(uint32_t u32PageAddr)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;

    if (u32PageAddr == FMC_SPROM_BASE)
    {
        return FMC_Erase_SPROM();
    }

    FMC->ISPCMD = FMC_ISPCMD_PAGE_ERASE;
    FMC->ISPADDR = u32PageAddr;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_ERASE;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPCTL & FMC_ISPCTL_ISPFF_Msk)
    {
        FMC->ISPCTL |= FMC_ISPCTL_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    return 0;
}


/**
  * @brief Execute FMC_ISPCMD_PAGE_ERASE command to erase SPROM. The page size is 4096 bytes.
  * @return   SPROM page erase success or not.
  * @retval   0  Success
  * @retval   -1  Erase failed
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Erase failed or erase time-out
  */
int32_t FMC_Erase_SPROM(void)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;
    FMC->ISPCMD = FMC_ISPCMD_PAGE_ERASE;
    FMC->ISPADDR = FMC_SPROM_BASE;
    FMC->ISPDAT = 0x0055AA03UL;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_ERASE;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPCTL & FMC_ISPCTL_ISPFF_Msk)
    {
        FMC->ISPCTL |= FMC_ISPCTL_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    return 0;
}

/**
  * @brief Execute FMC_ISPCMD_BLOCK_ERASE command to erase a flash block. The block size is 4 pages.
  * @param[in]  u32BlockAddr  Address of the flash block to be erased.
  *             It must be a 4 pages aligned address.
  * @return ISP page erase success or not.
  * @retval   0  Success
  * @retval   -1  Erase failed
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Erase failed or erase time-out
  */
int32_t FMC_Erase_Block(uint32_t u32BlockAddr)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;
    FMC->ISPCMD = FMC_ISPCMD_BLOCK_ERASE;
    FMC->ISPADDR = u32BlockAddr;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_ERASE;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPCTL & FMC_ISPCTL_ISPFF_Msk)
    {
        FMC->ISPCTL |= FMC_ISPCTL_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    return 0;
}

/**
  * @brief Execute FMC_ISPCMD_BANK_ERASE command to erase a flash block.
  * @param[in]  u32BankAddr Base address of the flash bank to be erased.
  * @return ISP page erase success or not.
  * @retval   0  Success
  * @retval   -1  Erase failed
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Erase failed or erase time-out
  */
int32_t FMC_Erase_Bank(uint32_t u32BankAddr)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;
    FMC->ISPCMD = FMC_ISPCMD_BANK_ERASE;
    FMC->ISPADDR = u32BankAddr;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_ERASE;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPCTL & FMC_ISPCTL_ISPFF_Msk)
    {
        FMC->ISPCTL |= FMC_ISPCTL_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    return 0;
}

/**
  * @brief  Execute Erase XOM Region
  *
  * @param[in]  u32XomNum  The XOMRn(n=0~3)
  *
  * @return   XOM erase success or not.
  * @retval    0  Success
  * @retval   -1  Erase failed
  * @retval   -2  Invalid XOM number.
  *
  * @details Execute FMC_ISPCMD_PAGE_ERASE command to erase XOM.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Erase failed or erase time-out
  *           -2  Invalid XOM number.
  */
int32_t FMC_EraseXOM(uint32_t u32XomNum)
{
    uint32_t  u32Addr;
    int32_t   i32Active, err = 0;
    int32_t   tout;

    if(u32XomNum >= 4UL)
    {
        err = -2;
    }

    if (err == 0)
    {
        i32Active = FMC_GetXOMState(u32XomNum);

        if(i32Active)
        {
            switch(u32XomNum)
            {
            case 0u:
                u32Addr = (FMC->XOMR0STS & 0xFFFFFF00u) >> 8u;
                break;
            case 1u:
                u32Addr = (FMC->XOMR1STS & 0xFFFFFF00u) >> 8u;
                break;
            case 2u:
                u32Addr = (FMC->XOMR2STS & 0xFFFFFF00u) >> 8u;
                break;
            case 3u:
                u32Addr = (FMC->XOMR3STS & 0xFFFFFF00u) >> 8u;
                break;
            default:
                break;
            }
            FMC->ISPCMD = FMC_ISPCMD_PAGE_ERASE;
            FMC->ISPADDR = u32Addr;
            FMC->ISPDAT = 0x55aa03u;
            FMC->ISPTRG = 0x1u;
#if ISBEN
            __ISB();
#endif
            tout = FMC_TIMEOUT_ERASE;
            while ((tout-- > 0) && FMC->ISPTRG) {}
            if (tout <= 0)
                err = -1;

            /* Check ISPFF flag to know whether erase OK or fail. */
            if(FMC->ISPCTL & FMC_ISPCTL_ISPFF_Msk)
            {
                FMC->ISPCTL |= FMC_ISPCTL_ISPFF_Msk;
                err = -1;
            }
        }
        else
        {
            err = -1;
        }
    }
    g_FMC_i32ErrCode = err;
    return err;
}

/**
  * @brief  Check the XOM is actived or not.
  *
  * @param[in] u32XomNum    The xom number(0~3).
  *
  * @retval   1   XOM is actived.
  * @retval   0   XOM is not actived.
  * @retval   -2  Invalid XOM number.
  *
  * @details To get specify XOMRn(n=0~3) active status
  */
int32_t FMC_GetXOMState(uint32_t u32XomNum)
{
    uint32_t u32act;
    int32_t  ret = 0;

    if (u32XomNum >= 4UL)
        ret = -2;

    if (ret >= 0)
    {
        u32act = (((FMC->XOMSTS) & 0xful) & (1ul << u32XomNum)) >> u32XomNum;
        ret = (int32_t)u32act;
    }
    return ret;
}

/**
  * @brief Get the current boot source.
  * @return The current boot source.
  * @retval   0  Is boot from APROM.
  * @retval   1  Is boot from LDROM.
  * @retval   2  Is boot from Boot Loader.
  */
int32_t FMC_GetBootSource (void)
{
    if (FMC->ISPCTL & FMC_ISPCTL_BL_Msk)
    {
        return 2;
    }
    if (FMC->ISPCTL & FMC_ISPCTL_BS_Msk)
    {
        return 1;
    }
    return 0;
}


/**
  * @brief Enable FMC ISP function
  * @return None
  */
void FMC_Open(void)
{
    FMC->ISPCTL |=  FMC_ISPCTL_ISPEN_Msk;
}


/**
  * @brief Execute FMC_ISPCMD_READ command to read a word from flash.
  * @param[in]  u32Addr Address of the flash location to be read.
  *             It must be a word aligned address.
  * @return  The word data read from specified flash address.
  *          Return 0xFFFFFFFF if read failed.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Read time-out
  */
uint32_t FMC_Read(uint32_t u32Addr)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;
    FMC->ISPCMD = FMC_ISPCMD_READ;
    FMC->ISPADDR = u32Addr;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_READ;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return 0xFFFFFFFF;
    }
    return FMC->ISPDAT;
}


/**
  * @brief Execute FMC_ISPCMD_READ_64 command to read a double-word from flash.
  * @param[in]  u32addr   Address of the flash location to be read.
  *             It must be a double-word aligned address.
  * @param[out] u32data0  Place holder of word 0 read from flash address u32addr.
  * @param[out] u32data1  Place holder of word 0 read from flash address u32addr+4.
  * @return   0   Success
  * @return   -1  Failed
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Read time-out
  */
int32_t FMC_Read_64(uint32_t u32addr, uint32_t * u32data0, uint32_t * u32data1)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;
    FMC->ISPCMD = FMC_ISPCMD_READ_64;
    FMC->ISPADDR    = u32addr;
    FMC->ISPDAT = 0x0UL;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_READ;
    while ((tout-- > 0) && (FMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPSTS & FMC_ISPSTS_ISPFF_Msk)
    {
        FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    else
    {
        *u32data0 = FMC->MPDAT0;
        *u32data1 = FMC->MPDAT1;
    }
    return 0;
}


/**
  * @brief    Get the base address of Data Flash if enabled.
  * @retval   The base address of Data Flash
  */
uint32_t FMC_ReadDataFlashBaseAddr(void)
{
    return FMC->DFBA;
}

/**
  * @brief      Set boot source from LDROM or APROM after next software reset
  * @param[in]  i32BootSrc
  *                1: Boot from LDROM
  *                0: Boot from APROM
  * @return    None
  * @details   This function is used to switch APROM boot or LDROM boot. User need to call
  *            FMC_SetBootSource to select boot source first, then use CPU reset or
  *            System Reset Request to reset system.
  */
void FMC_SetBootSource(int32_t i32BootSrc)
{
    if(i32BootSrc)
    {
        FMC->ISPCTL |= FMC_ISPCTL_BS_Msk; /* Boot from LDROM */
    }
    else
    {
        FMC->ISPCTL &= ~FMC_ISPCTL_BS_Msk;/* Boot from APROM */
    }
}

/**
  * @brief Execute ISP FMC_ISPCMD_PROGRAM to program a word to flash.
  * @param[in]  u32Addr Address of the flash location to be programmed.
  *             It must be a word aligned address.
  * @param[in]  u32Data The word data to be programmed.
  * @return   0   Success
  * @return   -1  Program Failed
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Program failed or time-out
  */
int32_t FMC_Write(uint32_t u32Addr, uint32_t u32Data)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;
    FMC->ISPCMD = FMC_ISPCMD_PROGRAM;
    FMC->ISPADDR = u32Addr;
    FMC->ISPDAT = u32Data;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_WRITE;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPSTS & FMC_ISPSTS_ISPFF_Msk)
    {
        FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    return 0;
}

/**
  * @brief Execute ISP FMC_ISPCMD_PROGRAM_64 to program a double-word to flash.
  * @param[in]  u32addr Address of the flash location to be programmed.
  *             It must be a double-word aligned address.
  * @param[in]  u32data0   The word data to be programmed to flash address u32addr.
  * @param[in]  u32data1   The word data to be programmed to flash address u32addr+4.
  * @return   0   Success
  * @return   -1  Failed
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Program failed or time-out
  */
int32_t FMC_Write8Bytes(uint32_t u32addr, uint32_t u32data0, uint32_t u32data1)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;
    FMC->ISPCMD  = FMC_ISPCMD_PROGRAM_64;
    FMC->ISPADDR = u32addr;
    FMC->MPDAT0  = u32data0;
    FMC->MPDAT1  = u32data1;
    FMC->ISPTRG  = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_WRITE;
    while ((tout-- > 0) && (FMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPSTS & FMC_ISPSTS_ISPFF_Msk)
    {
        FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    return 0;
}


/**
  * @brief   Program Multi-Word data into specified address of flash.
  * @param[in]  u32Addr    Start flash address in APROM where the data chunk to be programmed into.
  *                        This address must be 8-bytes aligned to flash address.
  * @param[in]  pu32Buf    Buffer that carry the data chunk.
  * @param[in]  u32Len     Length of the data chunk in bytes.
  * @retval   >=0  Number of data bytes were programmed.
  * @return   -1   Program failed.
  * @return   -2   Invalid address.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Program failed or time-out
  *           -2  Invalid address
  */
int32_t FMC_WriteMultiple(uint32_t u32Addr, uint32_t pu32Buf[], uint32_t u32Len)
{
    int   i, idx, retval = 0;
    int32_t  tout;

    g_FMC_i32ErrCode = 0;

    if ((u32Addr >= FMC_APROM_END) || ((u32Addr % 8) != 0))
    {
        g_FMC_i32ErrCode = -2;
        return -2;
    }

    u32Len = u32Len - (u32Len % 8);         /* u32Len must be multiple of 8. */

    idx = 0;

    while (u32Len >= 8)
    {
        FMC->ISPADDR = u32Addr;
        FMC->MPDAT0  = pu32Buf[idx++];
        FMC->MPDAT1  = pu32Buf[idx++];
        FMC->MPDAT2  = pu32Buf[idx++];
        FMC->MPDAT3  = pu32Buf[idx++];
        FMC->ISPCMD  = FMC_ISPCMD_PROGRAM_MUL;
        FMC->ISPTRG  = FMC_ISPTRG_ISPGO_Msk;

        for (i = 16; i < FMC_MULTI_WORD_PROG_LEN; )
        {
            tout = FMC_TIMEOUT_WRITE;
            while ((tout-- > 0) && (FMC->MPSTS & (FMC_MPSTS_D0_Msk | FMC_MPSTS_D1_Msk))) {}
            if (tout <= 0)
            {
                g_FMC_i32ErrCode = -1;
                return -1;
            }

            retval += 8;
            u32Len -= 8;
            if (u32Len < 8)
            {
                return retval;
            }

            if (!(FMC->MPSTS & FMC_MPSTS_MPBUSY_Msk))
            {
                /* printf("    [WARNING] busy cleared after D0D1 cleared!\n"); */
                i += 8;
                break;
            }

            FMC->MPDAT0 = pu32Buf[idx++];
            FMC->MPDAT1 = pu32Buf[idx++];

            if (i == FMC_MULTI_WORD_PROG_LEN/4)
                break;           // done

            tout = FMC_TIMEOUT_WRITE;
            while ((tout-- > 0) && (FMC->MPSTS & (FMC_MPSTS_D2_Msk | FMC_MPSTS_D3_Msk))) {}
            if (tout <= 0)
            {
                g_FMC_i32ErrCode = -1;
                return -1;
            }

            retval += 8;
            u32Len -= 8;
            if (u32Len < 8)
            {
                return retval;
            }

            if (!(FMC->MPSTS & FMC_MPSTS_MPBUSY_Msk))
            {
                /* printf("    [WARNING] busy cleared after D2D3 cleared!\n"); */
                i += 8;
                break;
            }

            FMC->MPDAT2 = pu32Buf[idx++];
            FMC->MPDAT3 = pu32Buf[idx++];
        }

        if (i != FMC_MULTI_WORD_PROG_LEN)
        {
            /* printf("    [WARNING] Multi-word program interrupted at 0x%x !!\n", i); */
            return retval;
        }

        tout = FMC_TIMEOUT_WRITE;
        while ((tout-- > 0) && (FMC->MPSTS & FMC_MPSTS_MPBUSY_Msk)) {}
        if (tout <= 0)
        {
            g_FMC_i32ErrCode = -1;
            return -1;
        }

        u32Addr += FMC_MULTI_WORD_PROG_LEN;
    }
    return retval;
}


/**
  * @brief Program a 64-bits data to the specified OTP.
  * @param[in] otp_num    The OTP number.
  * @param[in] low_word   Low word of the 64-bits data.
  * @param[in] high_word   Low word of the 64-bits data.
  * @retval   0   Success
  * @retval   -1  Program failed.
  * @retval   -2  Invalid OTP number.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Program failed or time-out
  *           -2  Invalid OTP number
  */
int32_t FMC_Write_OTP(uint32_t otp_num, uint32_t low_word, uint32_t high_word)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;

    if (otp_num > 255UL)
    {
        g_FMC_i32ErrCode = -2;
        return -2;
    }

    FMC->ISPCMD = FMC_ISPCMD_PROGRAM;
    FMC->ISPADDR = FMC_OTP_BASE + otp_num * 8UL;
    FMC->ISPDAT = low_word;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_WRITE;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPSTS & FMC_ISPSTS_ISPFF_Msk)
    {
        FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    FMC->ISPCMD = FMC_ISPCMD_PROGRAM;
    FMC->ISPADDR = FMC_OTP_BASE + otp_num * 8UL + 4UL;
    FMC->ISPDAT = high_word;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_WRITE;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPSTS & FMC_ISPSTS_ISPFF_Msk)
    {
        FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    return 0;
}

/**
  * @brief  Read the 64-bits data from the specified OTP.
  * @param[in] otp_num    The OTP number.
  * @param[in] low_word   Low word of the 64-bits data.
  * @param[in] high_word   Low word of the 64-bits data.
  * @retval   0   Success
  * @retval   -1  Read failed.
  * @retval   -2  Invalid OTP number.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Read failed or time-out
  *           -2  Invalid OTP number
  */
int32_t FMC_Read_OTP(uint32_t otp_num, uint32_t *low_word, uint32_t *high_word)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;

    if (otp_num > 255UL)
    {
        g_FMC_i32ErrCode = -2;
        return -2;
    }

    FMC->ISPCMD = FMC_ISPCMD_READ_64;
    FMC->ISPADDR = FMC_OTP_BASE + otp_num * 8UL ;
    FMC->ISPDAT = 0x0UL;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_WRITE;
    while ((tout-- > 0) && (FMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPSTS & FMC_ISPSTS_ISPFF_Msk)
    {
        FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    else
    {
        *low_word = FMC->MPDAT0;
        *high_word = FMC->MPDAT1;
    }
    return 0;
}

/**
  * @brief  Lock the specified OTP.
  * @param[in] otp_num    The OTP number.
  * @retval   0   Success
  * @retval   -1  Failed to write OTP lock bits.
  * @retval   -2  Invalid OTP number.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Failed to write OTP lock bits or write time-out
  *           -2  Invalid OTP number
  */
int32_t FMC_Lock_OTP(uint32_t otp_num)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;

    if (otp_num > 255UL)
    {
        g_FMC_i32ErrCode = -2;
        return -2;
    }

    FMC->ISPCMD = FMC_ISPCMD_PROGRAM;
    FMC->ISPADDR = FMC_OTP_BASE + 0x800UL + otp_num * 4UL;
    FMC->ISPDAT = 0UL;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_WRITE;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPSTS & FMC_ISPSTS_ISPFF_Msk)
    {
        FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    return 0;
}

/**
  * @brief  Check the OTP is locked or not.
  * @param[in] otp_num    The OTP number.
  * @retval   1   OTP is locked.
  * @retval   0   OTP is not locked.
  * @retval   -1  Failed to read OTP lock bits.
  * @retval   -2  Invalid OTP number.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Failed to read OTP lock bits or read time-out
  *           -2  Invalid OTP number
  */
int32_t FMC_Is_OTP_Locked(uint32_t otp_num)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;

    if (otp_num > 255UL)
    {
        g_FMC_i32ErrCode = -2;
        return -2;
    }

    FMC->ISPCMD = FMC_ISPCMD_READ;
    FMC->ISPADDR = FMC_OTP_BASE + 0x800UL + otp_num * 4UL;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_READ;
    while ((tout-- > 0) && (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    if (FMC->ISPSTS & FMC_ISPSTS_ISPFF_Msk)
    {
        FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;
        g_FMC_i32ErrCode = -1;
        return -1;
    }
    else
    {
        if (FMC->ISPDAT != 0xFFFFFFFFUL)
        {
            g_FMC_i32ErrCode = -1;
            return 1;   /* Lock work was progrmmed. OTP was locked. */
        }
    }
    return 0;
}

/**
  * @brief Execute FMC_ISPCMD_READ command to read User Configuration.
  * @param[out]  u32Config A two-word array.
  *              u32Config[0] holds CONFIG0, while u32Config[1] holds CONFIG1.
  * @param[in] u32Count Available word count in u32Config.
  * @return Success or not.
  * @retval   0  Success.
  * @retval   -1  Read failed
  * @retval   -2  Invalid parameter.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Read failed
  *           -2  Invalid parameter
  */
int32_t FMC_ReadConfig(uint32_t u32Config[], uint32_t u32Count)
{
    int32_t   ret = 0;

    u32Config[0] = FMC_Read(FMC_CONFIG_BASE);

    if (g_FMC_i32ErrCode != 0)
        return g_FMC_i32ErrCode;

    if (u32Count < 2UL)
    {
        ret = -2;
    }
    else
    {
        u32Config[1] = FMC_Read(FMC_CONFIG_BASE+4UL);
    }
    return ret;
}


/**
  * @brief Execute ISP commands to erase then write User Configuration.
  * @param[in] u32Config   A two-word array.
  *            u32Config[0] holds CONFIG0, while u32Config[1] holds CONFIG1.
  * @param[in] u32Count    The number of User Configuration words to be written.
  * @return Success or not.
  * @retval   0   Success
  * @retval   -1  Erase/program/read/verify failed
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           < 0  Errors caused by erase/program/read failed or time-out
  */
int32_t FMC_WriteConfig(uint32_t u32Config[], uint32_t u32Count)
{
    int   i;

    FMC_ENABLE_CFG_UPDATE();

    if (FMC_Erase(FMC_CONFIG_BASE) != 0)
        return -1;

    if ((FMC_Read(FMC_CONFIG_BASE) != 0xFFFFFFFF) || (FMC_Read(FMC_CONFIG_BASE+4) != 0xFFFFFFFF) ||
            (FMC_Read(FMC_CONFIG_BASE+8) != 0xFFFF5A5A))
    {
        FMC_DISABLE_CFG_UPDATE();
        return -1;
    }

    if (g_FMC_i32ErrCode != 0)
    {
        FMC_DISABLE_CFG_UPDATE();
        return -1;
    }

    for (i = 0; i < u32Count; i++)
    {
        if (FMC_Write(FMC_CONFIG_BASE+i*4UL, u32Config[i]) != 0)
        {
            FMC_DISABLE_CFG_UPDATE();
            return -1;
        }

        if (FMC_Read(FMC_CONFIG_BASE+i*4UL) != u32Config[i])
        {
            FMC_DISABLE_CFG_UPDATE();
            return -1;
        }

        if (g_FMC_i32ErrCode != 0)
        {
            FMC_DISABLE_CFG_UPDATE();
            return -1;
        }
    }

    FMC_DISABLE_CFG_UPDATE();
    return 0;
}


/**
  * @brief Run CRC32 checksum calculation and get result.
  * @param[in] u32addr   Starting flash address. It must be a page aligned address.
  * @param[in] u32count  Byte count of flash to be calculated. It must be multiple of 512 bytes.
  * @return Success or not.
  * @retval   0           Success.
  * @retval   0xFFFFFFFF  Invalid parameter or command failed.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  Run/Read check sum time-out failed
  *           -2  u32addr or u32count must be aligned with 512
  */
uint32_t  FMC_GetChkSum(uint32_t u32addr, uint32_t u32count)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;

    if ((u32addr % 512UL) || (u32count % 512UL))
    {
        g_FMC_i32ErrCode = -2;
        return 0xFFFFFFFF;
    }

    FMC->ISPCMD  = FMC_ISPCMD_RUN_CKS;
    FMC->ISPADDR = u32addr;
    FMC->ISPDAT  = u32count;
    FMC->ISPTRG  = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_CHKSUM;
    while ((tout-- > 0) && (FMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return 0xFFFFFFFF;
    }

    FMC->ISPCMD = FMC_ISPCMD_READ_CKS;
    FMC->ISPADDR    = u32addr;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_CHKSUM;
    while ((tout-- > 0) && (FMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return 0xFFFFFFFF;
    }

    return FMC->ISPDAT;
}


/**
  * @brief Run flash all one verification and get result.
  * @param[in] u32addr   Starting flash address. It must be a page aligned address.
  * @param[in] u32count  Byte count of flash to be calculated. It must be multiple of 512 bytes.
  * @retval   READ_ALLONE_YES      The contents of verified flash area are 0xFFFFFFFF.
  * @retval   READ_ALLONE_NOT  Some contents of verified flash area are not 0xFFFFFFFF.
  * @retval   READ_ALLONE_CMD_FAIL  Unexpected error occurred.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *           -1  RUN_ALL_ONE or CHECK_ALL_ONE commands time-out
  */
uint32_t  FMC_CheckAllOne(uint32_t u32addr, uint32_t u32count)
{
    int32_t  tout;

    g_FMC_i32ErrCode = 0;

    FMC->ISPSTS = 0x80UL;   /* clear check all one bit */

    FMC->ISPCMD   = FMC_ISPCMD_RUN_ALL1;
    FMC->ISPADDR  = u32addr;
    FMC->ISPDAT   = u32count;
    FMC->ISPTRG   = FMC_ISPTRG_ISPGO_Msk;

    tout = FMC_TIMEOUT_CHKALLONE;
    while ((tout-- > 0) && (FMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return READ_ALLONE_CMD_FAIL;
    }

    tout = FMC_TIMEOUT_CHKALLONE;
    do
    {
        FMC->ISPCMD = FMC_ISPCMD_READ_ALL1;
        FMC->ISPADDR    = u32addr;
        FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
        while ((tout-- > 0) && (FMC->ISPSTS & FMC_ISPSTS_ISPBUSY_Msk)) {}
        if (tout <= 0)
        {
            g_FMC_i32ErrCode = -1;
            return READ_ALLONE_CMD_FAIL;
        }
    }
    while (FMC->ISPDAT == 0UL);

    if ((FMC->ISPDAT == READ_ALLONE_YES) || (FMC->ISPDAT == READ_ALLONE_NOT))
        return FMC->ISPDAT;
    else
    {
        g_FMC_i32ErrCode = -1;
        return READ_ALLONE_CMD_FAIL;
    }
}


/**
  * @brief    Setup security key.
  * @param[in] key      Key 0~2 to be setup.
  * @param[in] kpmax    Maximum unmatched power-on counting number.
  * @param[in] kemax    Maximum unmatched counting number.
  * @param[in] lock_CONFIG   1: Security key lock CONFIG to write-protect. 0: Don't lock CONFIG.
  * @param[in] lock_SPROM    1: Security key lock SPROM to write-protect. 0: Don't lock SPROM.
  * @retval   0     Success.
  * @retval   -1    Key is locked. Cannot overwrite the current key.
  * @retval   -2    Failed to erase flash.
  * @retval   -3    Program key time-out failed
  * @retval   -4    Key lock function failed.
  * @retval   -5    CONFIG lock function failed.
  * @retval   -6    SPROM lock function failed.
  * @retval   -7    KPMAX function failed.
  * @retval   -8    KEMAX function failed.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *                 Same as the return value of this function.
  */
int32_t  FMC_SetSPKey(uint32_t key[3], uint32_t kpmax, uint32_t kemax,
                      const int32_t lock_CONFIG, const int32_t lock_SPROM)
{
    uint32_t  lock_ctrl = 0UL;
    uint32_t  u32KeySts;
    int32_t   tout;
    int32_t   ret = 0;

    g_FMC_i32ErrCode = 0;

    if (FMC->KPKEYSTS != 0x200UL)
    {
        ret = -1;
    }

    if (FMC_Erase(FMC_KPROM_BASE))
    {
        ret = -2;
    }

    if (FMC_Erase(FMC_KPROM_BASE+0x200UL))
    {
        ret = -3;
    }

    if (!lock_CONFIG)
    {
        lock_ctrl |= 0x1UL;
    }

    if (!lock_SPROM)
    {
        lock_ctrl |= 0x2UL;
    }

    if (ret == 0)
    {
        FMC_Write(FMC_KPROM_BASE, key[0]);
        FMC_Write(FMC_KPROM_BASE+0x4UL, key[1]);
        FMC_Write(FMC_KPROM_BASE+0x8UL, key[2]);
        FMC_Write(FMC_KPROM_BASE+0xCUL, kpmax);
        FMC_Write(FMC_KPROM_BASE+0x10UL, kemax);
        FMC_Write(FMC_KPROM_BASE+0x14UL, lock_ctrl);

        tout = FMC_TIMEOUT_WRITE;
        while ((tout-- > 0) && (FMC->KPKEYSTS & FMC_KPKEYSTS_KEYBUSY_Msk)) {}
        if (tout <= 0)
        {
            g_FMC_i32ErrCode = -3;
            return -3;
        }

        u32KeySts = FMC->KPKEYSTS;

        if (!(u32KeySts & FMC_KPKEYSTS_KEYLOCK_Msk))
        {
            /* Security key lock failed! */
            ret = -4;
        }
        else if ((lock_CONFIG && (!(u32KeySts & FMC_KPKEYSTS_CFGFLAG_Msk))) ||
                 ((!lock_CONFIG) && (u32KeySts & FMC_KPKEYSTS_CFGFLAG_Msk)))
        {
            /* CONFIG lock failed! */
            ret = -5;
        }
        else if ((lock_SPROM && (!(u32KeySts & FMC_KPKEYSTS_SPFLAG_Msk))) ||
                 ((!lock_SPROM) && (u32KeySts & FMC_KPKEYSTS_SPFLAG_Msk)))
        {
            /* CONFIG lock failed! */
            ret = -6;
        }
        else if (((FMC->KPCNT & FMC_KPCNT_KPMAX_Msk) >> FMC_KPCNT_KPMAX_Pos) != kpmax)
        {
            /* KPMAX failed! */
            ret = -7;
        }
        else if (((FMC->KPKEYCNT & FMC_KPKEYCNT_KPKEMAX_Msk) >> FMC_KPKEYCNT_KPKEMAX_Pos) != kemax)
        {
            /* KEMAX failed! */
            ret = -8;
        }
    }
    g_FMC_i32ErrCode = ret;
    return ret;
}


/**
  * @brief    Execute security key comparison.
  * @param[in] key  Key 0~2 to be compared.
  * @retval   0     Key matched.
  * @retval   -1    Command failed.
  * @retval   -2    Forbidden. Times of key comparison mismatch reach the maximum count.
  * @retval   -3    Key mismatched.
  * @retval   -4    No security key lock. Key comparison is not required.
  * @retval   -5    Key matched, but failed to unlock.
  *
  * @note     Global error code g_FMC_i32ErrCode
  *                 Same as the return value of this function.
  */
int32_t  FMC_CompareSPKey(uint32_t key[3])
{
    uint32_t  u32KeySts;
    int32_t   tout;

    g_FMC_i32ErrCode = 0;

    if (FMC->KPKEYSTS & FMC_KPKEYSTS_FORBID_Msk)
    {
        /* FMC_CompareSPKey - FORBID!  */
        g_FMC_i32ErrCode = -2;
        return -2;
    }

    if (!(FMC->KPKEYSTS & FMC_KPKEYSTS_KEYLOCK_Msk))
    {
        /* FMC_CompareSPKey - key is not locked!  */
        g_FMC_i32ErrCode = -4;
        return -4;
    }

    FMC->KPKEY0 = key[0];
    FMC->KPKEY1 = key[1];
    FMC->KPKEY2 = key[2];
    FMC->KPKEYTRG = FMC_KPKEYTRG_KPKEYGO_Msk | FMC_KPKEYTRG_TCEN_Msk;

    tout = FMC_TIMEOUT_READ;
    while ((tout-- > 0) && (FMC->KPKEYSTS & FMC_KPKEYSTS_KEYBUSY_Msk)) {}
    if (tout <= 0)
    {
        g_FMC_i32ErrCode = -1;
        return -1;
    }

    u32KeySts = FMC->KPKEYSTS;

    if (!(u32KeySts & FMC_KPKEYSTS_KEYMATCH_Msk))
    {
        /* Key mismatched! */
        g_FMC_i32ErrCode = -3;
        return -3;
    }
    else if (u32KeySts & FMC_KPKEYSTS_KEYLOCK_Msk)
    {
        /* Key matched, but failed to unlock! */
        g_FMC_i32ErrCode = -5;
        return -5;
    }
    return 0;
}


/*@}*/ /* end of group FMC_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group FMC_Driver */

/*@}*/ /* end of group Standard_Driver */

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/


