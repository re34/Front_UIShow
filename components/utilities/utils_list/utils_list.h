#ifndef __UTILS_LIST_H
#define __UTILS_LIST_H



typedef enum {
    LIST_HEAD, 
	LIST_TAIL
} e_list_direction;

typedef struct _listNode 
{
  struct _listNode *prev;
  struct _listNode *next;
  void *data;
} M_listNode;



typedef struct _listObject 
{
  M_listNode *head;
  M_listNode *tail;
  unsigned int len;
} M_listObject;

typedef struct {
  M_listNode *next;
  e_list_direction direction;
} list_iterator_t;



extern M_listObject *list_new(void);
extern void list_delete(M_listObject *self);
extern M_listNode *list_rpush(M_listObject *self, M_listNode *node);
extern M_listNode *list_node_new(void *data);
extern list_iterator_t *list_iterator_new(M_listObject *list, e_list_direction direction);
extern M_listNode *list_iterator_next(list_iterator_t *self);


#endif

