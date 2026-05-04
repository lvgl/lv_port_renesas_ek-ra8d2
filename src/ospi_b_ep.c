/***********************************************************************************************************************
 * File Name    : ospi_b_ep.c
 * Description  : Contains data structures and functions used in ospi_b_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "ospi_b_commands.h"
#include "ospi_b_ep.h"


#define RESET_VALUE             (0x00)


/*******************************************************************************************************************//**
 * @addtogroup ospi_b_ep.c
 * @{
 **********************************************************************************************************************/

/* External variable */
extern spi_flash_direct_transfer_t g_ospi_b_direct_transfer [OSPI_B_TRANSFER_MAX];


/* Function declarations */
static fsp_err_t ospi_b_write_enable(void);


#if defined (BOARD_RA8D1_EK) || defined (BOARD_RA8M1_EK) || defined (BOARD_RA8D2_EK)
    uint32_t g_autocalibration_data[] BSP_PLACE_IN_SECTION(".ospi0_cs1") =
    {
        0xFFFF0000U,
        0x000800FFU,
        0x00FFF700U,
        0xF700F708U
    };
#endif


/*******************************************************************************************************************//**
 * @brief       This function enables write and verify the read data.
 * @param[in]   None.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      FSP_ERR_ABORTED Upon incorrect read data.
 * @retval      Any other error code apart from FSP_SUCCESS Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t ospi_b_write_enable(void)
{
    fsp_err_t err = FSP_SUCCESS;
    spi_flash_direct_transfer_t transfer =
    {
        .command        = RESET_VALUE,
        .address        = RESET_VALUE,
        .data           = RESET_VALUE,
        .command_length = RESET_VALUE,
        .address_length = RESET_VALUE,
        .data_length    = RESET_VALUE,
        .dummy_cycles   = RESET_VALUE
    };

    /* Transfer write enable command */
    transfer = (SPI_FLASH_PROTOCOL_EXTENDED_SPI == g_ospi_b_ctrl.spi_protocol)
               ? g_ospi_b_direct_transfer[OSPI_B_TRANSFER_WRITE_ENABLE_SPI]
               : g_ospi_b_direct_transfer[OSPI_B_TRANSFER_WRITE_ENABLE_OPI];
    err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    if (err != FSP_SUCCESS)
    {
        return err;
    }

    /* Read Status Register */
    transfer = (SPI_FLASH_PROTOCOL_EXTENDED_SPI == g_ospi_b_ctrl.spi_protocol)
               ? g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_STATUS_SPI]
               : g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_STATUS_OPI];
    err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    if (err != FSP_SUCCESS)
    {
        return err;
    }

    /* Check Write Enable bit in Status Register */
    if (OSPI_B_WEN_BIT_MASK != (transfer.data & OSPI_B_WEN_BIT_MASK))
    {
        return FSP_ERR_ABORTED;
    }
    return err;
}



static spi_flash_cfg_t           g_ram_ospi_cfg;
static ospi_b_extended_cfg_t     g_ram_ospi_extended_cfg;



