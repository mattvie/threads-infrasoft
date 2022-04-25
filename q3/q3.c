#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define P 20	//numero de iteracoes
#define NUM_VARIAVEIS 4

double matrix_A[NUM_VARIAVEIS][NUM_VARIAVEIS] = {{2,1,1,1}, {1, 2, 1,1}, {1, 1, 2, 1},{1,1,1,2}};	//resposta deste SEL: {-1, 0, 1, 2}
double matrix_B[NUM_VARIAVEIS] = {1,2,3,4};
double matrix_X[NUM_VARIAVEIS] = {1,1,1,1};

double Res[NUM_VARIAVEIS]={0,0,0,0};	// Aqui fica guardada a resposta
int N = 0;
int next = 0;
int iterations = 0;
bool blocked = false;

pthread_t threads[NUM_VARIAVEIS];
pthread_mutex_t mutex;
pthread_cond_t waiter = PTHREAD_COND_INITIALIZER;
pthread_barrier_t execute_barrier;

double Sum(int tId) {
	int j=0;
	double varSum=0;
	
	while(j<NUM_VARIAVEIS) {
		if(j != tId)
			varSum += matrix_A[tId][j] * matrix_X[j];
		j++;
	}
	
	return varSum;
}

void jacobi(int varIndex, int threadId) {
	matrix_X[varIndex] = ( 1 / matrix_A[varIndex][varIndex]) * (matrix_B[varIndex] - Sum(varIndex));

	printf("[Thread #%d][Variavel #%d] x[%d]=%f\n", threadId, varIndex, varIndex, matrix_X[varIndex]);
}

void *run_thread(void *tid) {
	int  threadId = (*(int *)tid);
	int s = 0;
    int toExec;
    
    while (iterations <= P) {
        // Entrando na região crítica
		pthread_mutex_lock(&mutex);
        if (blocked || iterations>P){
            pthread_mutex_unlock(&mutex);
        }
        else
        {
            toExec = next;
            next++;

            if (next == NUM_VARIAVEIS){
                iterations++;
                blocked = true;
                next = 0;
            }
            pthread_mutex_unlock(&mutex);
            jacobi(toExec, threadId);
        }
        
        pthread_barrier_wait (&execute_barrier);
        blocked = false;
    }
}

int	main() {
  	clock_t tempo;
	tempo = clock();

	int *taskids[NUM_VARIAVEIS], i;
	
	do {
		printf("Informe o numero de processadores (max size %d): ", NUM_VARIAVEIS);
		scanf("%d", &N);
	} while(N<0 && N>=NUM_VARIAVEIS);

	pthread_barrier_init (&execute_barrier, NULL, N);

    for(i=0 ; i<N ; i++) {
        taskids[i] = (int *) malloc(sizeof(int));
        *taskids[i] = i;

        pthread_create(&threads[i], NULL, run_thread, taskids[i]);
    }

    for(i=0 ; i<N ; i++)
        pthread_join(threads[i], NULL);
  
  	// Obs: executar o programa algumas vezes, só então os valores de tempo ficam uniformes
  	printf("\nExec time: %.2f\n", (clock() - tempo)/((double) CLOCKS_PER_SEC));
	return 0;
	//com 1 processador:   Tempo:0.000157
    //com 2 processadores: Tempo:0.000212
    //com 4 processadores: Tempo:0.000329
    //com 8 processadores: Tempo:0.000738
}
