#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define OMP_NUM_THREADS 4

#define INICIO 0
#define FINAL 100
#define PASSO 1

int iterations_array[FINAL];

void fill_array(int chunk_size) {
	int j, i=INICIO;
	int id;
	
	// Distribuindo itera��es enquanto ainda tem espa�o para distribuir um chunk para todas threads
	while((i+chunk_size*OMP_NUM_THREADS) < FINAL) {
		for(id=0; id<OMP_NUM_THREADS; id++) {
			for(j=i; j<i+chunk_size; j+=PASSO)
				iterations_array[j]=id;
				
			i+=chunk_size;
		}
	}
	
	// Distribuindo itera��es quando n�o sobrou espa�o o suficiente para um chunk para cada thread
	// iter � o n�mero (inteiro) de itera��es restantes para cada thread
	// remainder � o numero deitera��es restantes que tamb�m devem ser distribu�das
	int iter = (FINAL-i) / OMP_NUM_THREADS,
		remainder = (FINAL-i) % OMP_NUM_THREADS;
	//printf("i = %d\niter = %d\nremainder = %d\n", i, iter, remainder);
	
	while(i < FINAL) {
		for(id=0; id<OMP_NUM_THREADS; id++) {
			for(j=i; j<i+((iter) + (remainder>0)); j+=PASSO)
				iterations_array[j]=id;
				
			i+=((iter) + (remainder>0));
			remainder--;
		}
	}
}

int find_final(int id) {
	int i=FINAL-1;
	
	while(1) {
		if(iterations_array[i]==id)
			return i;
		i--;
	}
}

typedef struct Parameters {
	int inicio;
	int passo;
	int final;
	int chunk;
	int id;
	void (*function) (int);
}Parameters;

void *func_thread(void* in) {
	Parameters *parameters = in;
	int inicio = parameters->inicio,
		passo = parameters->passo,
		final = parameters->final,
		id = parameters->id,
		chunk_size = parameters->chunk;
		
	int i=inicio, j;
	while(i<=final) {
		for(j=i; j<i+chunk_size; j++) {
			printf("[Thread %d]", id);
			parameters->function (j);
		}
		
		i+=chunk_size*OMP_NUM_THREADS;
	}
		
}

void omp_for(int inicio, int passo, int final, int schedule, int chunk_size, void (*f) (int))  {
	
	pthread_t threads[OMP_NUM_THREADS];
	Parameters parameters[OMP_NUM_THREADS];
	
	if(schedule==1) {
		// Static schedule
		fill_array(chunk_size);
		
		int i;
		for(i=0; i<OMP_NUM_THREADS; i++) {
			parameters[i].inicio	= inicio + i*chunk_size;
			parameters[i].passo		= passo;
			parameters[i].final		= find_final(i);
			parameters[i].chunk		= chunk_size;
			parameters[i].id		= i;
			parameters[i].function	= f;
		}
		
		for(i=0; i<OMP_NUM_THREADS; i++)
			printf("%d %d\n", parameters[i].inicio, parameters[i].final);
		
		for(i=0; i<OMP_NUM_THREADS; i++)
			pthread_create(&threads[i], NULL, func_thread, &parameters[i]);
			
		for(i=0; i<OMP_NUM_THREADS; i++)
			pthread_join(threads[i], NULL);	
		
		for(i=0; i<FINAL; i++) printf("%d: %d\n", i, iterations_array[i]);
	} else if(schedule==2) {
		// Dynamic schedule
	} else {
		// Guided schedule
	}
	//f(3);
	//(parameters[0].function) (4);
}

void simple_print(int n) {
	printf("%d\n", n);
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
	
	omp_for(INICIO, PASSO, FINAL, schedule, chunk_size, simple_print);
	
	return 0;
}
