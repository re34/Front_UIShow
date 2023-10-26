/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-27     SummerGift   add spi flash port file
 */

#include <rtthread.h>
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include "drv_spi.h"

#include "./sfud/inc/sfud.h"

#if defined(BSP_USING_SPI_FLASH)

#define W25Q_SPI_DEVICE_NAME     "spi50"

static void spi_w25q_sample(int argc, char *argv[])
{
    struct rt_spi_device *spi_dev_w25q;
    char name[RT_NAME_MAX];
    rt_uint8_t w25x_read_id = 0x90;
    rt_uint8_t id[5] = {0};

    if (argc == 2)
    {
        rt_strncpy(name, argv[1], RT_NAME_MAX);
    }
    else
    {
        rt_strncpy(name, W25Q_SPI_DEVICE_NAME, RT_NAME_MAX);
    }
    /* 查找 spi 设备获取设备句柄 */
    spi_dev_w25q = (struct rt_spi_device *)rt_device_find(name);
    if (!spi_dev_w25q)
    {
        rt_kprintf("spi sample run failed! can't find %s device!\n", name);
    }
    else
    {
        /* 方式1：使用 rt_spi_send_then_recv()发送命令读取ID */
        rt_spi_send_then_recv(spi_dev_w25q, &w25x_read_id, 1, id, 5);
        rt_kprintf("use rt_spi_send_then_recv() read w25q ID is:%x%x\n", id[3], id[4]);

        /* 方式2：使用 rt_spi_transfer_message()发送命令读取ID */
        struct rt_spi_message msg1, msg2;

        msg1.send_buf   = &w25x_read_id;
        msg1.recv_buf   = RT_NULL;
        msg1.length     = 1;
        msg1.cs_take    = 1;
        msg1.cs_release = 0;
        msg1.next       = &msg2;

        msg2.send_buf   = RT_NULL;
        msg2.recv_buf   = id;
        msg2.length     = 5;
        msg2.cs_take    = 0;
        msg2.cs_release = 1;
        msg2.next       = RT_NULL;

        rt_spi_transfer_message(spi_dev_w25q, &msg1);
        rt_kprintf("use rt_spi_transfer_message() read w25q ID is:%x%x\n", id[3], id[4]);
    }
}

MSH_CMD_EXPORT(spi_w25q_sample, spi w25q sample);


static void spi_w25q_sf_sample(int argc, char *argv[])
{
    uint8_t *read_data;  // 读取到的数据
    uint8_t *write_data; // 将要写入的数据
    sfud_flash *sfud_dev = NULL;
    sfud_err ret;
    sfud_dev = rt_sfud_flash_find(W25Q_SPI_DEVICE_NAME); // 获取 sfud_dev
    // 或者 sfud_dev = rt_sfud_flash_find_by_dev_name("W25Q_SPI_DEVICE_NAME");

    write_data = rt_malloc(32);
    rt_memset(write_data, '1', 32);
    ret = sfud_erase_write(sfud_dev, 0, 32, write_data); // 将数据 32 字节的 write_data 从 0 开始写入 flash
    if(ret == SFUD_SUCCESS)
    {
        rt_kprintf("sfud write data at 0 is:%s\n", write_data);
    }
    else
    {
        rt_kprintf("sfud write data failed\n");
    }
    read_data = rt_malloc(32);
    ret = sfud_read(sfud_dev, 0, 32, read_data);   // 读取从 0 开始的 32 字节，存入 read_data
    if(ret == SFUD_SUCCESS)
    {
        rt_kprintf("sfud read data at 0 is:%s\n", read_data);
    }
    else
    {
        rt_kprintf("sfud read data failed\n");
    }
	rt_free(write_data);
	rt_free(read_data);	
}
MSH_CMD_EXPORT(spi_w25q_sf_sample, spi w25q sf sample);




static int rt_hw_spi_flash_init(void)
{
    __HAL_RCC_GPIOF_CLK_ENABLE();
    rt_hw_spi_device_attach("spi5", W25Q_SPI_DEVICE_NAME, GPIOF, GPIO_PIN_6);

    if (RT_NULL == rt_sfud_flash_probe("W25Q256", W25Q_SPI_DEVICE_NAME))
    {
        return -RT_ERROR;
    };

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_spi_flash_init);



#endif