/*******************************************************************************************************************//**
 * @brief       This functions initializes OSPI module and Flash device.
 * @param       None.
 * @retval      FSP_SUCCESS     Upon successful initialization of OSPI module and Flash device.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t ospi_b_init(void)
{
    /* By default, the flash device is in SPI mode, so it is necessary to open the OSPI module in SPI mode */
    fsp_err_t err = FSP_SUCCESS;
    spi_flash_direct_transfer_t transfer =
    {
        .command        = RESET_VALUE,
        .address        = RESET_VALUE,
        .data           = RESET_VALUE,
        .command_length = RESET_VALUE,
        .address_length = RESET_VALUE,
        .data_length    = RESET_VALUE,
        .dummy_cycles   = RESET_VALUE
    };

    memcpy(&g_ram_ospi_cfg, &g_ospi_b_cfg, sizeof(spi_flash_cfg_t));
    memcpy(&g_ram_ospi_extended_cfg, g_ospi_b_cfg.p_extend, sizeof(ospi_b_extended_cfg_t));

    /* Set the erase commands for High Speed mode for 8D-8D-8D */
    g_ram_ospi_cfg.p_extend = &g_ram_ospi_extended_cfg;


    g_ram_ospi_extended_cfg.p_autocalibration_preamble_pattern_addr = (uint8_t*)&g_autocalibration_data[0];

    /* Open OSPI module */
    err = R_OSPI_B_Open(&g_ospi_b_ctrl, &g_ram_ospi_cfg);
    if (err != FSP_SUCCESS)
    {
        return err;
    }

    /* Switch OSPI module to 1S-1S-1S mode to configure flash device */
    err = R_OSPI_B_SpiProtocolSet(&g_ospi_b_ctrl, SPI_FLASH_PROTOCOL_EXTENDED_SPI);
    if (err != FSP_SUCCESS)
    {
        return err;
    }

    /* Reset flash device */
    R_XSPI0->LIOCTL_b.RSTCS0 = 0;
    R_BSP_SoftwareDelay(OSPI_B_TIME_RESET_PULSE, OSPI_B_TIME_UNIT);
    R_XSPI0->LIOCTL_b.RSTCS0 = 1;
    R_BSP_SoftwareDelay(OSPI_B_TIME_RESET_SETUP, OSPI_B_TIME_UNIT);

    /* Transfer write enable command */
    err = ospi_b_write_enable();
    if (err != FSP_SUCCESS)
    {
        return err;
    }

    /* Enter 4-bytes address */
    transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_ENTER_4BYTES_ADDRESS];
    err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    if (err != FSP_SUCCESS)
    {
        return err;
    }

    /* Write to ADDR 00000001H of CR to configure dummy cycle */
    transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_WRITE_CR_01H_SPI];
    transfer.data    = OSPI_B_DATA_SET_CR_01H;
    err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    if (err != FSP_SUCCESS)
    {
        return err;
    }

    /* Read back and verify CR register data */
    transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_CR_01H_SPI];
    err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    if (err != FSP_SUCCESS)
    {
        return err;
    }

    if (OSPI_B_DATA_SET_CR_01H != (uint8_t)transfer.data)
    {
        return FSP_ERR_ABORTED;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief       This function configures OSPI to extended SPI mode.
 * @param[in]   None.
 * @retval      FSP_SUCCESS     Upon successful transition to SPI operating mode.
 * @retval      FSP_ERR_ABORTED Upon incorrect read data.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t ospi_b_set_protocol_to_spi(void)
{
    fsp_err_t err = FSP_SUCCESS;
    spi_flash_direct_transfer_t transfer =
    {
        .command        = RESET_VALUE,
        .address        = RESET_VALUE,
        .data           = RESET_VALUE,
        .command_length = RESET_VALUE,
        .address_length = RESET_VALUE,
        .data_length    = RESET_VALUE,
        .dummy_cycles   = RESET_VALUE
    };
    bsp_octaclk_settings_t      octaclk  =
    {
        .source_clock  = RESET_VALUE,
        .divider       = RESET_VALUE
    };

    if (SPI_FLASH_PROTOCOL_EXTENDED_SPI == g_ospi_b_ctrl.spi_protocol)
    {
        /* Do nothing */
    }
    else if (SPI_FLASH_PROTOCOL_8D_8D_8D == g_ospi_b_ctrl.spi_protocol)
    {
        /* Transfer write enable command */
        err = ospi_b_write_enable();
        if (err != FSP_SUCCESS)
        {
            return err;
        }

        /* Change to SPI mode */
        transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_WRITE_CR_00H_OPI];
        transfer.data = OSPI_B_DATA_SET_SPI_CR_00H;
        err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
        if (err != FSP_SUCCESS)
        {
            return err;
        }

        /* Change the OCTACLK clock to 118 MHz in SDR mode without OM_DQS */
        octaclk.source_clock = BSP_CLOCKS_SOURCE_CLOCK_PLL2Q;
        octaclk.divider      = BSP_CLOCKS_OCTA_CLOCK_DIV_1;
        R_BSP_OctaclkUpdate(&octaclk);

        /* Switch OSPI module mode to SPI mode */
        err = R_OSPI_B_SpiProtocolSet(&g_ospi_b_ctrl, SPI_FLASH_PROTOCOL_EXTENDED_SPI);
        if (err != FSP_SUCCESS)
        {
            return err;
        }

        /* Read back and verify CR register data */
        transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_CR_00H_SPI];
        err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
        if (err != FSP_SUCCESS)
        {
            return err;
        }

        if (OSPI_B_DATA_SET_SPI_CR_00H != (uint8_t)transfer.data)
        {
            return FSP_ERR_ABORTED;
        }

    }

    else
    {
        return FSP_ERR_INVALID_MODE;
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief       This function configures OSPI to OPI mode.
 * @param[in]   None.
 * @retval      FSP_SUCCESS     Upon successful transition to OPI operating mode.
 * @retval      FSP_ERR_ABORTED Upon incorrect read data.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t ospi_b_set_protocol_to_opi(void)
{
    fsp_err_t err = FSP_SUCCESS;
    spi_flash_direct_transfer_t transfer =
    {
        .command        = RESET_VALUE,
        .address        = RESET_VALUE,
        .data           = RESET_VALUE,
        .command_length = RESET_VALUE,
        .address_length = RESET_VALUE,
        .data_length    = RESET_VALUE,
        .dummy_cycles   = RESET_VALUE
    };
    bsp_octaclk_settings_t      octaclk  =
    {
        .source_clock  = RESET_VALUE,
        .divider       = RESET_VALUE
    };

    if (SPI_FLASH_PROTOCOL_8D_8D_8D == g_ospi_b_ctrl.spi_protocol)
    {
        /* Do nothing */
    }
    else if (SPI_FLASH_PROTOCOL_EXTENDED_SPI == g_ospi_b_ctrl.spi_protocol)
    {
        /* Transfer write enable command */
        err = ospi_b_write_enable();
        if (err != FSP_SUCCESS)
        {
            return err;
        }

        /* Change to DOPI mode */
        transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_WRITE_CR_00H_SPI];
        transfer.data    = OSPI_B_DATA_SET_OPI_CR_00H;
        err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
        if (err != FSP_SUCCESS)
        {
            return err;
        }

        /* Change the OCTACLK clock to 133 MHz in DDR mode with OM_DQS */
        octaclk.source_clock = BSP_CFG_OCTACLK_SOURCE;
        octaclk.divider      = BSP_CLOCKS_OCTA_CLOCK_DIV_1;
        R_BSP_OctaclkUpdate(&octaclk);

        /* Switch OSPI module mode to OPI mode */
        err = R_OSPI_B_SpiProtocolSet(&g_ospi_b_ctrl, SPI_FLASH_PROTOCOL_8D_8D_8D);
        if (err != FSP_SUCCESS)
        {
            return err;
        }

        /* Read back and verify CR register data */
        transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_CR_00H_OPI];
        err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
        if (err != FSP_SUCCESS)
        {
            return err;
        }

        if (OSPI_B_DATA_SET_OPI_CR_00H != (uint8_t) transfer.data)
        {
            return FSP_ERR_ABORTED;
        }
    }
    else
    {
        return FSP_ERR_INVALID_MODE;
    }
    return err;
}

