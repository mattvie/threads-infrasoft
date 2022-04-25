#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define N 4
#define SIZE 8
#define VERTEX SIZE
#define EDGE SIZE

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[SIZE];

typedef struct Parameters {
	int id;
	int node_id;
} Parameters;

Parameters parameters[SIZE];

int G[SIZE][SIZE];
int source;
bool visited[SIZE];

void *DFS(void *in) {
	Parameters *parameters = in;
    int j,
		i = parameters->node_id,
		id = parameters->id;
	
    visited[i] = true;
    printf("Thread %d visitou %d\n", id, i++);

    for (j=0; j<VERTEX; j++) {
        if (G[i][j] == 1 && !visited[j]) {
        	// Região crítica -- cada thread deve ter um ID diferente
        	pthread_mutex_lock(&mutex);
        	
        	parameters[id+1].id = id+1;
        	parameters[id+1].node_id = j;
        	pthread_create(&threads[id+1], NULL, DFS, &parameters[id+1]);
        	
        	// Fim da região crítica
        	pthread_mutex_unlock(&mutex);
		}
    }
}

void fill_graph() {
	int i, j;
	printf("Matriz de adjacencia do grafo:\n");
	for(i=0; i<VERTEX; i++) {
		G[i][i] = 1;
		for (j=0; j<VERTEX; j++)
            printf(" %d ", G[i][j]);
        printf("\n");
	}
}

int main() {
    int i, j;
	fill_graph();
    
    printf("\nDigite o valor da origem:\n");
    scanf("%d", &source);
    parameters[0].id = 0;
    parameters[0].node_id = source;
    pthread_create(&threads[0], NULL, DFS, &parameters[0]);
    
    for(i=0; i<VERTEX; i++)
    	pthread_join(threads[i], NULL);
    
    return 0;
}
