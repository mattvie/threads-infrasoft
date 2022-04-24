#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define P 20	//numero de iterações
#define NUM_VARIAVEIS 4	// numero de variaveis

double matrix_A[NUM_VARIAVEIS][NUM_VARIAVEIS] = {{2,1,1,1}, {1, 2, 1,1}, {1, 1, 2, 1},{1,1,1,2}};	//resposta deste SEL: {-1, 0, 1, 2}
double matrix_B[NUM_VARIAVEIS] = {1,2,3,4};
double matrix_X[NUM_VARIAVEIS] = {1,1,1,1};

double Res[NUM_VARIAVEIS]={0,0,0,0};	//variavel auxiliar para calcular x[i] em cada iteraÃ§Ã£o
int N = 0;			//numero de cores
int next = 0;
int interations = 0;
int blocked = 0;


pthread_mutex_t mutex;
pthread_cond_t waiter = PTHREAD_COND_INITIALIZER;
pthread_barrier_t execute_barrier;


double Sum(int tId) {
	int j=0;
	double varSum=0;
	
	while(j<NUM_VARIAVEIS) {
		if(j != tId) {
			varSum += matrix_A[tId][j] * matrix_X[j];
		}
		j++;
	}
	
	return varSum;
}

void run_jacobi(int varIndex, int threadId) {
	matrix_X[varIndex] = ( 1 / matrix_A[varIndex][varIndex]) * (matrix_B[varIndex] - Sum(varIndex));
    //Res[varIndex] = ( 1 / matrix_A[varIndex][varIndex]) * (matrix_B[varIndex]);
	//x[varIndex] = Res[varIndex];
	printf("[Thread #%d][Variavel #%d] x[%d]=%f\n", threadId, varIndex, varIndex, matrix_X[varIndex]);
}

void *run_thread(void *tid) {
	int  threadId = (*(int *)tid);
	int s = 0;
    int toExec;
    
    while (interations <= P)
    {
        pthread_mutex_lock(&mutex);
        if (blocked == 1 || interations > P){
            pthread_mutex_unlock(&mutex);
        }
        else
        {
            toExec = next;
            next++;

            if (next == NUM_VARIAVEIS){
                interations++;
                blocked = 1;
                next = 0;
            }
            pthread_mutex_unlock(&mutex);
            run_jacobi(toExec, threadId);
        }
        
        pthread_barrier_wait (&execute_barrier);
        blocked = 0;
    }
}

int	main(int argc, char *argv[]) {
	pthread_t threads[NUM_VARIAVEIS];
  clock_t tempo;
	tempo = clock();

	//pthread_mutex_init(&mutex, NULL);
	int *taskids[NUM_VARIAVEIS];
	
	int 
		i = 0,	//auxilia "for" para criar threads
		u = 0, 	//auxilia "for" das iteracoes
		k = 0;	//auxilia "for" para atualizar valor de x[i]
		
	printf("Digite o numero de threads/processadores: ");
	scanf("%d", &N);

	pthread_barrier_init (&execute_barrier, NULL, N);

    for(i=0 ; i<N ; i++) {
        taskids[i] = (int *) malloc(sizeof(int));
        *taskids[i] = i;

        pthread_create(&threads[i], NULL, run_thread, (void *)taskids[i]);
        
    }
printf("\nTempo:%f\n",(clock() - tempo) / (double)CLOCKS_PER_SEC);
	while(interations <= P){

    }

    for(i=0 ; i<N ; i++) {
        pthread_cancel(threads[i]);
    }
  
	return 0;	//com 1 processador:   Tempo:0.000157
            //com 2 processadores: Tempo:0.000212
            //com 4 processadores: Tempo:0.000329
            //com 8 processadores: Tempo:0.000738
}
