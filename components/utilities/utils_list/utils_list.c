#include <rtthread.h>
#include "utils_list.h"

M_listObject *list_new(void) 
{
	M_listObject *self = (M_listObject *)rt_malloc(sizeof(M_listObject));
	if(self == RT_NULL)
		return NULL;
	self->head = NULL;
	self->tail = NULL;
	self->len = 0;
	return self;
}

void list_delete(M_listObject *self) 
{
  unsigned int len = self->len;
  M_listNode *next;
  M_listNode *curr = self->head;

  while (len--) {
    next = curr->next;
    if(curr->data) 
		rt_free(curr->data);
    rt_free(curr);
    curr = next;
  }
  rt_free(self);
}

M_listNode *list_rpush(M_listObject *self, M_listNode *node) 
{
  if (!node) 
  	return NULL;

  if (self->len) 
  {
    node->prev = self->tail;
    node->next = NULL;
    self->tail->next = node;
    self->tail = node;
  } else {
    self->head = self->tail = node;
    node->prev = node->next = NULL;
  }
  ++self->len;
  return node;
}


M_listNode *list_node_new(void *data) 
{
	M_listNode *self = (M_listNode *)rt_malloc(sizeof(M_listNode));
	if(self == RT_NULL)
		return NULL;
	self->prev = NULL;
	self->next = NULL;
	self->data = data;
	return self;
}



list_iterator_t *list_iterator_new_from_node(M_listNode *node, e_list_direction direction) 
{
  list_iterator_t *self = (list_iterator_t *)rt_malloc(sizeof(list_iterator_t));
  if(self == RT_NULL)
    return NULL;
  self->next = node;
  self->direction = direction;
  return self;
}

list_iterator_t *list_iterator_new(M_listObject *list, e_list_direction direction) 
{
  M_listNode *node = direction == LIST_HEAD? list->head : list->tail;
  return list_iterator_new_from_node(node, direction);
}


	
/*
 * Return the next list_node_t or NULL when no more
 * nodes remain in the list.
 */

M_listNode *list_iterator_next(list_iterator_t *self) 
{
  M_listNode *curr = self->next;
  if (curr) {
	self->next = self->direction == LIST_HEAD? curr->next : curr->prev;
  }
  return curr;
}

