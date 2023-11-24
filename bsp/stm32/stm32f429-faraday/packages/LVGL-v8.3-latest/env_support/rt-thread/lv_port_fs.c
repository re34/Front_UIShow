#include "lvgl.h"

#include <rtthread.h>


#if LV_USE_RTT_DFS 
#include "dfs_posix.h"
#include "lv_port_fs.h"


static int file_fd;
static lv_fs_drv_t fs_drv;

static void fs_init(void)
{
   //rtthread 已初始化完毕
}

//目前只支持只读
static void *fs_open (lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
    rt_size_t len = rt_strlen(path);
    RT_ASSERT(len > 2 && len < DFS_PATH_MAX + 2);

    if(mode == LV_FS_MODE_RD)
    {
        if ((file_fd = open(path, O_RDONLY)) > 0) 
		{
          	return (void *)&file_fd;
        } else {
            return NULL;
        }
    }
	return NULL;
}


static lv_fs_res_t fs_close (lv_fs_drv_t * drv, void * file_p)
{
    lv_fs_res_t res = LV_FS_RES_UNKNOWN;

    int fd = *(int *)file_p;
    if (close(fd) == 0)
        res = LV_FS_RES_OK;
    return res;
}

static lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    lv_fs_res_t res = LV_FS_RES_UNKNOWN;

    int fd = *(int *)file_p;
    int read_bytes = read(fd, buf, btr);
    if (read_bytes >= 0) {
        *br = read_bytes;
        res = LV_FS_RES_OK;
    } 
    return res;
}


static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
    return LV_FS_RES_NOT_IMP;
}


static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    lv_fs_res_t res = LV_FS_RES_UNKNOWN;

    int fd = *(int *)file_p;
    if (lseek(fd, pos, whence) >= 0)
        res = LV_FS_RES_OK;
    return res;

}

static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    lv_fs_res_t res = LV_FS_RES_UNKNOWN;

    int fd = *(int *)file_p;
    off_t pos = lseek(fd, 0, SEEK_CUR);
    if (pos >= 0) {
        *pos_p = pos;
        res = LV_FS_RES_OK;
    }
    return res;
}

void lv_port_fs_init(void)
{
    fs_init();
    
    lv_fs_drv_init(&fs_drv);

    fs_drv.letter = 'L';		//设置盘符号
    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;

    lv_fs_drv_register(&fs_drv);
}
#endif
