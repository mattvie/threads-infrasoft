#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define OMP_NUM_THREADS 4

#define INICIO 0
#define FINAL 10
#define PASSO 1

typedef struct Parameters {
	int inicio;
	int passo;
	int final;
	void (*function) (int);
}Parameters;

void *func_thread(void* in) {
	Parameters *parameters = in;
	int inicio = parameters->inicio,
		passo = parameters->passo,
		final = parameters->final;
		
	int i;
	for(i=parameters->inicio; i<parameters->final; i+=parameters->passo)
		parameters->function (i);
}

void omp_for(int inicio, int passo, int final, int schedule, int chunk_size, void (*f) (int))  {
	pthread_t threads[OMP_NUM_THREADS];
	Parameters parameters[OMP_NUM_THREADS];
	parameters[0].passo 	= parameters[1].passo	  = parameters[2].passo		= parameters[3].passo		= passo;
	parameters[0].function = parameters[1].function = parameters[2].function = parameters[3].function	= f;
	// Static schedule
	if(schedule==1) {
		int i;
		for(i=0; i<OMP_NUM_THREADS; i++) {
			parameters[i].inicio = inicio + i*chunk_size;
			
			int h=0, k=parameters[i].inicio;
			while((k + chunk_size + h*chunk_size*OMP_NUM_THREADS) <= final) h++;
			parameters[i].final = k + chunk_size + (h-1)*chunk_size*OMP_NUM_THREADS;
		}
		
		//for(i=0; i<OMP_NUM_THREADS; i++)
		//	printf("%d %d\n", parameters[i].inicio, parameters[i].final);
		
		for(i=0; i<OMP_NUM_THREADS; i++)
			pthread_create(&threads[i], NULL, func_thread, &parameters[i]);
			
		for(i=0; i<OMP_NUM_THREADS; i++)
			pthread_join(threads[i], NULL);	
		
		// Dynamic schedule
	} else if(schedule==2) {
		
		
		
		// Guided schedule
	} else {
		
		
		
		
	}
	//f(3);
	//(parameters[0].function) (4);
}

void polygon (int n) {
	if(n>2)
		printf("Poligono regular com %d lados.\n\tSeus angulos internos medem %.2f%c.\n\tSeus angulos externos medem %.2f%c\n\n", n, (double)180*(n-2)/n, 248, (double)360/n, 248);
	else
		printf("Nao foi definido um poligono regular com %d lado(s)!\n\n", n);
}

int main() {
	
	int schedule, chunk_size;
	
	do {
		printf("Escolha:\n\t1.\tStatic schedule.\n\t2.\tDynamic schedule.\n\t3.\tGuided schedule.\n");
		scanf("%d", &schedule);
	} while(schedule<1 || schedule>3);
	
	do {
		printf("Escolha chunk size (maior que 0):\n");
		scanf("%d", &chunk_size);
	} while(chunk_size<1);
	
	omp_for(INICIO, PASSO, FINAL, schedule, chunk_size, polygon);
	
	return 0;
}
