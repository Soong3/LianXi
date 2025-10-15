#include "kvstore.h"
#include "reactor.h"
 
extern kvs_array_t global_array;

const char *commend[] = {
	"SET","GET","DEL","MOD","EXIST"
};

enum{
	KVS_CMD_SET = 0,
	KVS_CMD_GET,
	KVS_CMD_DEL,
	KVS_CMD_MOD,
	KVS_CMD_EXIST,
	KVS_CMD_COUT
};


const char *response[] = {

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
    memset(&global_array, 0, sizeof(kvs_array_t));
	kvs_array_create(&global_array);
	return 0;
}

//SET Key Value
//tokens[0] : SET
//tokens[1] : Key
//tokens[2] : Value
int kvs_parser(char *msg, int length, char *response){
    if(msg == NULL || length <= 0 || response == NULL) return -1;
	char *tokens[KVS_MAX_TOKENS_LENGTH] = {0};
	int token_count = kvs_split(msg, tokens);
	if(token_count == -1) return -1;

	int i = 0;
	for(i = 0; i < KVS_CMD_COUT; i++){
	    if(strcmp(tokens[0], commend[i]) == 0){
		    break;
		}
	}
	
	char *key = tokens[1];
	char *value = tokens[2];
	int ret = 0;
	switch(i){
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

}