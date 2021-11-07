#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>

#define ELEMENTS 3 // ���������Ĵ�С����Ϊ3 Ҳ�Ǽ�¼Ƭ����Ϊ3

typedef sem_t semaphore;

typedef struct documentary
{
    char name[16]; //��¼Ƭ����
    int DID;       //��¼ƬID
    int dTime;     //��¼Ƭʱ��
} Doc;

typedef struct user
{
    int UID;   //�û�ID
    Doc doc;   //Ҫ�ۿ��ļ�¼Ƭ
    int uTime; //�û�Ԥ�ڹ�Ӱʱ��
} User;

typedef struct element
{
    semaphore mutex;    //����ȷ���ü�¼Ƭ��viewer_count����ʱ�Ļ���
    semaphore rw_mutex; //����ȷ���ö�дʱ�Ļ���
    int viewer_count;   //���ڹۿ������� ��ʼ��Ϊ0
    Doc doc;            //Ҫ�ۿ��ļ�¼Ƭ
} Element;

semaphore buffer_mutex; // �ṩ���������ʵĻ���Ҫ��
semaphore full;         // ����������������
/*ע�����в���Ҫ���������е��������źţ���Ҫ����Ϊ��¼Ƭ�������ޣ����ᳬ����������С��*/

//������
Element buffer[ELEMENTS]; //ʹ��element������Ϊbuffer
int bufferHead = 0;       //ָ����һ��Ҫ���ŵ�element
int bufferTail = 0;       //ָ�����һ��Ҫ���ŵ�element

Element *showing; //ά�������ڲ��ŵļ�¼Ƭ

User users[50]; //��ͬ����Ĺ���

void *liveRoom(void)
{
    while (1)
    {
        sem_wait(&full); //�ȴ��м�¼Ƭ��ѡ��

        sem_wait(&buffer_mutex); //�ӻ�������ȡ��һ����¼Ƭ
        showing = &(buffer[bufferHead]);
        bufferHead = (bufferHead + 1) % ELEMENTS; //����ͷָ��

        sem_post(&buffer_mutex);

        printf("#######��¼Ƭ%d��ʼ����\n", showing->doc.DID);
        sem_post(&(showing->rw_mutex)); //������е�д ��ʼ���ż�¼Ƭ

        sem_wait(&(showing->rw_mutex)); //�ȴ����й����뿪 ��ʼ��һ��д

        printf("#######��¼Ƭ%d����\n", showing->doc.DID);
        // Ҫ��� showing
        showing = 0;
    }
}

void *viewer(void *user)
{
    int isWaiting = 0;
    // printUserInfo(user);
    // �ж����ڲ��ŵ��ǲ��Ǹ��û�Ҫ����
    if (showing && showing->doc.DID == ((User *)user)->doc.DID)
    {
        sem_wait(&(showing->mutex));
        showing->viewer_count += 1;
        printf("+++�û�%d��ʼ�ۿ���¼Ƭ%d\n", ((User *)user)->UID, showing->doc.DID);
        sem_post(&(showing->mutex));
    }
    else
    {
        // һ�����ڽ���ֱ����������ڲ��ŵĲ����Լ�Ҫ���ĵ�Ӱ�����ǽ���buffer�����Ŷ�
        // ���ȿ�buffer����û����ͬ�ĵȺ����
        int i = 0;
        int index;
        for (i = bufferHead; i != bufferTail; i = (i + 1) % ELEMENTS)
        { // �ѽ��еȺ�Ĺ�����
            if (buffer[i].doc.DID == ((User *)user)->doc.DID)
            {
                sem_wait(&(buffer[i].mutex));
                buffer[i].viewer_count += 1;
                printf("+++�û�%d��ʼ�ۿ���¼Ƭ%d\n", ((User *)user)->UID, buffer[i].doc.DID);
                sem_post(&(buffer[i].mutex));
                isWaiting = 1;
            }
        }
        // ���û�н���ĳ����¼Ƭ�µȴ������ǵ�һ���뿴�ü�¼Ƭ�Ĺ�����keyViewer
        if (!isWaiting)
        {
            //������Ϊ������дbuffer
            sem_wait(&buffer_mutex); //�ӻ�������ȡ��һ����¼Ƭ
            // ��Ϊ�����߽�����д��buffer��
            buffer[bufferTail].doc.DID = (*(User *)user).doc.DID;
            // ����Ҫ��buffer��element��ʼ�� ��Ϊ��һ������ ��ʼ�ۿ�
            sem_init(&(buffer[bufferTail].mutex), 0, 0);
            // ��ʼ����д������
            sem_init(&(buffer[bufferTail].rw_mutex), 0, 0);

            buffer[bufferTail].viewer_count = 1;
            //����β��ָ�룬ָ����һ����λ��������buffer�Ѿ��������
            // ���index��λ�ò��ܱ䣬���û���bug
            index = bufferTail;
            bufferTail = (bufferTail + 1) % ELEMENTS;

            sem_post(&buffer_mutex);
            // buffer�е������������1
            sem_post(&full);

            // �ȴ����� ��ֱ���俪ʼ���Ŷ�Ӧ�ļ�¼Ƭ
            sem_wait(&(buffer[index].rw_mutex));
            // �ͷŶ��������Ķ�д
            sem_post(&(buffer[index].mutex));
            printf("+++�û�%d��ʼ�ۿ���¼Ƭ%d\n", ((User *)user)->UID, showing->doc.DID);
        }
    }

    printf("---�û�%d�����ˣ�Ҫ�뿪ֱ����\n", ((User *)user)->UID);
    //�����˳�����  ����û�п����û����߳������
    // ����buffer[bufferTail].viewer_count --;
    // �������е��û���ֻ����current_Documentaryһ�µ�ʱ��Ż����е����
    sem_wait(&(showing->mutex));
    showing->viewer_count -= 1;
    if (showing->viewer_count == 0)
    {
        sem_post(&(showing->rw_mutex));
    }

    sem_post(&(showing->mutex));
}

void initUsers()
{
    int i = 0;
    for (i = 0; i < 20; ++i)
    {
        users[i].UID = i;
        users[i].doc.DID = 0;
        users[i].uTime = 0; //��ʱû����
    }
    users[1].doc.DID = 2;
    users[3].doc.DID = 1;
    for (i = 20; i < 50; ++i)
    {
        users[i].UID = i;
        users[i].doc.DID = (i % 3);
        users[i].uTime = 0; //��ʱû����
    }
}

int main()
{

    pthread_t p_liveRoom;
    pthread_t p_viewer[50];

    int num_viewers = 50;
    initUsers();

    // ��ʼ���������������ź���
    sem_init(&full, 0, 0);

    pthread_create(&p_liveRoom, 0, liveRoom, 0);
    int i;
    for (i = 0; i < num_viewers; i++)
    {
        pthread_create(&p_viewer[i], 0, viewer, &users[i]);
        if(i == 25)
            sleep(1);
    }
    for (i = 0; i < num_viewers; i++)
    {
        pthread_join(p_viewer[i], 0);
    }
    sleep(100);
    return 0;
}
