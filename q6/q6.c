#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define OMP_NUM_THREADS 3

#define INICIO 0
#define FINAL 100
#define PASSO 1

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[OMP_NUM_THREADS];

typedef struct Parameters {
	int inicio;
	int passo;
	int final;
	int chunk;
	int id;
	void (*function) (int);
}Parameters;

Parameters parameters[OMP_NUM_THREADS];
	
int iterations_array[FINAL];
int next_iter = INICIO;

void iterations_history() {
	// Printa o histórico de iterações
}

void fill_iterations_array(int chunk_size) {
	int j, i=INICIO;
	int id;
	
	// Distribuindo iterações enquanto ainda tem espaço para distribuir um chunk para todas threads
	while((i+chunk_size*OMP_NUM_THREADS) < FINAL) {
		for(id=0; id<OMP_NUM_THREADS; id++) {
			for(j=i; j<i+chunk_size; j+=PASSO)
				iterations_array[j]=id;
				
			i+=chunk_size;
		}
	}
	
	// Distribuindo iterações quando não sobrou espaço o suficiente para um chunk para cada thread
	// iter é o número (inteiro) de iterações restantes para cada thread
	// remainder é o numero deiterações restantes que também devem ser distribuídas
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

int get_final(int id) {
	int i=FINAL-1;
	
	while(1) {
		if(iterations_array[i]==id)
			return i;
		i--;
	}
}

void *run_static(void* in) {
	Parameters *parameters = in;
	int inicio = parameters->inicio,
		passo = parameters->passo,
		final = parameters->final,
		id = parameters->id,
		chunk_size = parameters->chunk;
		
	int i=inicio, j;
	while(i<=final) {
		for(j=i; j<(i+chunk_size); j+=PASSO) {
			printf("[Thread %d]", id);
			parameters->function (j);
		}
		
		i+=chunk_size*OMP_NUM_THREADS;
	}
		
}

void *run_dynamic(void* in) {
	Parameters *parameters = in;
	int i, iter=0,
		id			= parameters->id,
		chunk_size	= parameters->chunk;
		
	// Entrando na região crítica
	pthread_mutex_lock(&mutex);
	iter = next_iter;
	next_iter += chunk_size;
	pthread_mutex_unlock(&mutex);
		
	while(iter < FINAL) {
		
		for(i=iter; (i<(iter+chunk_size)) && i<FINAL ; i+=PASSO) {
			printf("[Thread %d]", id);
			parameters->function (i);
		}
		
		// Buscando a nova iteração
		pthread_mutex_lock(&mutex);
		iter = next_iter;
		next_iter += chunk_size;
		pthread_mutex_unlock(&mutex);
	}
}

void omp_for(int inicio, int passo, int final, int schedule, int chunk_size, void (*f) (int)) {
	
	
	
	if(schedule==1) {
		// Static schedule
		fill_iterations_array(chunk_size);
		
		int i;
		for(i=0; i<OMP_NUM_THREADS; i++) {
			parameters[i].inicio	= inicio + (i*chunk_size);
			parameters[i].passo		= passo;
			parameters[i].final		= get_final(i);
			parameters[i].chunk		= chunk_size;
			parameters[i].id		= i;
			parameters[i].function	= f;
		}
		
		//for(i=0; i<OMP_NUM_THREADS; i++)
		//	printf("%d %d\n", parameters[i].inicio, parameters[i].final);
		
		for(i=0; i<OMP_NUM_THREADS; i++)
			pthread_create(&threads[i], NULL, run_static, &parameters[i]);
			
		for(i=0; i<OMP_NUM_THREADS; i++)
			pthread_join(threads[i], NULL);	
		
		//for(i=0; i<FINAL; i++)
		//	printf("%d: %d\n", i, iterations_array[i]);
	} else if(schedule==2) {
		// Dynamic schedule
		
		int i;
		for(i=0; i<OMP_NUM_THREADS; i++) {
			parameters[i].chunk		= chunk_size;
			parameters[i].id		= i;
			parameters[i].function	= f;
		}
		
		for(i=0; i<OMP_NUM_THREADS; i++)
			pthread_create(&threads[i], NULL, run_dynamic, &parameters[i]);
			
		for(i=0; i<OMP_NUM_THREADS; i++)
			pthread_join(threads[i], NULL);
		
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
