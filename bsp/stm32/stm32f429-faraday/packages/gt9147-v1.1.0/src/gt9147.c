/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-06-01     tyustli     the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include <string.h>
#include <stdlib.h>

#define DBG_TAG "gt9147"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#include "touch.h"
#include "gt9147.h"

#define USING_POWERUP_CFG

#if 1

static rt_uint8_t GT9147_CFG_TBL[] =
{
	0x61,0xE1,0x01,0x11,0x01,0x05,0x3C,0x00,0x01,0x08,
	0x28,0x0F,0x50,0x32,0x03,0x09,0x00,0x00,0x00,0x00,
	0x22,0x22,0x06,0x18,0x1A,0x1F,0x14,0x89,0x28,0x0A,
	0x52,0x54,0x27,0x06,0x00,0x00,0x01,0x22,0x02,0x1D,
	0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x46,0x78,0x94,0xE5,0x02,0x07,0x00,0x00,0x04,
	0x8D,0x49,0x00,0x80,0x52,0x00,0x75,0x5B,0x00,0x6A,
	0x66,0x00,0x62,0x71,0x00,0x62,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,0x16,
	0x18,0x1A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x02,0x04,0x05,0x06,0x08,0x0A,0x0C,
	0x0E,0x1D,0x1E,0x1F,0x20,0x22,0x24,0x28,0x29,0xFF,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00
};
#else
//GT9147配置参数表
//第一个字节为版本号(0X60),必须保证新的版本号大于等于GT9147内部
//flash原有版本号,才会更新配置.
static uint8_t GT9147_CFG_TBL[] = {
    0x61, 0xE0, 0x01, 0x10, 0x01, 0x05, 0x0C, 0x00, 0x0F, 0x08,
    0x28, 0x05, 0x32, 0x0F, 0x03, 0x05, 0x00, 0x00, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x89, 0x28, 0x0A,
    0x17, 0x15, 0x31, 0x0D, 0x00, 0x00, 0x02, 0x9B, 0x03, 0x25,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00,
    0x00, 0x0F, 0x94, 0x94, 0xC5, 0x02, 0x07, 0x00, 0x00, 0x04,
    0x8D, 0x13, 0x00, 0x5C, 0x1E, 0x00, 0x3C, 0x30, 0x00, 0x29,
    0x4C, 0x00, 0x1E, 0x78, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16,
    0x18, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0x02, 0x04, 0x05, 0x06, 0x08, 0x0A, 0x0C,
    0x0E, 0x1D, 0x1E, 0x1F, 0x20, 0x22, 0x24, 0x28, 0x29, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF
};
#endif


