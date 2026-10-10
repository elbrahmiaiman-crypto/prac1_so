#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

// variables globales donde se almacenan los pids
pid_t arb, a, b, x, y, z, pid;
int tiempo;

/* ===================== MANEJADORAS ===================== */

// esto lo hace A
void ejecuta_pstree(){
	char arb_cad[100];
	sprintf(arb_cad, "%d", arb);
	if(fork() == 0){
		// el proceso se convierte en el comando.
		execlp("pstree", "pstree", "-c", arb_cad, NULL);
	}
	else{
		wait(NULL); // para que el hijo acabe el comando
		printf("mandando señal a arb: %d\n", arb);
		kill(arb, SIGUSR2);	// para que inicie la destruccion del arbol.
		pause();
	}
}

void destroy_ejec(){
	printf("ejec => arb\n");
	kill(a, SIGUSR2);
}

void destroy_A(){
	printf("A => B\n");
	kill(b, SIGUSR2);
}

void nada(){
}

/* ===================== PROCESOS HOJA ===================== */

// X e Y: se presentan, esperan la destruccion y mueren.
void hijo_espera_destruccion(char nombre){
	printf("Soy el proceso %c: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo %d\n", nombre, getpid(), b, a, arb);
	signal(SIGUSR2, nada);
	pause();
	printf("Soy %c (%d) y muero\n", nombre, getpid());
	exit(0);
}

// Z: espera 'tiempo' segundos, avisa a A y espera la destruccion.
void proceso_Z(){
	printf("Soy el proceso Z: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo %d\n", getpid(), b, a, arb);
	signal(SIGALRM, nada);
	alarm(tiempo);
	pause();
	kill(a, SIGUSR1);
	signal(SIGUSR2, nada);
	pause();
	printf("Soy Z (%d) y muero\n", getpid());
	exit(0);
}

/* ===================== PROCESO B ===================== */

void crea_hijos_B(){
	x = fork();
	if(x == 0) hijo_espera_destruccion('X');
	y = fork();
	if(y == 0) hijo_espera_destruccion('Y');
	z = fork();
	if(z == 0) proceso_Z();
}

void destruye_hijos_B(){
	printf("B manda señales a hijos\n");
	kill(x, SIGUSR2);
	wait(NULL);
	kill(y, SIGUSR2);
	wait(NULL);
	kill(z, SIGUSR2);
	wait(NULL);
}

void proceso_B(){
	b = getpid();
	printf("Soy el proceso B: mi pid es %d. Mi padre es %d. Mi abuelo es %d\n", b, a, arb);
	crea_hijos_B();
	signal(SIGUSR2, nada);
	pause(); // me espero a la destruccion.
	destruye_hijos_B();
	exit(0); // B muere
}

/* ===================== PROCESO A ===================== */

void proceso_A(){
	a = getpid();
	printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n", a, arb);
	b = fork();
	if(b == 0){
		proceso_B();
	}
	signal(SIGUSR1, ejecuta_pstree);
	signal(SIGUSR2, destroy_A);
	pause(); // aqui me llega SIGUSR1
	wait(NULL);
	printf("Soy A (%d) y muero\n", a);
	exit(0);
}

/* ===================== PROCESO EJEC ===================== */

void proceso_ejec(){
	signal(SIGUSR2, destroy_ejec);
	pause(); // espero
	wait(NULL);
	printf("Soy ejec (%d) y muero\n", arb);
	exit(0);
}

/*
	argv[0] = "./ejec"
	argv[1] = "123"
*/
int main(int argc, char *argv[]){
	if(argc != 2){
		printf("Usage: ./ejec tiempo\n");
	}
	else{
		tiempo = atoi(argv[1]);
		arb = getpid();
		printf("Soy el proceso ejec: mi pid es %d\n", arb);
		a = fork();
		if(a != 0){
			proceso_ejec();
		}
		else{
			proceso_A();
		}
	}
	return 0;
}
