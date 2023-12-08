#ifndef __APPLICATION_H
#define __APPLICATION_H

#include "mod_dac.h"
#include "mod_input.h"
#include "utils_list.h"


enum {
	CLASS_DISP = 0,
	CLASS_CHIPDAC,
	CLASS_CHIPADC,	
	CLASS_TRANS,
	CLASS_INPUT,
	CLASS_MAX,
};

typedef struct {
   M_DacChipOpr		*mod_dac;	
   M_listObject		*mod_InputList;	
}MOD_INSERT;

typedef struct _LaserCore
{
    void (*new_object)(MOD_INSERT* core, int classType, uint8_t id, void *argv);
    void (*del_object)(MOD_INSERT* core, int mode, char *pcName);
}T_LaserCore;


extern MOD_INSERT g_tLaserManager;
extern T_LaserCore MainCore;

#endif
