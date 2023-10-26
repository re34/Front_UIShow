#include <rtthread.h>
#include "mod_dac.h"
#include "mod_para.h"

/***********dac 具体芯片使能*****************/
#ifdef RT_USING_AD5541
#include "spi_ad5433.h"
#endif

#ifdef RT_USING_AD9833
#include "spi_ad9833.h"
#endif

static M_DacChipOpr *g_ptDacOprHead = NULL;
/**********************************************************************
 * 函数名称： RegisterDispOpr
 * 功能描述： 注册显示设备
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2020/01/17	     V1.0	  刘显友	      创建
 ***********************************************************************/
/***** check ok *****/
M_DacChipOpr *mod_DacChipRegister(M_DacChipOpr *ptDacChipOpr)
{
	M_DacChipOpr *ptTmp;

	if (!g_ptDacOprHead) {
		g_ptDacOprHead = ptDacChipOpr;
		ptDacChipOpr->ptNext = NULL;
	} else {
		ptTmp = g_ptDacOprHead;
		while (ptTmp->ptNext) {
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext = ptDacChipOpr;
		ptDacChipOpr->ptNext = NULL;
	}
	return g_ptDacOprHead;
}


M_DacChipOpr *GetMatchDacChip(M_DacChipOpr *head, char *pcName)
{	
    M_DacChipOpr *pHead = head;
	while (pHead)
	{
		if (rt_strcmp(pHead->name, pcName) == 0)
		{
			return pHead;
		}
		pHead = pHead->ptNext;
	}
	return NULL;
}

/**********************************************************************
 * 函数名称： DisplayInit
 * 功能描述： 注册显示设备
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/03/15	     V1.0	  刘显友	      创建
 ***********************************************************************/
M_DacChipOpr* DacChipInit(void *argv)
{
	M_DacChipOpr *m_DacChipHead = NULL;
#ifdef RT_USING_AD5541
	m_DacChipHead = HW_Ad5541Register((void *)(((M_DacPara *)argv)->_tScanPara));
#endif
#ifdef RT_USING_AD9833
	M_DacPara *sPara = (M_DacPara *)argv;
	m_DacChipHead = HW_Ad9833Register((void *)&sPara->_tDdsPara);
#endif
    return m_DacChipHead;
}



M_DacChipOpr* DacChipExit(M_DacChipOpr **pHead, char *pcName)
{
	M_DacChipOpr *ptNode = NULL;
    M_DacChipOpr *ptTmp = *pHead;						//让pb指向头节点
    if(*pHead == NULL)  								//链表为空链表
    {
        rt_kprintf("Head of DacChips list is empty\n");
        return NULL;
    }
    while(rt_strcmp(ptTmp->name, pcName)!= 0 && ptTmp->ptNext != NULL)//循环查找要删除的节点
    {
        ptNode = ptTmp;
        ptTmp = ptTmp->ptNext;
    }
    if(rt_strcmp(ptTmp->name, pcName) == 0)//找到了名称相同的节点
    {
        if(ptTmp == *pHead)  //找到的节点是头节点
        {
            *pHead = ptTmp->ptNext;
        }
        else//找到的节点是普通节点
        {
            ptNode->ptNext = ptTmp->ptNext;
        }
        rt_free(ptTmp);
    }
    else
    {
        rt_kprintf("no node find in List\n");
    }
		return *pHead;
}


