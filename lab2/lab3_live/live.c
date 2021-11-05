#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>

typedef sem_t semaphore;
typedef struct target
{
    char documentary[16]; //纪录片名称
    int documentaryID;    //纪录片ID
    int documentary_time; //纪录片时长
} Target;

typedef struct user
{
    int ID;             //用户ID
    Target Documentary; //要观看的纪录片
    int viewing_time;   //用户预期观影时间
} User;

typedef struct element
{

    semaphore mutex;    //用于确保该纪录片的viewer_count更新时的互斥
    int viewer_count;   //正在观看的人数 初始化为0
    Target Documentary; //要观看的纪录片
} Element;

#define ELEMENTS 3 // 将缓冲区的大小定义为3

semaphore buffer_mutex; // 提供缓冲区访问的互斥要求
semaphore full;         // 缓冲区被填充的数量
/*注本题中不需要缓冲区空闲的数量的信号，主要是因为纪录片数量有限，不会超过缓冲区大小。*/

//缓冲区
Element buffer[ELEMENTS];
int bufferHead = 0;
int bufferTail = 0;

semaphore rw_mutex; //用于确保该读写时的互斥

Element *current_Documentary; //维护的正在播放的纪录片

User users[5]; //不同需求的观众

void *liveRoom(void)
{
    while (1)
    {
        sem_wait(&full); //等待有纪录片被选择

        sem_wait(&buffer_mutex); //从缓存区读取下一部纪录片
        current_Documentary = &(buffer[bufferHead]);
        bufferHead = (bufferHead + 1) % ELEMENTS; //更新头指针

        sem_post(&buffer_mutex);

        sem_post(&rw_mutex); //开始播放纪录片
        printf("#######纪录片%d开始播放\n", current_Documentary->Documentary.documentaryID);
        sem_wait(&rw_mutex); //等待所有观众离开

        // 这里可以增加对电影的计时,和对顾客的踢出
    }
}

void *viewer(void *user)
{

    int isWaiting = 0;
    // printUserInfo(user);
    // 判断正在播放的是不是该用户要看的
    if (current_Documentary &&
        current_Documentary->Documentary.documentaryID ==
            ((User *)user)->Documentary.documentaryID)
    {
        sem_wait(&(current_Documentary->mutex));
        current_Documentary->viewer_count += 1;
        printf("+++用户%d开始观看纪录片%d\n", ((User *)user)->ID, current_Documentary->Documentary.documentaryID);
        sem_post(&(current_Documentary->mutex));
    }
    else
    {
        // 一个观众进入直播间后发现正在播放的不是自己要看的电影，于是进入buffer进行排队
        // 首先看buffer中有没有相同的等候观众
        int i = 0;
        for (i = bufferHead; i != bufferTail; (bufferHead + 1) % ELEMENTS)
        { // 已近有等候的观众了
            if (buffer[i].Documentary.documentaryID == ((User *)user)->Documentary.documentaryID)
            {
                sem_wait(&(buffer[i].mutex));
                buffer[i].viewer_count += 1;
                printf("+++用户%d开始观看纪录片%d\n", ((User *)user)->ID, buffer[i].Documentary.documentaryID);
                printUserInfo(user);
                sem_post(&(buffer[i].mutex));
                isWaiting = 1;
            }
        }
        // 如果没有进到某个纪录片下等待，他是第一个想看该纪录片的观众是keyViewer
        if (!isWaiting)
        {
            //首先作为 生产者 写buffer
            sem_wait(&buffer_mutex); //从缓存区读取下一部纪录片
            // 作为生产者 和 读者 将需求写入buffer中
            buffer[bufferTail].Documentary.documentaryID = (*(User *)user).Documentary.documentaryID;
            // 现在要对buffer中element初始化 作为第一个读者 开始观看
            sem_init(&(buffer[bufferTail].mutex), 0, 0);

            buffer[bufferTail].viewer_count = 1;
            //更新尾巴指针，指向下一个空位 到这里buffer已经更新完成
            int index = bufferTail;
            bufferTail = (bufferTail + 1) % ELEMENTS;

            sem_post(&buffer_mutex);
            // buffer中的填充数量增加1
            sem_post(&full);

            // 等待作者 ：直播间开始播放对应的纪录片
            sem_wait(&(rw_mutex));
            // 释放读者数量的读写
            sem_post(&(buffer[index].mutex));
            printf("+++用户%d开始观看纪录片%d\n", ((User *)user)->ID, current_Documentary->Documentary.documentaryID);
        }
    }
    // 这里是用户根据信息的观看纪录片的等待

    printf("---用户%d看完了，要离开直播间\n", ((User *)user)->ID);
    //读者退出部分  这里没有考虑用户被踢出的情况
    // 就是buffer[bufferTail].viewer_count --;
    // 所以所有的用户都只到了current_Documentary一致的时候才会运行到这里。
    sem_wait(&(current_Documentary->mutex));
    current_Documentary->viewer_count -= 1;
    if (current_Documentary->viewer_count == 0)
    {
        sem_post(&(rw_mutex));
        // 最后一个要清空 current_Documentary        
        printf("##########纪录片%d结束\n", current_Documentary->Documentary.documentaryID);
        current_Documentary = 0;
    }

    sem_post(&(current_Documentary->mutex));
}

void initUsers()
{
    int i = 0;
    for (i = 0; i < 5; ++i)
    {
        users[i].ID = i;
        users[i].Documentary.documentaryID = 1;
        users[i].viewing_time = 0; //暂时没有用
    }
}

void printUserInfo(void *user)
{
    printf("--------------------\n");
    printf("user:\n");
    printf("用户ID：\t%d\n", ((User *)user)->ID);
    printf("viewing_time：\t%d\n", ((User *)user)->viewing_time);
    printDocuInfo(&(((User *)user)->Documentary));
    printf("--------------------\n");
}

void printDocuInfo(void *docu)
{
    printf("documentary info:\n");
    printf("纪录片：\t%s\n", ((Target *)docu)->documentary);
    printf("纪录片ID：\t%d\n", ((Target *)docu)->documentaryID);
    printf("纪录片时长：\t%d\n", ((Target *)docu)->documentary_time);
}

int main()
{

    pthread_t p_liveRoom;
    pthread_t p_viewer[5];

    int num_viewers = 5;
    initUsers();

    // 初始化读写互斥量
    sem_init(&(rw_mutex), 0, 0);
    // 初始化生产者消费者信号量
    sem_init(&full, 0, 0);

    pthread_create(&p_liveRoom, 0, liveRoom, 0);
    int i;
    for (i = 0; i < num_viewers; i++)
    {
        pthread_create(&p_viewer[i], 0, viewer, &users[i]);
        sleep(1);
    }
    for (i = 0; i < num_viewers; i++)
    {
        pthread_join(p_viewer[i], 0);
    }
    sleep(100);
    return 0;
}
