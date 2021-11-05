#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>

typedef sem_t semaphore;
typedef struct target
{
    char documentary[16]; //��¼Ƭ����
    int documentaryID;    //��¼ƬID
    int documentary_time; //��¼Ƭʱ��
} Target;

typedef struct user
{
    int ID;             //�û�ID
    Target Documentary; //Ҫ�ۿ��ļ�¼Ƭ
    int viewing_time;   //�û�Ԥ�ڹ�Ӱʱ��
} User;

typedef struct element
{

    semaphore mutex;    //����ȷ���ü�¼Ƭ��viewer_count����ʱ�Ļ���
    int viewer_count;   //���ڹۿ������� ��ʼ��Ϊ0
    Target Documentary; //Ҫ�ۿ��ļ�¼Ƭ
} Element;

#define ELEMENTS 3 // ���������Ĵ�С����Ϊ3

semaphore buffer_mutex; // �ṩ���������ʵĻ���Ҫ��
semaphore full;         // ����������������
/*ע�����в���Ҫ���������е��������źţ���Ҫ����Ϊ��¼Ƭ�������ޣ����ᳬ����������С��*/

//������
Element buffer[ELEMENTS];
int bufferHead = 0;
int bufferTail = 0;

semaphore rw_mutex; //����ȷ���ö�дʱ�Ļ���

Element *current_Documentary; //ά�������ڲ��ŵļ�¼Ƭ

User users[5]; //��ͬ����Ĺ���

void *liveRoom(void)
{
    while (1)
    {
        sem_wait(&full); //�ȴ��м�¼Ƭ��ѡ��

        sem_wait(&buffer_mutex); //�ӻ�������ȡ��һ����¼Ƭ
        current_Documentary = &(buffer[bufferHead]);
        bufferHead = (bufferHead + 1) % ELEMENTS; //����ͷָ��

        sem_post(&buffer_mutex);

        sem_post(&rw_mutex); //��ʼ���ż�¼Ƭ
        printf("#######��¼Ƭ%d��ʼ����\n", current_Documentary->Documentary.documentaryID);
        sem_wait(&rw_mutex); //�ȴ����й����뿪

        // ����������ӶԵ�Ӱ�ļ�ʱ,�ͶԹ˿͵��߳�
    }
}

void *viewer(void *user)
{

    int isWaiting = 0;
    // printUserInfo(user);
    // �ж����ڲ��ŵ��ǲ��Ǹ��û�Ҫ����
    if (current_Documentary &&
        current_Documentary->Documentary.documentaryID ==
            ((User *)user)->Documentary.documentaryID)
    {
        sem_wait(&(current_Documentary->mutex));
        current_Documentary->viewer_count += 1;
        printf("+++�û�%d��ʼ�ۿ���¼Ƭ%d\n", ((User *)user)->ID, current_Documentary->Documentary.documentaryID);
        sem_post(&(current_Documentary->mutex));
    }
    else
    {
        // һ�����ڽ���ֱ����������ڲ��ŵĲ����Լ�Ҫ���ĵ�Ӱ�����ǽ���buffer�����Ŷ�
        // ���ȿ�buffer����û����ͬ�ĵȺ����
        int i = 0;
        for (i = bufferHead; i != bufferTail; (bufferHead + 1) % ELEMENTS)
        { // �ѽ��еȺ�Ĺ�����
            if (buffer[i].Documentary.documentaryID == ((User *)user)->Documentary.documentaryID)
            {
                sem_wait(&(buffer[i].mutex));
                buffer[i].viewer_count += 1;
                printf("+++�û�%d��ʼ�ۿ���¼Ƭ%d\n", ((User *)user)->ID, buffer[i].Documentary.documentaryID);
                printUserInfo(user);
                sem_post(&(buffer[i].mutex));
                isWaiting = 1;
            }
        }
        // ���û�н���ĳ����¼Ƭ�µȴ������ǵ�һ���뿴�ü�¼Ƭ�Ĺ�����keyViewer
        if (!isWaiting)
        {
            //������Ϊ ������ дbuffer
            sem_wait(&buffer_mutex); //�ӻ�������ȡ��һ����¼Ƭ
            // ��Ϊ������ �� ���� ������д��buffer��
            buffer[bufferTail].Documentary.documentaryID = (*(User *)user).Documentary.documentaryID;
            // ����Ҫ��buffer��element��ʼ�� ��Ϊ��һ������ ��ʼ�ۿ�
            sem_init(&(buffer[bufferTail].mutex), 0, 0);

            buffer[bufferTail].viewer_count = 1;
            //����β��ָ�룬ָ����һ����λ ������buffer�Ѿ��������
            int index = bufferTail;
            bufferTail = (bufferTail + 1) % ELEMENTS;

            sem_post(&buffer_mutex);
            // buffer�е������������1
            sem_post(&full);

            // �ȴ����� ��ֱ���俪ʼ���Ŷ�Ӧ�ļ�¼Ƭ
            sem_wait(&(rw_mutex));
            // �ͷŶ��������Ķ�д
            sem_post(&(buffer[index].mutex));
            printf("+++�û�%d��ʼ�ۿ���¼Ƭ%d\n", ((User *)user)->ID, current_Documentary->Documentary.documentaryID);
        }
    }
    // �������û�������Ϣ�Ĺۿ���¼Ƭ�ĵȴ�

    printf("---�û�%d�����ˣ�Ҫ�뿪ֱ����\n", ((User *)user)->ID);
    //�����˳�����  ����û�п����û����߳������
    // ����buffer[bufferTail].viewer_count --;
    // �������е��û���ֻ����current_Documentaryһ�µ�ʱ��Ż����е����
    sem_wait(&(current_Documentary->mutex));
    current_Documentary->viewer_count -= 1;
    if (current_Documentary->viewer_count == 0)
    {
        sem_post(&(rw_mutex));
        // ���һ��Ҫ��� current_Documentary        
        printf("##########��¼Ƭ%d����\n", current_Documentary->Documentary.documentaryID);
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
        users[i].viewing_time = 0; //��ʱû����
    }
}

void printUserInfo(void *user)
{
    printf("--------------------\n");
    printf("user:\n");
    printf("�û�ID��\t%d\n", ((User *)user)->ID);
    printf("viewing_time��\t%d\n", ((User *)user)->viewing_time);
    printDocuInfo(&(((User *)user)->Documentary));
    printf("--------------------\n");
}

void printDocuInfo(void *docu)
{
    printf("documentary info:\n");
    printf("��¼Ƭ��\t%s\n", ((Target *)docu)->documentary);
    printf("��¼ƬID��\t%d\n", ((Target *)docu)->documentaryID);
    printf("��¼Ƭʱ����\t%d\n", ((Target *)docu)->documentary_time);
}

int main()
{

    pthread_t p_liveRoom;
    pthread_t p_viewer[5];

    int num_viewers = 5;
    initUsers();

    // ��ʼ����д������
    sem_init(&(rw_mutex), 0, 0);
    // ��ʼ���������������ź���
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
