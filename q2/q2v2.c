#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

# define MAX_SIZE 100

pthread_t threads[2];
//thread[0] no lado esquerdo
//thread[1] no lado direito

typedef struct arguments {
	int left;
	int right;
} Parameters;

//int array[] = {-11,-9,1,-4,14,3,24,-6,-8,-17,9,25,20,-24,-13,10,17,-19,8,21};
int array[] = {0,14,16,26,21,39,28,49,19,17,8,34,50,45,1,12,35,42,6,33,46};
void swap(int a, int b) {int aux=array[a]; array[a]=array[b]; array[b]=aux;}

void quicksort(int l, int r) {
	int pivot, aux;
	int i, j;

	pivot = array[r+(l-r)/2];
	i = l;
	j = r;
	
	if(j>l)
		quicksort(l, j-1);
		
	if(i<r)
		quicksort(j+1, r);
}

void *threaded_quicksort(void *in) {
	Parameters* parameters = in;
	int l = parameters->left, r = parameters->right;
	
	int pivot, aux;
	int i, j;

	pivot = array[r+(l-r)/2];
	i = l;
	j = r;
	
	while(i <= j){
		while(array[i] < pivot)
			i++;

		while(array[j] > pivot)
			j--;

		if(i <= j){
			aux = array[i];
			array[i] = array[j];
			array[j] = aux;
			i++;j--;
        }
	}
	
	if(j>l)
		quicksort(l, j);
		
	if(i<r)
		quicksort(j+1, r);
}

// Função que chama as threads, ela vai de 0 a n (toda extensão do array)
void begin_quicksort(int n) {
	int pivot, aux;
	int i, j;

	pivot = array[n/2];
	i = 1;
	j = n;
	
	while(i <= j){
		while(array[i] < pivot)
			i++;

		while(array[j] > pivot)
			j--;

		if(i <= j){
			aux = array[i];
			array[i] = array[j];
			array[j] = aux;
			i++;j--;
        }
	}
	
	Parameters parameters[2];
	
	parameters[0].left = 0;			parameters[0].right = j;
	parameters[1].left = j+1;	parameters[1].right = n;
	
	if(j>0)
		pthread_create(&threads[0], NULL, threaded_quicksort, &parameters[0]);
		//quicksort(0, a.j);
		
	if(i<n)
		pthread_create(&threads[1], NULL, threaded_quicksort, &parameters[1]);
		//quicksort(a.j+1, n);
		
	pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
}

int main() {	
	// N é o tamanho do array
	int N = 20;
	
	begin_quicksort(N);
	
	int i;
	for(i=1; i<21; i++)
		printf("arr[%d] = %d\n", i, array[i]);
	
	return 0;
}
