#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>

#define CHAIRS 5

typedef sem_t semaphore;

// 等待人数
int waiting = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;         // 提供缓冲区访问的互斥要求
pthread_mutex_t waiting_mutex = PTHREAD_MUTEX_INITIALIZER; // 提供等待人数访问的互斥要求
semaphore full;                                            // 缓冲区被填充的数量

// 缓冲区 总共有CHAIRS把椅子
int chairs[CHAIRS];    //缓冲区数组
int nextCustomers = 0; //指向下一位等待的顾客
int nextChair = 0;     //指向下一个空椅子

void *barber(void)
{
    while (1)
    {
        int id;
        sem_wait(&full);            //等待客人来
        pthread_mutex_lock(&mutex); //有客人，读取客人id 进行理发
        id = chairs[nextCustomers];
        nextCustomers = (nextCustomers + 1) % CHAIRS;
        pthread_mutex_unlock(&mutex);

        printf("barber正在给第%d为顾客理发\n", id);
        sleep(2);

        //送走客人
        pthread_mutex_lock(&waiting_mutex);
        waiting -= 1;
        pthread_mutex_unlock(&waiting_mutex);
    }
}

void *customer(void *num)
{
    int id = *(int *)num;

    pthread_mutex_lock(&waiting_mutex);
    if (waiting == 0)
    {
        printf("       ####叫醒老板\n");
    }
    if (waiting < CHAIRS)
    {
        waiting += 1;
        //顾客坐上椅子等待被叫
        printf("第%d位顾客\t坐在第%d把椅子上\n", id, nextChair);

        pthread_mutex_lock(&mutex);
        chairs[nextChair] = id;
        nextChair = (nextChair + 1) % CHAIRS;
        pthread_mutex_unlock(&mutex);
    }
    else
    {
        printf("-----第%d位顾客没有位置离开了\n", id);
    }

    pthread_mutex_unlock(&waiting_mutex);
    sem_post(&full);
}

int main()
{
    pthread_t p_barber;
    pthread_t p_customers[20];

    int num_customers = 20;

    sem_init(&full, 0, 0);

    pthread_create(&p_barber, 0, barber, 0);
    for (int i = 0; i < num_customers; i++)
    {
        // 把i传入可能有bug！！ i的值是变化的
        pthread_create(&p_customers[i], 0, customer, &i);
        sleep(1);
        if (i == 8)
        {
            sleep(10);
        }
    }
    for (int i = 0; i < num_customers; i++)
    {
        pthread_join(p_customers[i], 0);
    }
    sleep(10);
    return 0;
}
