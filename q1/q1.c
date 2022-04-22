#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define MAX_SIZE 20

// 3 arquivos, 3 threads, 5 produtos
#define N 3
#define T 3
#define P 5

pthread_mutex_t mutexes[MAX_SIZE+1];
pthread_t threads[MAX_SIZE+1];

int vendas[MAX_SIZE+1];

void *func_thread(void *id) {
	char file_name[10];
	FILE *file;
	
	// A thread vai percorrer os arquivos i.in, (i+T).in, (i+2T).in, até não ter mais arquivos...
	int i = id, prod=1, value;
	
	while(i<=T) {
		// Setando o nome do arquivo a ser aberto
		sprintf(file_name, "%d.in", id);
		
		printf("[Thread %d] Lendo o arquivo %s.\n", id, file_name);
		file = fopen(file_name, "r");
		
		// Lendo todas as linhas do arquivo
		while(fscanf(file, "%d", &value)!=EOF) {
			// Entrando na região crítica do número de vendas[prod]
			pthread_mutex_lock(&mutexes[prod]);
			vendas[prod]+=value;
			pthread_mutex_unlock(&mutexes[prod]);
			// Saindo da região crítica. Agora outras threads podem acessar o recurso
			
			printf("[Thread %d] Produto %d: %d vendas.\n", id, prod, value);
			prod++;
		}
		
		// Próxima iteração (se tiver alguma...)
		i+=T;
	}
}

	// Resultado: 4 2 7 6 9
int main(){
	// Numero de arquivos N, threads T, e produtos P setados no define
	
	// Zerando o vetor vendas
	int i;
	for(i=1; i<=MAX_SIZE; i++)
		vendas[i]=0;
	
	// id não precisa ser um vetor. Alterar depois.
    for(i=1; i<=T; i++) {
        pthread_create(&threads[i], NULL, func_thread, i);
    }
    
    for(i=1; i<=N; i++) {
        pthread_join(threads[i], NULL);
    }
    
    int champ=0;
    printf("\n\n*** RESUMO DAS VENDAS ***\n\n");
    
    for(i=1; i<=P; i++) {
    	printf("[Produto %d] Vendas: %d\n", i, (vendas[i]));
    	if(vendas[i]>champ)
    		champ = i;
	}
	
	printf("\n\n*** TEMOS UM VENCEDOR!! ***\n PRODUTO %d, COM %d VENDAS!", champ, vendas[champ]);
    return champ;
}

