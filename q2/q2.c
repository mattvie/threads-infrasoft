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
	int thread_number;
} Parameters;

typedef struct partition {
	int pivot;
	int i;
	int j;
} Partition;

//int array[] = {-11,-9,1,-4,14,3,24,-6,-8,-17,9,25,20,-24,-13,10,17,-19,8,21};
int array[] = {14,16,26,21,39,28,49,19,17,8,34,50,45,1,12,35,42,6,33,46};
void swap(int a, int b) {int aux=array[a]; array[a]=array[b]; array[b]=aux;}

Partition partition(int l, int r) {
	Partition ans;
	ans.pivot = array[l];
	ans.i = l-1, ans.j = r+1;
	
	while(1) {
		
		// Elemento mais a esquerda maior ou igual ao pivot
		do ans.i++; while(array[ans.i]<ans.pivot);
		// Elemento mais a direita menor ou igual ao pivot
		do ans.j--; while(array[ans.j]>ans.pivot);
		
		if(ans.i>=ans.j)
			return ans;
			
		swap(ans.i, ans.j);
	}	
}

void quicksort(int l, int r, int thread_number) {
	Partition a = partition(l, r);
	int pivot = a.pivot, i = a.i, j = a.j;
	
	printf("[Thread %d] left=%d\tpivot=%d\tright=%d\n", thread_number, l, pivot, r);
	
	if(j>l)
		quicksort(l, j-1, thread_number);
		
	if(i<r)
		quicksort(j+1, r, thread_number);
}

void *threaded_quicksort(void *in) {
	Parameters* parameters = in;
	int l = parameters->left, r = parameters->right;
	
	Partition a = partition(l, r);
	int pivot = a.pivot, i = a.i, j = a.j;
	
	printf("[Thread %d] left=%d\tpivot=%d\tright=%d\n", parameters->thread_number, l, pivot, r);
	
	if(j>l)
		quicksort(l, j-1, parameters->thread_number);
		
	if(i<r)
		quicksort(j+1, r, parameters->thread_number);
}

void begin_quicksort(int n) {
	Partition a = partition(0, n);
	Parameters parameters[2];
	
	parameters[0].left = 0;			parameters[0].right = a.pivot;	parameters[0].thread_number=0;
	parameters[1].left = a.pivot+1;	parameters[1].right = n;			parameters[1].thread_number=1;
	
	printf("[Begin] left=%d\tpivot=%d\tright=%d\n", 0, a.pivot, n);
	
	if(a.j>0)
		pthread_create(&threads[0], NULL, threaded_quicksort, &parameters[0]);
		//quicksort(0, a.j);
		
	if(a.i<n)
		pthread_create(&threads[1], NULL, threaded_quicksort, &parameters[1]);
		//quicksort(a.j+1, n);
		
	pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
}

int main() {	
	// N é o tamanho do array
	int N = 20;
	
	// Função que chama as threads
	begin_quicksort(N-1);
	
	int i;
	for(i=0; i<20; i++)
		printf("arr[%d] = %d\n", i, array[i]);
	
	return 0;
}
