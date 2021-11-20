

## 代码到可执行程序的过程

![image-20211120100005865](Makefile-learn_image/image-20211120100005865.png)



### 0. 源文件

```c
#include<stdio.h>

#define STR "Hello world!!"
#define PRT printf


int main(){
    PRT(STR);
    getchar();
    return 0;
}
```





### 1. 预处理

`gcc -E hello.c -o hello.i`

把宏定义全部替换：

```c
# 7 "hello.c"
int main(){
    printf("Hello world!!");
    getchar();
    return 0;
}
```

