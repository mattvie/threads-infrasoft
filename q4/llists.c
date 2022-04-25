#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define N 4
#define SIZE 8
#define VERTEX SIZE
#define EDGE SIZE

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[SIZE*SIZE];

typedef struct Parameters {
	int id;
	int node_id;
} Parameters;

int G[SIZE][SIZE];
int source, time;
bool visited[SIZE];

void DFS(int i) {
    int j;
    visited[i] = true;
    printf(" %d->", i++);
    
    for (j=0; j<VERTEX; j++) {
        if (G[i][j] == 1 && !visited[j])
            DFS(j);
    }
}

void fill_graph() {
	int i;
	for(i=0; i<VERTEX; i++)
		G[i][i] = 1; 
}

int main() {
    int i, j;
	fill_graph();
	
	printf("Matriz de adjacencia do grafo:\n");
    for (i = 0; i < VERTEX; i++) {
        for (j = 0; j < VERTEX; j++)
            printf(" %d ", G[i][j]);
        printf("\n");
    }
    
    printf("Digite o valor da origem:\n");
    scanf("%d", & source);
    
    DFS(source-1);
    return 0;
}


