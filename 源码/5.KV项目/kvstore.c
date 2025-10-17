#include "kvstore.h"
#include "reactor.h"

#define ENABLE_ARRAY 0
#define ENABLE_RBTREE 1

#if ENABLE_ARRAY
extern kvs_array_t global_array;
#endif

#if ENABLE_RBTREE
extern rbtree global_tree;
#endif

const char *commend[] = {

#if ENABLE_ARRAY
	"SET","GET","DEL","MOD","EXIST",
#endif
	"RSET", "RGET", "RDEL", "RMOD", "REXIST"
};

enum{
	KVS_CMD_START = 0,
#if ENABLE_ARRAY
	KVS_CMD_SET = KVS_CMD_START,
	KVS_CMD_GET,
	KVS_CMD_DEL,
	KVS_CMD_MOD,
	KVS_CMD_EXIST,
#endif
	KVS_CMD_RSET = KVS_CMD_START,
	KVS_CMD_RGET,
	KVS_CMD_RDEL,
	KVS_CMD_RMOD,
	KVS_CMD_REXIST,
	KVS_CMD_COUT,
};


char *response[] = {

};

void *kvs_malloc(size_t size) { //对系统调用封装 便于修改
	return malloc(size);
}
 
void kvs_free(void *ptr) {
	return free(ptr);
}

//将字符串分割成多个token
int kvs_split(char *msg,char *tokens[]){
	if(msg == NULL || tokens == NULL) return -1;
	int index = 0;
	char *token = strtok(msg," ");
	while(token != NULL){
		tokens[index++] = token;//将token存入tokens数组
		token = strtok(NULL," ");//继续获取下一个token
	}
	return index;//返回token的个数
}

//启动kvstore引擎
int init_kvengine(void){

#if ENABLE_ARRAY
    memset(&global_array, 0, sizeof(kvs_array_t));
	kvs_array_create(&global_array);
#endif

#if ENABLE_RBTREE
	memset(&global_tree, 0, sizeof(rbtree));
	kvs_rbtree_create(&global_tree);
#endif
	return 0;
}

//SET Key Value
//tokens[0] : SET
//tokens[1] : Key
//tokens[2] : Value
int kvs_parser(char *msg, int length, char *response){
    if(msg == NULL || length <= 0 || response == NULL) return -1;
	//printf("[DEBUG] msg='%s', length=%d\n", msg, length);  // 检查输入是否合法
	char *tokens[KVS_MAX_TOKENS_LENGTH] = {0};
	int token_count = kvs_split(msg, tokens);
	if(token_count == -1) return -1;
	//int token_count = kvs_split(msg, tokens);
	if (token_count <= 0) {
    	sprintf(response, "ERROR: Failed to parse command\r\n");
    return -1;
	}
	printf("[DEBUG] First token: %s\n", tokens[0]);  // 检查 tokens[0] 是否非空

	int i = 0;
	for(i = 0; i < KVS_CMD_COUT; i++){
	    if(strcmp(tokens[0], commend[i]) == 0){
		    break;
		}
	}
	
	if(i == KVS_CMD_COUT){
        perror("cmd error");
        return -2;
    }

	char *key = tokens[1];
	char *value = tokens[2];
	int ret = 0;
	printf("[DEBUG] Secend token: %s\n", tokens[1]);
	printf("[DEBUG] Command index: %d\n", i);
	switch(i){
#if ENABLE_ARRAY
	    case KVS_CMD_SET:
			ret = kvs_array_set(&global_array, key, value);
			if(ret < 0){
				sprintf(response,"ERROR\r\n");
			}
			else if(ret == 0){
				sprintf(response,"OK\r\n");
			}else{
				sprintf(response,"EXISTS\r\n");
			}
			break;
		case KVS_CMD_GET:{	//标签后面申明变量需要加括号
			char *str = kvs_array_get(&global_array, key);
			if(str == NULL){
			    sprintf(response,"NO EXITE\r\n");
			}else{
			    sprintf(response,"%s\r\n",str);
			}
			break;
		}
		case KVS_CMD_DEL:
			ret = kvs_array_delete(&global_array, key);
			if(ret < 0){
				sprintf(response,"ERROR\r\n");
			}else if(ret == 0){
				sprintf(response,"OK\r\n");
			}else{
				sprintf(response,"NO EXITE\r\n");
			}
			break;
		case KVS_CMD_MOD:
			ret = kvs_array_modify(&global_array, key, value);
			if(ret < 0){
				sprintf(response,"ERROR\r\n");
			}else if(ret == 0){
			    sprintf(response,"OK\r\n");
			}else{
				sprintf(response,"NO EXITE\r\n");
			}
			break;
		case KVS_CMD_EXIST:
			ret = kvs_array_exists(&global_array, key);
			if(ret == 0){
				sprintf(response,"SUCCESS\r\n");
			}else{
				sprintf(response,"MO EXITE\r\n");
			}
			break;
	}
#endif

#if ENABLE_RBTREE
	    case KVS_CMD_RSET:
		printf("[DEBUG] Third token: %s\n", tokens[2]);
		ret = kvs_rbtree_set(&global_tree ,key,value);
		if (ret < 0) {
			sprintf(response,"ERROR\r\n");
		} else if (ret == 0) {
			printf("[DEBUG] Set key %s to %s\n",key,value);
			sprintf(response,"OK\r\n");
		} else {
			sprintf(response,"EXIST\r\n");
		} 
		
		break;
	case KVS_CMD_RGET: {
		char *result = kvs_rbtree_get(&global_tree, key);
		if (result == NULL) {
			sprintf(response,"NO EXIST\r\n");
		} else {
			sprintf(response,"%s\r\n",result);
		}
		break;
	}
	case KVS_CMD_RDEL:
		ret = kvs_rbtree_delete(&global_tree ,key);
		if (ret < 0) {
			sprintf(response,"ERROR\r\n");
 		} else if (ret == 0) {
			sprintf(response,"OK\r\n");
		} else {
			sprintf(response,"NO EXIST\r\n");
		}
		break;
	case KVS_CMD_RMOD:
		ret = kvs_rbtree_modify(&global_tree ,key, value);
		if (ret < 0) {
			sprintf(response,"ERROR\r\n");
 		} else if (ret == 0) {
			sprintf(response,"OK\r\n");
		} else {
			sprintf(response,"NO EXIST\r\n");
		}
		break;
	case KVS_CMD_REXIST:
		ret = kvs_rbtree_exists(&global_tree ,key);
		if (ret == 0) {
			sprintf(response,"EXIST\r\n");
		} else if (ret == 1){
			sprintf(response,"NO EXIST\r\n");
		}
        else {
            sprintf(response,"error\r\n");
        }
		break;
	
	
#endif
	}	
    return strlen(response);  
}

/*
//定义一个kvs协议，传给网络层。
int kvs_protocol(char *msg, int length, char *response){
 
	if(msg == NULL || length <= 0 || response == NULL) return -1;
	printf("recv %d : %s\n", length, msg);
	memcpy(response, msg, length);//
 
 
	return strlen(response);
}
*/

//port kvs_protocol
int main(int argc, char *argv[]){
 
	if(argc != 2) return -1;
 
	int port = atoi(argv[1]);

    init_kvengine();
	reactor_start(port, kvs_parser);
	return 0;
}