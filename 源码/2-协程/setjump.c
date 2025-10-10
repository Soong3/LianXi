#include<stdio.h>
#include<setjmp.h>
#include<stdlib.h>
#include<ucontext.h>
#include<unistd.h>

ucontext_t ctx[2];//定义两个上下文
ucontext_t main_ctx;//定义一个上下文
/*
setjmp例子:
jmp_buf env;//定义一个全局变量env，用于保存跳转信息
void func(int arg){
    printf("func: %d\n",arg);
    longjmp(env,++arg);//跳转到main函数中setjmp的位置
}
*/
int cout = 0;
void func1(void){
    while(cout ++ < 5){
        printf("1\n");
        swapcontext(&ctx[0],&ctx[1]);//切换到ctx[1]上下文
        printf("4\n");
    }
}

void func2(void){
    while(cout ++ < 5){
        printf("2\n");
        swapcontext(&ctx[1],&ctx[0]);//切换到ctx[0]上下文
        printf("5\n");
    }
}
int main()
{

#if 0
    int ret = setjmp(env);//保存当前环境到env中
    if(ret == 0){
        func(ret);
    }else if(ret == 1){
        func(ret);
    }else if(ret == 2){
        func(ret);
    }else if(ret == 3){
        func(ret);
    }
#else
    //为子协程分配栈空间
    char stack1[2048] = {0};
    char stack2[2048] = {0};

    //获取当前上下文
    getcontext(&ctx[0]);
    //设置子协程的上下文
    ctx[0].uc_stack.ss_sp = stack1;
    ctx[0].uc_stack.ss_size = sizeof(stack1);
    ctx[0].uc_link = &main_ctx;//设置子协程的上下文返回地址
    makecontext(&ctx[0],func1,0);//设置子协程的入口函数

    //获取当前上下文
    getcontext(&ctx[1]);
    //设置子协程的上下文
    ctx[1].uc_stack.ss_sp = stack2;
    ctx[1].uc_stack.ss_size = sizeof(stack2);
    ctx[1].uc_link = &main_ctx;//设置子协程的上下文返回地址
    makecontext(&ctx[1],func2,0);//设置子协程的入口函数

    printf("swapcontext start\n");
    swapcontext(&main_ctx,&ctx[0]);//切换到子协程1
    printf("swapcontext end\n");
#endif
    return 0;
}