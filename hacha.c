#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

void partir(char *nombre, int tamanyo);
int calcula_trozos(char *nombre, int tamanyo);
void genera_nombre(char *dest, char *nombre, int i);
void proceso_padre(int fEntrada, int tubo[2], char *trozoLeido, int tamanyo);
void proceso_hijo(int tubo[2], char *trozoLeido, int tamanyo, char *nombre, int i);
void espera_hijos(int n);

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("Error. Uso: %s fichero tam_trozo\n", argv[0]);
	}
	else{
		partir(argv[1], atoi(argv[2])); // nombre del fichero y tamaño.
	}
	return 0;
}

// numero de trozos = total del fichero / tamaño del trozo (+1 si sobra resto)
int calcula_trozos(char *nombre, int tamanyo){
	struct stat propFichero;
	int numTrozos;

	stat(nombre, &propFichero);
	numTrozos = propFichero.st_size / tamanyo;
	if(propFichero.st_size % tamanyo != 0){
		numTrozos++;
	}
	return numTrozos;
}

// nombre.h00, nombre.h01, ...
void genera_nombre(char *dest, char *nombre, int i){
	sprintf(dest, "%s.h%02d", nombre, i);
}

// el padre lee del fichero original y escribe en el tubo.
void proceso_padre(int fEntrada, int tubo[2], char *trozoLeido, int tamanyo){
	int numLeidos;

	numLeidos = read(fEntrada, trozoLeido, tamanyo);
	write(tubo[1], trozoLeido, numLeidos);
}

// el hijo lee del tubo y escribe su trozo en un fichero.
void proceso_hijo(int tubo[2], char *trozoLeido, int tamanyo, char *nombre, int i){
	char nombreFichero[50];
	int fSalida, numLeidos;

	genera_nombre(nombreFichero, nombre, i);
	fSalida = creat(nombreFichero, 0666);
	numLeidos = read(tubo[0], trozoLeido, tamanyo);
	write(fSalida, trozoLeido, numLeidos);
	close(fSalida);
	free(trozoLeido);
	exit(0);
}

void espera_hijos(int n){
	int i;
	for(i = 0; i < n; i++){
		wait(NULL);
	}
}

void partir(char nombre[], int tamanyo){
	int i, numTrozos, fEntrada;
	int tubo[2]; // = {lectura, escritura}
	char *trozoLeido;

	fEntrada = open(nombre, O_RDONLY);
	if(fEntrada < 0){
		printf("Error. No existe el fichero\n");
	}
	else{
		trozoLeido = (char *) malloc(sizeof(char) * tamanyo);
		numTrozos = calcula_trozos(nombre, tamanyo);
		for(i = 0; i < numTrozos; i++){
			pipe(tubo); // tuberia distinta para cada hijo.
			if(fork() != 0){
				proceso_padre(fEntrada, tubo, trozoLeido, tamanyo);
			}
			else{
				proceso_hijo(tubo, trozoLeido, tamanyo, nombre, i);
			}
		}
		free(trozoLeido);
		espera_hijos(numTrozos);
	}
}
