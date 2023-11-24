/*********************
 *      INCLUDES
 *********************/
#include "drv_lcd.h"
#include "lv_port_disp.h"
#include <stdbool.h>


static lv_disp_drv_t disp_drv;
static lv_disp_draw_buf_t disp_buf;

extern DMA2D_HandleTypeDef hdma2d;
extern LTDC_HandleTypeDef LtdcHandle;

static lv_disp_drv_t *disp_drv_p;

#ifdef RT_USING_MEMHEAP_AS_HEAP
extern struct rt_memheap sdram_heap;
#endif





#ifndef RT_USING_LTDC_DOUBLE_BUF
static void lcd_fb_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
#if 0
	/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
	int16_t w = (area->x2 - area->x1 + 1);
	int16_t h = (area->y2 - area->y1 + 1);
	disp_drv_p = disp_drv;

	hdma2d.Init.Mode 		= DMA2D_M2M;
	hdma2d.Init.ColorMode	= DMA2D_OUTPUT_RGB565;
	hdma2d.Init.OutputOffset = (480 - w);	  
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputAlpha = 0xFF;
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565 ;
	hdma2d.LayerCfg[1].InputOffset = 0;
	hdma2d.Instance = DMA2D; 
	HAL_DMA2D_Init(&hdma2d);
	HAL_DMA2D_ConfigLayer(&hdma2d, 1);	
	//用dma通道将数据往当前FBStartAdress搬数据，LTDC总线显示数据
	HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)color_p, (uint32_t)(LtdcHandle.LayerCfg[0].FBStartAdress) + sizeof(lv_color_t) *(480 * area->y1 + area->x1), w, h);
#else
	int16_t w = (area->x2 - area->x1 + 1);
	int16_t h = (area->y2 - area->y1 + 1);

	disp_drv_p = disp_drv;
	uint32_t addr = (uint32_t)(LtdcHandle.LayerCfg[0].FBStartAdress) + sizeof(lv_color_t) *(USER_LCD_WITH * area->y1 + area->x1);

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2DEN;
	DMA2D->CR &= ~(1<<0); 								//先停止DMA2D
	DMA2D->CR = 0 << 16;								//存储器到存储器模式
	DMA2D->FGPFCCR = DMA2D_OUTPUT_RGB565;				//设置颜色格式
	DMA2D->OPFCCR = DMA2D_OUTPUT_RGB565;
	DMA2D->FGOR = 0;									//前景层行偏移为0
	DMA2D->OOR = USER_LCD_WITH - w;						//设置行偏移
	DMA2D->FGMAR = (uint32_t)(uint16_t *)color_p;		//源地址
	DMA2D->OMAR = addr;									//目标地址
	DMA2D->NLR = (h)|(w << 16);							//设定行数寄存器
	DMA2D->CR |= DMA2D_CR_TCIE; 						//传输完成中断使能
	DMA2D->CR |= DMA2D_CR_START;						//启动DMA2D
#endif
	
}

#else
static struct drv_lcd_device *p_lcdDev;

static void lcd_fb_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
	if(RT_EOK == rt_sem_trytake(&p_lcdDev->lcd_dmaSet))
	{
		uint32_t Address;
		if(p_lcdDev->cur_buf)
			Address = (uint32_t)p_lcdDev->front_buf + sizeof(lv_color_t) * (USER_LCD_WITH * area->y1 + area->x1);
		else
			Address = (uint32_t)p_lcdDev->back_buf + sizeof(lv_color_t) *(USER_LCD_WITH * area->y1 + area->x1);
		int16_t w = (area->x2 - area->x1 + 1);
		int16_t h = (area->y2 - area->y1 + 1);
		disp_drv_p = disp_drv;

		hdma2d.Init.Mode 		= DMA2D_M2M;
		hdma2d.Init.ColorMode	= DMA2D_OUTPUT_RGB565;
		hdma2d.Init.OutputOffset = (480 - w);	  
		hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
		hdma2d.LayerCfg[1].InputAlpha = 0xFF;
		hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565 ;
		hdma2d.LayerCfg[1].InputOffset = 0;
		hdma2d.Instance = DMA2D; 
		HAL_DMA2D_Init(&hdma2d);
		HAL_DMA2D_ConfigLayer(&hdma2d, 1);	
		//用dma通道将数据往当前FBStartAdress搬数据，LTDC总线显示数据
		HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)color_p, Address, w, h);			
	}
}
#endif

void DMA2D_IRQHandler(void)
{
	rt_enter_critical();
	lv_disp_flush_ready(disp_drv_p);
	HAL_DMA2D_IRQHandler(&hdma2d);
	rt_exit_critical();
}

void lv_port_disp_init(void)
{
	uint32_t devSize = USER_LCD_WITH * USER_LCD_HEIGHT;
#ifdef RT_USING_LTDC_DOUBLE_BUF
	rt_device_t s_lcd = RT_NULL;
	s_lcd = rt_device_find("lcd");
    if (s_lcd == RT_NULL)
    {
        rt_kprintf("can't find device\n");
        return;
    }
	p_lcdDev = (struct drv_lcd_device *)s_lcd->user_data;
#endif
	/**************************************
	*  2. 设置单缓存（半屏或全屏, 默认全屏）
	***************************************/
    void *lcdbuf_1 = (lv_color_t *)rt_memheap_alloc(&sdram_heap, devSize * sizeof(lv_color_t));
	rt_memset(lcdbuf_1, 0, devSize * sizeof(lv_color_t));
	/***************************
	*  3. LCD设备基础初始化
	****************************/
	lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = USER_LCD_WITH;
    disp_drv.ver_res = USER_LCD_HEIGHT;
	/***************************
	*  4. LCD刷新回调
	****************************/
	disp_drv.flush_cb = lcd_fb_flush;
	lv_disp_draw_buf_init(&disp_buf, lcdbuf_1, NULL, devSize);
	disp_drv.draw_buf = &disp_buf;
	lv_disp_drv_register(&disp_drv);

}

