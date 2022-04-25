#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define SIZE_BUFFER 20 	//Tamanho do buffer
#define PRODUTORES 1
#define CONSUMIDORES 1

pthread_t producer[PRODUTORES], consumer[CONSUMIDORES];
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct elem {
	int value;
	struct elem *prox;
} Elem;

typedef struct blockingQueue {
	// sizeBuffer = tamanho max da fila
	// statusBuffer = tamanho atual da fila
	unsigned int sizeBuffer, statusBuffer, itemGen;
	Elem *head, *last;
} BlockingQueue;

BlockingQueue *newBlockingQueue(unsigned int SizeBuffer){
    BlockingQueue *new;
    new = malloc(sizeof(BlockingQueue));
    new->last = NULL;
    new->head = NULL;
    new->itemGen = 1;
    new->statusBuffer = 0;
    new->sizeBuffer = SizeBuffer;
}

	// insere valor no final da fila
void putBlockingQueue(BlockingQueue*Q, int newValue) {
	
	printf("Inserindo um elemento no final da fila...\n");
	
	// Entrando na região crítica
	pthread_mutex_lock(&mutex);
	while(Q->statusBuffer == SIZE_BUFFER) {
        printf("Fila cheia!!\n");
		pthread_cond_wait(&empty, &mutex);
	}
	
	Elem *newElem = malloc(sizeof(Elem));
	newElem->value = newValue;
	newElem->prox = NULL;
    
    if(!Q->last){
        Q->head = Q->last = newElem;
    }
    else
    {
        Q->last->prox = newElem;
        Q->last = newElem;
    }
    
    Q->itemGen++;
	Q->statusBuffer++;

    if (Q->itemGen == 10)
        Q->itemGen = 1;
	if(Q->statusBuffer == 1)
        pthread_cond_broadcast(&full); //Broadcast em vez de signal!!!!!
	pthread_mutex_unlock(&mutex);
}


int takeBlockingQueue(BlockingQueue* Q) {
	int result;
	pthread_mutex_lock(&mutex); //Trava mutex
	while(Q->statusBuffer == 0){
        printf("Fila vazia!!\n");
		pthread_cond_wait(&full, &mutex);
    }
    //Inicio da retirada do elemento
	Elem *toDel;
    toDel = Q->head;
	result = Q->head->value;
    Q->head = Q->head->prox;

    if (Q->head == NULL){
        Q->last = Q->head;
    }

    free(toDel);
    //Fim da retirada do elemento
    Q->statusBuffer--; //VariÃ¡vel de controle

	if(Q->statusBuffer == SIZE_BUFFER - 1)
		pthread_cond_broadcast(&empty); //Broadcast em vez de signal!!!!!
	
	pthread_mutex_unlock(&mutex); //Libera mutex
	return result;
}

void *func_producer(void *pno) {
    BlockingQueue *args = pno;
	while (1){
		putBlockingQueue(args, args->itemGen);
	}
	pthread_exit(NULL);
}

void *func_consumer(void *cno) {
    BlockingQueue *args = cno;
    //printf("Consumidor Criado!!\n");
    int v;
	
	while (1) {
		v = takeBlockingQueue(args);
		//printf("Consumi %d\n", v);
	}
	pthread_exit(NULL);
}

int main() {
    BlockingQueue *Queue;
    Queue = newBlockingQueue(SIZE_BUFFER);

    pthread_t prod[PRODUTORES], cons[CONSUMIDORES];
    pthread_mutex_init(&mutex, NULL);

    int i = 0;
    

    //Cria todas as threads produtoras
    for(i = 0; i < PRODUTORES; i++) {
        
        pthread_create(&prod[i], NULL, func_producer, (void *)Queue);
    }

    //Cria todas as threads consumidoras
    for(i = 0; i < CONSUMIDORES; i++) {
        pthread_create(&cons[i], NULL, func_consumer, (void *)Queue);
    }

    //Aguarda finalizaÃ§Ã£o de todas as threads (nunca)
    for(i = 0; i < 5; i++) {
        pthread_join(prod[i], NULL);
    }
    for(i = 0; i < 5; i++) {
        pthread_join(cons[i], NULL);
    }


    pthread_mutex_destroy(&mutex);

    return 0;
}
