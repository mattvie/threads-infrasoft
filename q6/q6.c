#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define OMP_NUM_THREADS 4

#define INICIO 0
#define FINAL 15
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

void run_iterations_history() {
	// Printa o histórico de iterações
	int i, j, id;
	printf("Historico de threads!\n\n");
	
	for(id=0; id<4; id++) {
		printf("[Historico da thread %d]\n", id);
		for(j=INICIO; j<FINAL; j+=PASSO) {
			if(iterations_array[j] == id)
				printf("%d ", j);
		}
		printf("\n\n");			
	}
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

int max(int a, int b) {	return (a>b) ? a : b; }

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
			iterations_array[i] = id;
		}
		
		// Buscando a nova iteração
		pthread_mutex_lock(&mutex);
		iter = next_iter;
		next_iter += chunk_size;
		pthread_mutex_unlock(&mutex);
	}
}

void *run_guided(void* in) {
	Parameters *parameters = in;
	int i, iter, range,
		id			= parameters->id,
		chunk_size	= parameters->chunk;
		
	// Entrando na região crítica
	pthread_mutex_lock(&mutex);
	iter = next_iter;
	range = ceil((double) (FINAL-iter) / OMP_NUM_THREADS);
	range = max(range, chunk_size);
	next_iter += range;
	pthread_mutex_unlock(&mutex);
		
	while(iter < FINAL) {
		for(i=iter; (i<(iter+range)) && i<FINAL ; i+=PASSO) {
			printf("[Thread %d]", id);
			parameters->function (i);
			iterations_array[i] = id;
		}
		
		// Buscando a nova iteração
		pthread_mutex_lock(&mutex);
		iter = next_iter;
		range = ceil((double) (FINAL-iter) / OMP_NUM_THREADS);
		range = max(range, chunk_size);
		next_iter += range;
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
		
		int i;
		for(i=0; i<OMP_NUM_THREADS; i++) {
			parameters[i].chunk		= chunk_size;
			parameters[i].id		= i;
			parameters[i].function	= f;
		}
		
		for(i=0; i<OMP_NUM_THREADS; i++)
			pthread_create(&threads[i], NULL, run_guided, &parameters[i]);
			
		for(i=0; i<OMP_NUM_THREADS; i++)
			pthread_join(threads[i], NULL);
	}
	//f(3);
	//(parameters[0].function) (4);
}

void simple_print(int n) {
	printf("%d\n", n);
}

void polygon (int n) {
	if(n>2)
		printf("\n\tPoligono regular com %d lados.\n\tSeus angulos internos medem %.2f%c.\n\tSeus angulos externos medem %.2f%c.\n\n", n, (double)180*(n-2)/n, 248, (double)360/n, 248);
	else
		printf("\n\tNao foi definido um poligono regular com %d lado(s)!\n\n", n);
}

int main() {
	
	int schedule, chunk_size, function_choice;
	
	printf("\tBem-vindo ao simulador de OMP!\n");
	printf("\tCaso deseje testes diferentes, INICIO, FINAL e PASSO se encontram nas linhas 8, 9 e 10 do codigo.\n");
	
	do {
		printf("\n\nEscolha:\n\t1.\tStatic schedule.\n\t2.\tDynamic schedule.\n\t3.\tGuided schedule.\n");
		scanf("%d", &schedule);
	} while(schedule<1 || schedule>3);
	
	do {
		printf("\n\nEscolha chunk size (maior que 0):\n");
		scanf("%d", &chunk_size);
	} while(chunk_size<1);
	
	do {
		printf("\n\nEscolha:\n\t1.\tPolygon function.\n\t\t(Exibe dados de um poligono regular de lado n)\n\t2.\tPrint function.\n\t\t(Simplesmente printa n na tela)\n");
		scanf("%d", &function_choice);
	} while(function_choice<1 || function_choice>2);
	
	if(function_choice==1)
		omp_for(INICIO, PASSO, FINAL, schedule, chunk_size, polygon);
	else
		omp_for(INICIO, PASSO, FINAL, schedule, chunk_size, simple_print);
		
	run_iterations_history();
	
	return 0;
}
