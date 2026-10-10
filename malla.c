#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void nada(){
}

// ultimo proceso de la columna: espera la alarma y muere.
void proceso_ultimo(){
	signal(SIGALRM, nada);
	alarm(10);
	pause();
	exit(0);
}

// crea una columna de x procesos (el que la inicia es el primero).
void crea_columna(int x){
	int j;
	for(j = 1; j <= x - 1; j++){
		if(fork() != 0){
			wait(NULL);
			exit(0);
		}
	}
	proceso_ultimo();
}

void espera_hijos(int n){
	int i;
	for(i = 1; i <= n; i++){
		wait(NULL);
	}
}

// crea y columnas de x procesos cada una.
void crea_malla(int x, int y){
	int i;
	for(i = 1; i <= y; i++){
		if(fork() == 0){
			crea_columna(x);
		}
	}
	espera_hijos(y);
}

int main(int argc, char *argv[]){
	int x, y;

	if(argc == 3){
		x = atoi(argv[1]);
		y = atoi(argv[2]);
		if(x > 0 && y > 0){
			crea_malla(x, y);
		}
	}
	return 0;
}
