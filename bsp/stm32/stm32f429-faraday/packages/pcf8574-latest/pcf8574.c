/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-21     SummerGift   first version
 * 2018-11-22     flybreak     Make the first version of pcf8574's package
 */

#include "pcf8574.h"
#include <rtdevice.h>


#define DBG_ENABLE
#define DBG_SECTION_NAME     "pcf8574"
#define DBG_LEVEL            DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

/***************************基础函数*******************************/
rt_uint8_t pcf8574_port_read(struct rt_touch_device *dev)
{
    rt_uint8_t value;
    rt_device_read(&dev->bus->parent, dev->info.i2c_addr, &value, 1);
    return value;
}

void pcf8574_port_write(struct rt_touch_device *dev, rt_uint8_t value)
{
    rt_device_write(&dev->bus->parent, dev->info.i2c_addr, &value, 1);
}


/***************************************************************************/
rt_err_t pcf8574_control(struct rt_touch_device *dev, int cmd, void *arg)
{
	rt_uint8_t data;
	rt_uint8_t bit = *(rt_uint8_t *)arg;
    switch(cmd)
    {
	    case RT_MUXIO_CTRL_SET_IO:
	    {	    			
				data = pcf8574_port_read(dev);
				data |= 1 << bit;			
				pcf8574_port_write(dev, data);
	        break;
	    }
		case RT_MUXIO_CTRL_CLEAR_IO:
	    {
				data = pcf8574_port_read(dev);
				data &= ~(1 << bit);			
				pcf8574_port_write(dev, data);
	        break;
	    }			
		default:
			return -RT_ERROR;
    }
	return RT_EOK;
}

rt_size_t pcf8574_read_pin(struct rt_touch_device *dev, void *buf, rt_size_t bit)
{
    rt_size_t data;
    data = pcf8574_port_read(dev);
	if(READ_ALL_BIT == bit)
		return data;
	else{
	    if (data & (1 << bit))
	        return 1;
	    else
	        return 0;
	}
}

static struct rt_touch_ops MuxIO_ops =
{
    .touch_readpoint = pcf8574_read_pin,
    .touch_control = pcf8574_control,
};

/***************************************************************************/

int rt_hw_pcf8574_init(const char *dev_name, struct rt_touch_config *cfg, rt_uint8_t i2c_addr)
{
    RT_ASSERT(dev_name);

	rt_uint8_t buffer[] = { 0xFF };
    rt_touch_t muxInput_dev = RT_NULL;
	/*********************
	* 1. 申请输入设备
	*********************/
    muxInput_dev = (rt_touch_t)rt_calloc(1, sizeof(struct rt_touch_device));
    if(muxInput_dev == RT_NULL)
    {
		LOG_E("dev for PCF8574 calloc fail!");
		goto __exit;
	}
	/*********************
	* 2. 查找已注册的i2c总线
	*********************/
    muxInput_dev->bus = (struct rt_i2c_bus_device *)rt_device_find(cfg->dev_name);
    if (muxInput_dev->bus == RT_NULL)
    {
        LOG_E("i2c_bus %s for PCF8574 not found!", cfg->dev_name);
        goto __exit;
    }
	/*********************
	* 3. 填写i2c地址并打开总线
	*********************/	
    if (i2c_addr != RT_NULL)
        muxInput_dev->info.i2c_addr = i2c_addr;
    else
        muxInput_dev->info.i2c_addr = PCF8574_ADDR_DEFAULT;
    if (rt_device_open(&muxInput_dev->bus->parent, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_D("%s bus for PCF8574 opened failed!", cfg->dev_name);
        goto __exit;
    }
	rt_device_write(&muxInput_dev->bus->parent, muxInput_dev->info.i2c_addr, &buffer, 1);

	/*********************
	* 4. 注册输入设备
	*********************/	
    muxInput_dev->info.type = RT_TOUCH_TYPE_MUXIO;
    muxInput_dev->info.vendor = RT_TOUCH_VENDOR_IO;
	/*********************
	* 5. 复制私有数据
	*********************/	
	rt_memcpy(&muxInput_dev->config, cfg, sizeof(struct rt_touch_config));
    muxInput_dev->ops = &MuxIO_ops;
    rt_hw_touch_register(muxInput_dev, dev_name, RT_DEVICE_FLAG_INT_RX, RT_NULL);

    LOG_I("input device pcf8574 init success\n");
	return RT_EOK;
__exit:
    if (muxInput_dev != RT_NULL)
        rt_free(muxInput_dev);
    return RT_ERROR;
}

void rt_hw_pcf8574_deinit(struct rt_touch_device *dev)
{
    RT_ASSERT(dev);
    rt_free(dev);
}
