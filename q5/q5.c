#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define SIZE_BUFFER 5	// Tamanho maximo da fila
#define PRODUTORES 1
#define CONSUMIDORES 1

pthread_t produtores[PRODUTORES], consumidores[CONSUMIDORES];
pthread_cond_t vazio = PTHREAD_COND_INITIALIZER;
pthread_cond_t cheio = PTHREAD_COND_INITIALIZER;
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

BlockingQueue *newBlockingQueue(unsigned int SizeBuffer) {
	
	BlockingQueue *queue = malloc(sizeof(BlockingQueue));
    queue->head = queue->last = NULL;
    queue->itemGen = 1;
    queue->statusBuffer = 0;
    queue->sizeBuffer = SizeBuffer;
    
    return queue;
}

	// insere valor no final da fila
void putBlockingQueue(BlockingQueue*Q, int newValue) {
	
	printf("Inserindo um elemento no final da fila...\n");
	
	// Entrando na região crítica
	pthread_mutex_lock(&mutex);
	while(Q->statusBuffer == SIZE_BUFFER) {
        printf("Fila cheia. Tentando novamente.\n");
		pthread_cond_wait(&vazio, &mutex);
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
        pthread_cond_broadcast(&cheio); //Broadcast em vez de signal!!!!!
	pthread_mutex_unlock(&mutex);
}


int takeBlockingQueue(BlockingQueue* Q) {
	printf("Retirando um elemento da fila...\n");
	
	// Entra na região crítica
	pthread_mutex_lock(&mutex);
	while(Q->statusBuffer == 0) {
        printf("Fila vazia. Esperando um elemento para retirar.\n");
		pthread_cond_wait(&cheio, &mutex);
    }
    
    int res;
	Elem *outElem;
	// Desencadeando outElem
    outElem = Q->head;
	res = Q->head->value;
    Q->head = Q->head->prox;

    if (Q->head==NULL){
        Q->last = Q->head;
    }

    free(outElem);
    //Fim da retirada do elemento
    Q->statusBuffer--; //VariÃ¡vel de controle

	if(Q->statusBuffer == SIZE_BUFFER - 1)
		pthread_cond_broadcast(&vazio); //Broadcast em vez de signal!!!!!
	
	pthread_mutex_unlock(&mutex); //Libera mutex
	return res;
}

void *func_produtor(void *in) {
    BlockingQueue *parameters = in;
    
	while (1) {
		putBlockingQueue(parameters, parameters->itemGen);
	}
	
	pthread_exit(NULL);
}

void *func_consumidor(void *in) {
    BlockingQueue *parameters = in;

    int res;
	while (1) {
		res = takeBlockingQueue(parameters);
		//printf("Consumi %d\n", v);
	}
	
	pthread_exit(NULL);
}

int main() {
	BlockingQueue *queue = newBlockingQueue(SIZE_BUFFER);
	 pthread_mutex_init(&mutex, NULL);
	
	int i;
	for(i=0; i<PRODUTORES; i++)
        pthread_create(&produtores[i], NULL, func_produtor, &queue);
        
	for(i=0; i<CONSUMIDORES; i++)
        pthread_create(&consumidores[i], NULL, func_consumidor, &queue);
	
	// Join...
	for(i=0; i<PRODUTORES; i++)
        pthread_join(produtores[i], NULL);
        
	for(i=0; i<CONSUMIDORES; i++)
        pthread_join(consumidores[i], NULL);
	
	pthread_mutex_destroy(&mutex);
	return 0;
}