static rt_err_t gt9147_write_reg(struct rt_touch_device *client, rt_uint16_t reg, rt_uint8_t *write_data, rt_uint8_t write_len)
{
	rt_uint8_t buf[256];
    struct rt_i2c_msg msgs;

	buf[0] = reg >> 8;							/* 寄存器首地址低8位 */
    buf[1] = reg & 0XFF;						/* 寄存器首地址高8位 */
	memcpy(&buf[2], write_data, write_len);		/* 将要写入的数据拷贝到数组buf里面 */

    msgs.addr  = client->info.i2c_addr;
    msgs.flags = RT_I2C_WR;
    msgs.buf   = buf;
    msgs.len   = write_len + 2;
    if (rt_i2c_transfer(client->bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}


static rt_err_t gt9147_read_regs(struct rt_touch_device *client, rt_uint16_t reg, rt_uint8_t *read_buf, rt_uint8_t read_len)
{
    struct rt_i2c_msg msgs[2];
	rt_uint8_t cmd_buf[2];

	cmd_buf[0] = reg >> 8;
	cmd_buf[1] = reg & 0xff;	

	msgs[0].addr  = client->info.i2c_addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &cmd_buf[0];
    msgs[0].len   = 2;

    msgs[1].addr  = client->info.i2c_addr;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = read_buf;
    msgs[1].len   = read_len;
    if (rt_i2c_transfer(client->bus, msgs, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}


static rt_err_t gt9147_get_product_id(struct rt_touch_device *dev, rt_uint8_t *read_data, rt_uint8_t read_len)
{
    if (gt9147_read_regs(dev, GT9XX_PRODUCT_ID, read_data, read_len) != RT_EOK)
    {
        LOG_D("read id failed \n");
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t gt9147_get_info(struct rt_touch_device *dev, struct rt_touch_info *info)
{
    rt_uint8_t opr_buf[7] = {0};
    if (gt9147_read_regs(dev, GT9147_CONFIG, opr_buf, 7) != RT_EOK)
    {
        LOG_D("read id failed \n");
        return -RT_ERROR;
    }
    info->range_x = (opr_buf[2] << 8) + opr_buf[1];
    info->range_y = (opr_buf[4] << 8) + opr_buf[3];
    info->point_num = opr_buf[5] & 0x0f;
    return RT_EOK;
}

static rt_err_t gt9147_soft_reset(struct rt_touch_device *dev, rt_uint8_t enable)
{
    rt_uint8_t data;
	if(enable)
    	data = 0x02;
	else
		data = 0;
    if (gt9147_write_reg(dev, GT9147_COMMAND, &data, 1) != RT_EOK)
    {
        LOG_D("soft reset gt9147 failed\n");
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t gt9147_control(struct rt_touch_device *device, int cmd, void *data)
{

    if (cmd == RT_TOUCH_CTRL_GET_ID)
    {
        return gt9147_get_product_id(device, data, 6);
    }
    if (cmd == RT_TOUCH_CTRL_GET_INFO)
    {
        return gt9147_get_info(device, data);
    }
    if (cmd == RT_TOUCH_CTRL_CLEAR)
    {
		rt_uint8_t  data = 0;
		gt9147_write_reg(device, GT9147_READ_STATUS, &data, 1);
    }
    return RT_EOK;
}


static rt_uint8_t s_tp_dowm[GT9147_MAX_TOUCH];
static struct rt_touch_data *read_data;


static void gt9147_touch_up(void *buf, int8_t id)
{
    read_data = (struct rt_touch_data *)buf;

    if(s_tp_dowm[id] == 1)
    {
        s_tp_dowm[id] = 0;
        read_data[id].event = RT_TOUCH_EVENT_UP;
    }
    else
    {
        read_data[id].event = RT_TOUCH_EVENT_NONE;
    }
}

static void gt9147_touch_down(void *buf, int8_t id, int16_t x, int16_t y)
{
    read_data = (struct rt_touch_data *)buf;

    if (s_tp_dowm[id] == 1)
    {
        read_data[id].event = RT_TOUCH_EVENT_MOVE;
    }
    else
    {
        read_data[id].event = RT_TOUCH_EVENT_DOWN;
        s_tp_dowm[id] = 1;
    }
    read_data[id].x_coordinate = x;
    read_data[id].y_coordinate = y;
    read_data[id].track_id = id;
}




static rt_size_t gt9147_read_point(struct rt_touch_device *touch, void *buf, rt_size_t read_num)
{
    rt_uint8_t point_status = 0;
    rt_uint8_t touch_num = 0;
    rt_uint8_t write_buf[1];
    rt_uint8_t read_buf[8 * GT9147_MAX_TOUCH] = {0};
    int8_t read_id = 0;
    int16_t input_x = 0;
    int16_t input_y = 0;

    if (gt9147_read_regs(touch, GT9147_READ_STATUS, &point_status, 1) != RT_EOK)
    {
        LOG_D("read point failed\n");
        read_num = 0;
        goto exit_;
    }
    if (point_status == 0 || (point_status & 0x80) == 0)             /* no data */
    {
        read_num = 0;
        goto exit_;
    }
    touch_num = point_status & 0x0f;  /* get point num */
    if (touch_num > GT9147_MAX_TOUCH) /* point num is not correct */
    {
        read_num = 0;
        goto exit_;
    }
	
    if (gt9147_read_regs(touch, GT9147_POINT1_REG, read_buf, read_num * GT9147_POINT_INFO_NUM) != RT_EOK)
    {
        LOG_D("read point failed\n");
        read_num = 0;
		goto exit_;
    }
    if(touch_num)                                                 /* point down */
    {
        read_id = read_buf[0] & 0x0f;
		if(read_id == 0)
		{
			input_x = read_buf[1] | (read_buf[2] << 8); /* x */
			input_y = read_buf[3] | (read_buf[4] << 8); /* y */ 	
			gt9147_touch_down(buf, read_id, input_x, input_y);
		}
    }
    else if(touch_num == 0)
    {
    	gt9147_touch_up(buf, 0);
    }
exit_:
	write_buf[0] = 0;
	gt9147_write_reg(touch, GT9147_READ_STATUS, write_buf, 1);
    return read_num;
}


static struct rt_touch_ops touch_ops =
{
    .touch_readpoint = gt9147_read_point,
    .touch_control = gt9147_control,
};

#ifdef USING_POWERUP_CFG
static void gt9147_Set_Config(struct rt_touch_device *dev, rt_uint8_t mode)
{
	unsigned char buf[2];
	unsigned int i = 0;

	buf[0] = 0;
	buf[1] = mode;
	for(i = 0; i < (sizeof(GT9147_CFG_TBL)); i++) 
        buf[0] += GT9147_CFG_TBL[i];            
	buf[0] = (~buf[0]) + 1;
	gt9147_write_reg(dev, GT9147_CONFIG, GT9147_CFG_TBL, sizeof(GT9147_CFG_TBL));
	gt9147_write_reg(dev, GT9147_CHECK_SUM, buf, 2);
}
#endif



int rt_hw_gt9147_init(const char *name, struct rt_touch_config *cfg)
{
    rt_touch_t touch_device = RT_NULL;

    touch_device = (rt_touch_t)rt_calloc(1, sizeof(struct rt_touch_device));

    if (touch_device == RT_NULL)
        return -RT_ERROR;
	
    /* 复位脚输出 */
    rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
    rt_pin_mode(cfg->irq_pin.pin, PIN_MODE_OUTPUT);
	rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_LOW);
	rt_pin_write(cfg->irq_pin.pin, PIN_LOW);	
#if (DEV_I2C_ADDR == GT9147_ADDRESS_LOW)
	rt_thread_mdelay(10);  
	rt_pin_write(cfg->irq_pin.pin, PIN_HIGH);
#endif
	rt_thread_mdelay(10);
    rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_HIGH);
    rt_thread_mdelay(10);
	//中断脚配置
    rt_pin_mode(cfg->irq_pin.pin, PIN_MODE_INPUT);
    rt_thread_mdelay(100);

    touch_device->bus = (struct rt_i2c_bus_device *)rt_device_find(cfg->dev_name);
    if (touch_device->bus == RT_NULL)
    {
        LOG_E("Can't find device\n");
        return -RT_ERROR;
    }
    if (rt_device_open((rt_device_t)touch_device->bus, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("open device failed\n");
        return -RT_ERROR;
    }
	/*********************************************
	* 必须有完整的软复位，否则初始化失败
	**********************************************/
#if (DEV_I2C_ADDR == GT9147_ADDRESS_LOW)		
    touch_device->info.i2c_addr = GT9147_ADDRESS_LOW;
#else
    touch_device->info.i2c_addr = GT9147_ADDRESS_HIGH;
#endif
		
    gt9147_soft_reset(touch_device, 1);
	rt_thread_mdelay(100);
    gt9147_soft_reset(touch_device, 0);

    /* register touch device */
    touch_device->info.type = RT_TOUCH_TYPE_CAPACITANCE;
    touch_device->info.vendor = RT_TOUCH_VENDOR_GT;
    rt_memcpy(&touch_device->config, cfg, sizeof(struct rt_touch_config));
    touch_device->ops = &touch_ops;
    rt_hw_touch_register(touch_device, name, RT_DEVICE_FLAG_INT_RX, RT_NULL);	
#ifdef USING_POWERUP_CFG
	gt9147_Set_Config(touch_device, 1);
	rt_uint8_t all_buf[186] = {0};
	gt9147_read_regs(touch_device, GT9147_CONFIG, all_buf, 186);
	for(uint8_t i = 0; i < 186; i++)
	{
		rt_kprintf("0x%02X,",all_buf[i]);
		if(i % 10 == 0)
			rt_kprintf("\n");
	}
#endif
    return RT_EOK;
}
/************************** end of file ********************************/


