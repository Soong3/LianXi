#ifndef __KVSTORE_H__
#define __KVSTORE_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define KVS_ARRAY_SIZE 1024
#define KVS_MAX_TOKENS_LENGTH 1024
int kvs_protocol(char *msg, int length, char *response);

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
int kvs_array_destroy(kvs_array_t *array);
int kvs_array_set(kvs_array_t *array, char *key, char *value);
char *kvs_array_get(kvs_array_t *array, char *key);
int kvs_array_delete(kvs_array_t *array, char *key);
int kvs_array_modify(kvs_array_t *array, char *key, char *value);
int kvs_array_exists(kvs_array_t *array, char *key);

void *kvs_malloc(size_t size);
void kvs_free(void *ptr);
#endif
