#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

# define MAX_SIZE 100

typedef struct arguments {
	int left;
	int right;
	int thread_number;
} Parameters;

// Array de testes
int default_array[] = {-11,-9,1,-4,14,3,24,-6,-8,-17,9,25,20,-24,-13,10,17,-19,8,21};
int array[MAX_SIZE];
//int array[] = {14,16,26,21,39,28,49,19,17,8,34,50,45,1,12,35,42,6,33,46};
int SLEEP_TIME = 0;

void print_array(int high, int i, int j) {
	printf("[");
	int k;
	for(k=0; k<high+1; k++) {
		printf(" %d", array[k]);
		
		// São marcados com * os números que serão trocados no swap
		if(k==i || k==j)
			printf("*");
	}
	printf(" ]\n");
	sleep(SLEEP_TIME);
}

void swap(int high, int i, int j) {
	print_array(high, i, j);
	
	int aux=array[i];
	array[i]=array[j];
	array[j]=aux;
}

int partition(int l, int r) {
	int pivot = array[l];
	int i = l, j = r+1;
	
	while(1) {
		
		// Elemento mais a esquerda maior ou igual ao pivot
		do i++; while(array[i]<pivot && i<r);
		// Elemento mais a direita menor ou igual ao pivot
		do j--; while(array[j]>pivot);
		
		swap(r, i, j);
				
		if(i>=j) {
			printf("[Desfazendo swap quando %d>=%d]\n", array[i], array[j]);
			printf("[i=%d   j=%d]\n", i, j);
			swap(r, i, j);
			swap(r, l, j);
			return j;
		}
	}	
}

void quicksort(int l, int r, int thread_number) {
	if(l < r) {
		printf("[On thread %d][left=%d   right=%d]\n", thread_number, l, r);
		
		int pivot = partition(l, r);
		quicksort(l, pivot-1, thread_number);
		quicksort(pivot+1, r, thread_number);
	}
}

void *threaded_quicksort(void *in) {
	Parameters* parameters = in;
	int l = parameters->left,
		r = parameters->right;
	
	if(l < r) {
		printf("[On thread %d][left=%d   right=%d]\n", parameters->thread_number, l, r);
		
		int pivot = partition(l, r);
		quicksort(l, pivot-1, parameters->thread_number);
		quicksort(pivot+1, r, parameters->thread_number);
	}
}

void *begin_quicksort(void *in) {
	Parameters* initial_parameters = in;
	int l = initial_parameters->left,
		r = initial_parameters->right;

	int pivot = partition(l, r);
	Parameters parameters[2];
	
	parameters[0].left = l;			parameters[0].right = pivot;		parameters[0].thread_number=0;
	parameters[1].left = pivot+1;	parameters[1].right = r;			parameters[1].thread_number=1;
	
	printf("[Start][left=%d   right=%d]\n", l, r);
	//print_array(r);
	pthread_t threads[2];
	//thread[0] no lado esquerdo
	//thread[1] no lado direito
	
	pthread_create(&threads[0], NULL, threaded_quicksort, &parameters[0]);
	pthread_create(&threads[1], NULL, threaded_quicksort, &parameters[1]);
		
	pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
}

int main() {
	pthread_t threads_initializer;
	// Primeira thread
	Parameters initial_parameter;
	
	int i, option, N;
	
	printf("\tWelcome to parallel Quicksort!\n\n");
	sleep(1);
	printf("\tPlease choose one of the following options:\n");
	printf("\t1. Input your own array!\n");
	printf("\tX. Test this program with the default array -- no input needed!\n");
	
	scanf("%d", &option);
	
	if(option==1) {
		printf("\n\tSelect your array size (maximum size: 100)\n");
		scanf("%d", &N);
		
		printf("\tSelect your SLEEP TIME in seconds (so you can see the swaps)\n");
		scanf("%d", &SLEEP_TIME);
		
		printf("\tInput your array %d values\n", N);
		for(i=0; i<N; i++)
			scanf("%d", &array[i]);
			
		// Array customizado vai de 0 a N-1
		initial_parameter.left = 0;
		initial_parameter.right = N-1;
		
		pthread_create(&threads_initializer, NULL, begin_quicksort, &initial_parameter);
		//pthread_join(threads_initializer, NULL);
					
	} else {
		printf("\tSelected default mode!\n");
		printf("\tSelect your SLEEP TIME in seconds (so you can see the swaps)\n");
		scanf("%d", &SLEEP_TIME);
		
		// Array padrão vai de 0 a 19
		N = 20;
		initial_parameter.left = 0;
		initial_parameter.right = 19;
		
		// Carregando array padrão no array a ser ordenado
		for(i=0; i<20; i++)
			array[i]=default_array[i];
		
		pthread_create(&threads_initializer, NULL, begin_quicksort, &initial_parameter);
			
	}
	pthread_join(threads_initializer, NULL);
	printf("\n\n\t FINAL ARRAY:\n");
	for(i=0; i<N; i++)
		printf("%d ", array[i]);
	
	return 0;
}
