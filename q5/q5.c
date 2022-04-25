#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define SIZE_BUFFER 10
#define PRODUTORES 3
#define CONSUMIDORES 2

pthread_t producer[PRODUTORES], consumer[CONSUMIDORES];
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned int contador = 1;

typedef struct elem {
	int value;
	struct elem *prox;
} Elem;

typedef struct blockingQueue {
	// sizeBuffer = tamanho max da fila
	// statusBuffer = tamanho atual da fila
	unsigned int sizeBuffer, statusBuffer;
	Elem *head, *last;
} BlockingQueue;

BlockingQueue *newBlockingQueue(unsigned int SizeBuffer){
    BlockingQueue *q = malloc(sizeof(BlockingQueue));
    q->last = q->head = NULL;
    q->statusBuffer = 0;
    q->sizeBuffer = SizeBuffer;
}

	// insere valor no final da fila
void putBlockingQueue(BlockingQueue *Q, int newValue) {
	
	printf("Inserindo %d no final da fila...\n", newValue);
	
	// Entrando na região crítica
	pthread_mutex_lock(&mutex);
	
	// Enquanto estiver cheia...
	while(Q->statusBuffer==Q->sizeBuffer)
		pthread_cond_wait(&empty, &mutex);
	
	// Criando Node do novo elemento a ser inserido
	Elem *newElem = malloc(sizeof(Elem));
	newElem->value = newValue;
	newElem->prox = NULL;
	
	// Se Q->last==NULL, quer dizer que a lista ainda está vazia
    if(!Q->last)
        Q->head = Q->last = newElem;
    else
        Q->last->prox = Q->last = newElem;
    
	Q->statusBuffer++;

	// Antes estava vazio
	if(Q->statusBuffer>0)
        pthread_cond_broadcast(&full);
        
	pthread_mutex_unlock(&mutex);
	
	printf("Elemento %d inserido com sucesso\n", newValue);
	sleep(1);
}


int takeBlockingQueue(BlockingQueue *Q) {
	// Resposta que vai no return
	int ans;
	printf("Retirando um elemento do inicio da fila...\n");
	
	pthread_mutex_lock(&mutex);
	
	// Enquanto estiver vazia...
	while(!Q->statusBuffer)
		pthread_cond_wait(&full, &mutex);
		
    //Inicio da retirada do elemento
    // Criado o Node que vai segurar o elemento antigo
	Elem *oldElem = malloc(sizeof(Elem));
	
	// Na cabeça da lista está o elemento a ser retirado 
	oldElem = Q->head;
	
	// Guardando a resposta em ans
	ans = Q->head->value;
	
	// Desvinculando o elemento da lista (unchaining)
    Q->head = Q->head->prox;

	// Se era o último elemento da lista, last -> head
    if (!Q->head)
    	Q->last = Q->head;

    Q->statusBuffer--;

	if(Q->statusBuffer==Q->sizeBuffer-1)
		pthread_cond_broadcast(&empty);
	
	// Saiu da regiao crítica
	pthread_mutex_unlock(&mutex);
	printf("Retirado o elemento %d\n", ans);
	sleep(1);
	
	return ans;
}

void *func_producer(void *input) {
    BlockingQueue *parameters = input;
    
	for(;;) {
		putBlockingQueue(parameters, contador++);
	}
}

void *func_consumer(void *input) {
    BlockingQueue *parameters = input;
    int ans;
	
	for (;;) {
		ans = takeBlockingQueue(parameters);
	}
}

int main() {
    BlockingQueue *Queue;
    Queue = newBlockingQueue(SIZE_BUFFER);

    pthread_mutex_init(&mutex, NULL);

    int i;

    for(i=0; i<PRODUTORES; i++)
        pthread_create(&producer[i], NULL, func_producer, (void *)Queue);
    for(i=0; i<CONSUMIDORES; i++)
        pthread_create(&consumer[i], NULL, func_consumer, (void *)Queue);

	// Nunca vão finalizar
    for(i=0; i<PRODUTORES; i++)
        pthread_join(producer[i], NULL);
    for(i=0; i<CONSUMIDORES; i++)
        pthread_join(consumer[i], NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}
