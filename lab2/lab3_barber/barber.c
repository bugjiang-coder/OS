#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>

#define CHAIRS 5

typedef sem_t semaphore;

// �ȴ�����
int waiting = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;         // �ṩ���������ʵĻ���Ҫ��
pthread_mutex_t waiting_mutex = PTHREAD_MUTEX_INITIALIZER; // �ṩ�ȴ��������ʵĻ���Ҫ��
semaphore full;                                            // ����������������

// ������ �ܹ���CHAIRS������
int chairs[CHAIRS];    //����������
int nextCustomers = 0; //ָ����һλ�ȴ��Ĺ˿�
int nextChair = 0;     //ָ����һ��������

void *barber(void)
{
    while (1)
    {
        int id;
        sem_wait(&full);            //�ȴ�������
        pthread_mutex_lock(&mutex); //�п��ˣ���ȡ����id ������
        id = chairs[nextCustomers];
        nextCustomers = (nextCustomers + 1) % CHAIRS;
        pthread_mutex_unlock(&mutex);

        printf("barber���ڸ���%dλ�˿���\n", id);

        //���߿���
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
        printf("       ####�����ϰ�\n");
    }
    if (waiting < CHAIRS)
    {
        waiting += 1;
        //�˿��������ӵȴ�����
        printf("��%dλ�˿�\t���ڵ�%d��������\n", id, nextChair);

        pthread_mutex_lock(&mutex);
        chairs[nextChair] = id;
        nextChair = (nextChair + 1) % CHAIRS;
        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&waiting_mutex);
        sem_post(&full);
    }
    else
    {
        printf("-----��%dλ�˿�û��λ���뿪��\n", id);
        pthread_mutex_unlock(&waiting_mutex);
    }
}

int main()
{
    pthread_t p_barber;
    pthread_t p_customers[20];

    int num_customers = 20;

    sem_init(&full, 0, 0);

    pthread_create(&p_barber, 0, barber, 0);
    // ���ڴ����û�ID
    int nums[num_customers];
    for (int i = 0; i < num_customers; i++)
    {
        nums[i] = i;
        pthread_create(&p_customers[i], 0, customer, &nums[i]);
        if (i == 6)
            sleep(1);
    }
    for (int i = 0; i < num_customers; i++)
    {
        pthread_join(p_customers[i], 0);
    }
    sleep(10);
    return 0;
}
