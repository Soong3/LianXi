#include "kvstore.h"

kvs_array_t global_array = {0};//初始化全局数组

int kvs_array_create(kvs_array_t *array){
    if(array == NULL){
        return -1;
    }

    array->array_list = kvs_malloc(KVS_ARRAY_SIZE * sizeof(kvs_array_item_t));
    array->total = 0;
    return 0;
}

int kvs_array_destroy(kvs_array_t *array){
    if(array == NULL){
        return -1;
    }

    kvs_free(array->array_list);
    array->total = 0;
    return 0;
}

int kvs_array_set(kvs_array_t *array, char *key, char *value){
    if(array == NULL || key == NULL || value == NULL){
        return -1;
    }

    //判断key是否已经存在
    char *str = kvs_array_get(array, key);
    if(str != NULL){
        return 1;//key已经存在
    }

    //将key-value存入数组
    char * tempKey = kvs_malloc(strlen(key) + 1);
    memset(tempKey, 0, strlen(key) + 1);
    strncpy(tempKey, key, strlen(key));

    char * tempValue = kvs_malloc(strlen(value) + 1);
    memset(tempValue, 0, strlen(value) + 1);
    strncpy(tempValue, value, strlen(value));

    for(int i = 0; i < KVS_ARRAY_SIZE; i++){
        if(array->array_list[i].key == NULL){
            array->array_list[i].key = tempKey;
            array->array_list[i].value = tempValue;
            printf("key:%s, value:%s\n", array->array_list[i].key, array->array_list[i].value);
            array->total++;
            return 0;
        }
    }
}

char* kvs_array_get(kvs_array_t *array, char *key){
    if(array == NULL || key == NULL){
        return NULL;
    }

    for(int i = 0; i < array->total; i++){
        if(array->array_list[i].key == NULL){
            continue;//避免前面的被删除后变成NULL  被用来strcmp会出现段错误
        }
        if(strcmp(array->array_list[i].key, key) == 0){
            return array->array_list[i].value;
        }
    }
    return NULL;
}

int kvs_array_delete(kvs_array_t *array, char *key){
    if(array == NULL || key == NULL){
        return -1;
    }

    int i = 0;
    for(i = 0; i < array->total; i++){
        if(array->array_list[i].key == NULL){
            continue;
        }
        if(strcmp(array->array_list[i].key, key) == 0){
            free(array->array_list[i].key);//释放key
            array->array_list[i].key = NULL;

            free(array->array_list[i].value);//释放value
            array->array_list[i].value = NULL;
            return 0;
        }
    }
    return -1;//key不存在
}
int kvs_array_modify(kvs_array_t *array, char *key, char *value){
    if(array == NULL || key == NULL || value == NULL){
        return -1;
    }

    int ret = kvs_array_exists(array, key);
    if(ret < 0){
        return 1;//key不存在
    }

    int i = 0;
    for(i = 0; i < array->total; i++){
        if(array->array_list[i].key == NULL){
            continue;
        }
        if(strcmp(array->array_list[i].key, key) == 0){
            kvs_free(array->array_list[i].value);//释放value
            
            //重新分配value
            char * tempValue = kvs_malloc(strlen(value) + 1);//重新分配value
            memset(tempValue, 0, strlen(value) + 1);
            strncpy(tempValue, value, strlen(value));
            array->array_list[i].value = tempValue;//赋值
            return 0;
        }
    }
    return -1;
}

int kvs_array_exists(kvs_array_t *array, char *key){
    if(array == NULL || key == NULL){
        return -1;
    }

    int i = 0;
    for(i = 0; i < array->total; i++){
        if(array->array_list[i].key == NULL){
            continue;
        }
        if(strcmp(array->array_list[i].key, key) == 0){
            return 0;//key存在
        }
    }
    return -1;//key不存在
}