/**********************************************************************************************************************
 * @brief       This function reads flash device id.
 * @param[out]  *p_device_id        Pointer will be used to store device id.
 * @retval      FSP_SUCCESS         Upon successful direct transfer operation.
 * @retval      FSP_ERR_ABORTED     On incorrect device id read.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t ospi_b_read_device_id (uint32_t * const p_id)
{
    fsp_err_t err = FSP_SUCCESS;
    spi_flash_direct_transfer_t transfer    =
    {
        .command        = RESET_VALUE,
        .address        = RESET_VALUE,
        .data           = RESET_VALUE,
        .command_length = RESET_VALUE,
        .address_length = RESET_VALUE,
        .data_length    = RESET_VALUE,
        .dummy_cycles   = RESET_VALUE
    };

    /* Read and check flash device ID */
    transfer = (SPI_FLASH_PROTOCOL_EXTENDED_SPI == g_ospi_b_ctrl.spi_protocol)
               ? g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_DEVICE_ID_SPI]
               : g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_DEVICE_ID_OPI];
    err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    if (err != FSP_SUCCESS)
    {
        return err;
    }

    if (OSPI_B_DEVICE_ID != transfer.data)
    {
        return FSP_ERR_ABORTED;
    }

    /* Get flash device ID */
    *p_id = transfer.data;
    return err;
}


/*******************************************************************************************************************//**
 * @} (end addtogroup ospi_b_ep.c)
 **********************************************************************************************************************/
