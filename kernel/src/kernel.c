#include <planificacion.h>

void sigint_handler(int sig);
int socket_kernel_escucha;

t_config *config_kernel;

t_log *logger_kernel;
char *ip_kernel;
char *ip_filesystem;
char *ip_memoria;
char *ip_cpu;
char *puerto_memoria;
char *puerto_filesystem;
char *puerto_escucha;
char *puerto_cpu;
char *ip;
int grado_multiprogramacion = 1;
double hrrn_alfa;
int estimacion_inicial;
char **recursos;
char **cant_recursos;
char *algoritmo_planificacion;
uint32_t conexion_memoria, conexion_filesystem, conexion_cpu;

int cliente_dispatch;

sem_t semFileSystem;
sem_t semMultiprogramacion;
sem_t semProcesoNuevo;
sem_t semProcesoListo;
sem_t compactacion;
t_list *tabla_global_archivos_abiertos;

int estimado_rafaga;
void atender_consolas_nuevas(void *conexion);



int main(void)
{
	levantar_config();

	log_info(logger_kernel, "Iniciando kernel");
	conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, logger_kernel);
	send(conexion_memoria, "kernel", strlen("kernel") + 1, 0); // mensaje para que memoria sepa quién se conecta
	conexion_filesystem = crear_conexion(ip_filesystem, puerto_filesystem, logger_kernel);
	
	iniciar_listas_y_semaforos();
	iniciar_recursos(recursos, cant_recursos);
	iniciar_archivos();
	
	crear_hilo_planificador_largo_plazo();
	crear_hilo_planificador_corto_plazo();
	iniciar_file_system();

	socket_kernel_escucha = iniciar_servidor(logger_kernel, "KERNEL", ip_kernel, puerto_escucha);

	pthread_t hilo_escucha;

	while (1)
	{
		int conexion = esperar_cliente(socket_kernel_escucha, logger_kernel);
		pthread_create(&hilo_escucha, NULL, (void *)atender_consolas_nuevas, (void *)(intptr_t)conexion);
		pthread_detach(hilo_escucha);
		log_info(logger_kernel, "Se creo un hilo para atender la nueva peticion");
	}

	log_info(logger_kernel, "Se cerrará la conexión");
	list_destroy(tabla_global_archivos_abiertos);
	terminar_programa(socket_kernel_escucha, logger_kernel, config_kernel);
}


void levantar_config()
{
	config_kernel = config_create(CONFIG_FILE_PATH);
	logger_kernel = log_create(LOG_FILE_PATH, "KERNEL", true, LOG_LEVEL_INFO);
	ip_kernel = config_get_string_value(config_kernel, "IP_KERNEL");
	ip_filesystem = config_get_string_value(config_kernel, "IP_FILESYSTEM");
	ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
	ip_cpu = config_get_string_value(config_kernel, "IP_CPU");
	puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
	puerto_filesystem = config_get_string_value(config_kernel, "PUERTO_FILESYSTEM");
	puerto_escucha = config_get_string_value(config_kernel, "PUERTO_ESCUCHA");
	puerto_cpu = config_get_string_value(config_kernel, "PUERTO_CPU");
	estimado_rafaga = config_get_int_value(config_kernel, "ESTIMACION_INICIAL");
	grado_multiprogramacion = config_get_int_value(config_kernel, "GRADO_MAX_MULTIPROGRAMACION");
	recursos = config_get_array_value(config_kernel, "RECURSOS");
	cant_recursos = config_get_array_value(config_kernel, "INSTANCIAS_RECURSOS");
	hrrn_alfa = config_get_double_value(config_kernel, "HRRN_ALFA");
	algoritmo_planificacion = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
	//string_get_string_as_array para los recursos
}


void atender_consolas_nuevas(void* conexion)
{
    //bool exit = false;
    intptr_t conexionConsola = (intptr_t) conexion;
    t_paquete* paquete = get_paquete(conexionConsola, logger_kernel); 
    t_pcb* pcb = crear_pcb(conexion, get_instrucciones(paquete), NUEVO, estimado_rafaga); 
    t_procesoMemoria* procesoDeMemoria = solicitarCreacionDeProcesoEnMemoria(pcb->pid);
	pcb->tabla_segmentos = procesoDeMemoria->tabla_segmento;
	pcb->tamanio_tabla = procesoDeMemoria->tamanio_tabla;
    log_info(logger_kernel, "Se crea el proceso %d en NEW", pcb->pid);
    agregar_pcb_cola_nuevos(pcb);
    destruir_paquete(paquete);
}