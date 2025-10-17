#ifndef __KVSTORE_H__
#define __KVSTORE_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define KVS_ARRAY_SIZE 1024
#define KVS_MAX_TOKENS_LENGTH 1024
#define ENABLE_KEY_CHAR 1


#if ENABLE_KEY_CHAR
	typedef char* KEY_TYPE;
#else
	typedef int KEY_TYPE;
#endif


void *kvs_malloc(size_t size);
void kvs_free(void *ptr);

int kvs_protocol(char *msg, int length, char *response);

//红黑树存储引擎
//定义红黑树节点结构
typedef struct _rbtree_node {
	unsigned char color;
	struct _rbtree_node *right;
	struct _rbtree_node *left;
	struct _rbtree_node *parent;
	KEY_TYPE key;
	void *value;
} rbtree_node;

//定义红黑树结构
typedef struct _rbtree {
	rbtree_node *root;
	rbtree_node *nil;
} rbtree;

//数组存储引擎
//定义数组元素结构
typedef struct kvs_array_item_s{
    char *key;
    char *value;
}kvs_array_item_t;

//定义数组结构
typedef struct kvs_array_s{
    kvs_array_item_t *array_list;
    int total;//数组大小
}kvs_array_t;

//array.c实现的函数
int kvs_array_create(kvs_array_t *array);
void kvs_array_destroy(kvs_array_t *array);
int kvs_array_set(kvs_array_t *array, char *key, char *value);
char *kvs_array_get(kvs_array_t *array, char *key);
int kvs_array_delete(kvs_array_t *array, char *key);
int kvs_array_modify(kvs_array_t *array, char *key, char *value);
int kvs_array_exists(kvs_array_t *array, char *key);

//rbtree.c实现的函数
int kvs_rbtree_create(rbtree *tree);
void kvs_rbtree_destroy(rbtree *tree);
int kvs_rbtree_set(rbtree *tree, KEY_TYPE key, KEY_TYPE value);
char *kvs_rbtree_get(rbtree *tree, KEY_TYPE key);
int kvs_rbtree_delete(rbtree *tree, KEY_TYPE key);
int kvs_rbtree_modify(rbtree *tree, KEY_TYPE key, KEY_TYPE value);
int kvs_rbtree_exists(rbtree *tree, KEY_TYPE key);
#endif
