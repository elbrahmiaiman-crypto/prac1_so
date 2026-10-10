#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void despierta(){}

/*
	./hijos 4 3

	hijos
	|                     0   1   2   3
	x1		vx = [x1, x2, x3, x4]
	|                     0   1   2
	x2		vy = [y1, y2, y3]
	|
	x3
	|
	x4
      /  | \
    y1  y2  y3	=> alarm(10)
*/

// crea un vector compartido de n enteros; devuelve su id y deja en *v su direccion.
int crea_vector(int n, int **v){
	int shmid = shmget(IPC_PRIVATE, sizeof(int) * n, IPC_CREAT | 0666);
	*v = (int *) shmat(shmid, 0, 0);
	return shmid;
}

// desengancha y elimina un vector compartido.
void libera_vector(int shmid, int *v){
	shmdt(v);
	shmctl(shmid, IPC_RMID, NULL);
}

// el proceso de nivel 'nivel' muestra a sus padres.
void muestra_padres(int nivel, int *vx, pid_t pid_hijos){
	int j;
	printf("Soy el proceso %d. Mis padres son: ", getpid());
	printf("%d", pid_hijos);
	for(j = 0; j < nivel - 1; j++){
		printf(", %d", vx[j]);
	}
	printf("\n");
}

// crea la cadena vertical. Devuelve el nivel del proceso que retorna:
// 0 = super padre, 1..x = procesos de la vertical.
int crea_vertical(int x, int *vx, pid_t pid_hijos){
	int nivel = 0;
	while(nivel < x){
		if(fork() != 0){
			wait(NULL);
			break;
		}
		nivel++;
		vx[nivel - 1] = getpid();
		muestra_padres(nivel, vx, pid_hijos);
	}
	return nivel;
}

// proceso horizontal: guarda su pid, espera la alarma y muere.
void proceso_final(int i, int *vy){
	vy[i - 1] = getpid();
	signal(SIGALRM, despierta);
	alarm(10);
	pause();
	exit(0);
}

void espera_hijos(int n){
	int i;
	for(i = 0; i < n; i++){
		wait(NULL);
	}
}

// el ultimo hijo vertical crea los y hijos horizontales.
void crea_horizontal(int y, int *vy){
	int i;
	for(i = 1; i <= y; i++){
		if(fork() == 0){
			proceso_final(i, vy);
		}
	}
	espera_hijos(y);
}

void super_padre(int y, int *vy){
	int i;
	printf("Soy el super padre %d, mis hijos finales son: ", getpid());
	for(i = 0; i < y; i++){
		printf("%d", vy[i]);
		if(i != y - 1){
			printf(", ");
		}
	}
	printf("\n");
}

int main(int argc, char *argv[]){
	int x, y, nivel, shmidx, shmidy;
	int *vx, *vy;
	pid_t pid_hijos;

	if(argc != 3){
		printf("error en argumentos\n");
	}
	else{
		pid_hijos = getpid();
		x = atoi(argv[1]);
		y = atoi(argv[2]);

		shmidx = crea_vector(x, &vx);
		shmidy = crea_vector(y, &vy);

		nivel = crea_vertical(x, vx, pid_hijos);
		if(nivel == 0){
			super_padre(y, vy);
			libera_vector(shmidx, vx);
			libera_vector(shmidy, vy);
		}
		else if(nivel == x){
			crea_horizontal(y, vy);
		}
	}
	return 0;
}
