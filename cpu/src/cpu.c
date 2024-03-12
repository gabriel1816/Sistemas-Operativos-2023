#include <cpu.h>

int conexion_memoria;
int cliente_dispatch;
t_log *logger_cpu;
int retardo_instruccion;
char *ip_cpu;
char *ip_memoria;
char *puerto_escucha;
char *puerto_memoria;
t_temporal* temporizador;
sem_t semConexion;
t_config* config_cpu;
uint32_t tamanio_maximo_segmentos;



int main(void)
{

	logger_cpu = log_create(LOG_FILE_PATH, "CPU", true, LOG_LEVEL_INFO);
	levantar_config();
	send(conexion_memoria, "cpu", strlen("cpu") + 1, 0); // mensaje para que memoria sepa quién se conecta
	// sem_init(&semConexion, 0, 1);
	int socket_cpu_escucha = iniciar_servidor(logger_cpu, "CPU", ip_cpu, puerto_escucha);

	pthread_t hilo_escucha;

	while (1)
	{
		int conexion = esperar_cliente(socket_cpu_escucha, logger_cpu);
		pthread_create(&hilo_escucha, NULL, (void *)atender_clientes, (void *)(intptr_t)conexion);
		pthread_detach(hilo_escucha);
	}

	log_info(logger_cpu, "Se cerrara la conexion");
	close(socket_cpu_escucha);
	log_destroy(logger_cpu);
	config_destroy(config_cpu);
}


void atender_clientes(void *conexion)
{

	intptr_t una_conexion = (intptr_t)conexion;
	// log_info(logger_cpu, "Se conecto un cliente");
	t_pcb *pcb = recibir_pcb(una_conexion, logger_cpu);
	temporizador = temporal_create();
	comenzar_ciclo_instruccion(pcb, una_conexion);
	log_info(logger_cpu, "Se cerrara la conexion (Kernel - CPU) ");
	destruir_pcb(pcb);
	close(una_conexion);
}