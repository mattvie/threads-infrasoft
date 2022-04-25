#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

# define MAX_SIZE 100

typedef struct arguments {
	int left;
	int right;
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
			swap(r, i, j);
			swap(r, l, j);
			return j;
		}
	}	
}

void *threaded_quicksort(void *in) {
	Parameters parameters;
	Parameters* initial_parameters = in;
	int l = initial_parameters->left,
		r = initial_parameters->right;
	printf("[On thread][left=%d   right=%d]\n", l, r);
	
	if(l < r) {
		pthread_t thread;
		
		// Setando pivot
		int pivot = partition(l, r);
		
		// Setando parâmetros para o lado esquerdo do pivot
		parameters.left = l;
		parameters.right = pivot-1;
	
		pthread_create(&thread, NULL, threaded_quicksort, &parameters);
		pthread_join(thread, NULL);
		
		// Setando parâmetros para o lado direito do pivot
		parameters.left = pivot+1;
		parameters.right = r;
		
		pthread_create(&thread, NULL, threaded_quicksort, &parameters);
		pthread_join(thread, NULL);
	}
}

int main() {
	pthread_t threads_initializer;
	// Primeira thread
	Parameters initial_parameter;
	
	int i, option, N;
	
	printf("\tWelcome to parallel Quicksort!\n\n");
	printf("\tPlease choose one of the following options:\n");
	printf("\t1. Input your own array!\n");
	printf("\tX. Test this program with the default array -- no input needed!\n");
	
	scanf("%d", &option);
	
	if(option==1) {
		printf("\tSelected custom mode!\n");
		printf("\n\tSelect your array size (maximum size: 100)\n");
		scanf("%d", &N);
		
		printf("\tSelect your SLEEP TIME in seconds (that way you can see the swaps)\n");
		scanf("%d", &SLEEP_TIME);
		
		printf("\tInput your array %d values\n", N);
		for(i=0; i<N; i++)
			scanf("%d", &array[i]);
			
		// Array customizado vai de 0 a N
		initial_parameter.left = 0;
		initial_parameter.right = N-1;
		
		pthread_create(&threads_initializer, NULL, threaded_quicksort, &initial_parameter);
							
	} else {
		printf("\tSelected default mode!\n");
		printf("\tSelect your SLEEP TIME in seconds (that way you can see the swaps)\n");
		scanf("%d", &SLEEP_TIME);
		
		// Array padrão vai de 0 a 20
		N = 20;
		initial_parameter.left = 0;
		initial_parameter.right = N-1;
		
		// Carregando array padrão no array a ser ordenado
		for(i=0; i<20; i++)
			array[i]=default_array[i];
		
		pthread_create(&threads_initializer, NULL, threaded_quicksort, &initial_parameter);	
	}
	
	pthread_join(threads_initializer, NULL);
	
	printf("\n\n\t FINAL ARRAY:\n");
	for(i=0; i<N; i++)
		printf("%d ", array[i]);
	
	return 0;
}

