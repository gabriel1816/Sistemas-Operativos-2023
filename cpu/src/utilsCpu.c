#include <cpu.h>

// ------------------------------------------------------------------------------------------
// -- Config --
// ------------------------------------------------------------------------------------------ 

void levantar_config()
{
	config_cpu = config_create(CONFIG_FILE_PATH);
	ip_cpu = config_get_string_value(config_cpu, "IP_CPU");
	ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
	puerto_escucha = config_get_string_value(config_cpu, "PUERTO_ESCUCHA");
	puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
	retardo_instruccion = config_get_int_value(config_cpu, "RETARDO_INSTRUCCION");
	tamanio_maximo_segmentos = config_get_int_value(config_cpu, "TAM_MAX_SEGMENTO");
	conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, logger_cpu);
}



// ------------------------------------------------------------------------------------------
// -- Registro --
// ------------------------------------------------------------------------------------------

void insertar_nuevo_valor_registro(t_registros *registros, char *registro, char *nuevo_valor)
{ 
	if (string_equals_ignore_case(registro, "AX"))
	{
		strncpy(registros->AX, nuevo_valor, 4);
		registros->AX[sizeof(registros->AX) - 1] = '\0';
	} // string_equals_ignore_case: compara 2 strings, ignorando las diferencias con mayúsculas o minúsculas
	if (string_equals_ignore_case(registro, "BX"))
	{
		strncpy(registros->BX, nuevo_valor, 4);
		registros->BX[sizeof(registros->BX) - 1] = '\0';
	}
	if (string_equals_ignore_case(registro, "CX"))
	{
		strncpy(registros->CX, nuevo_valor, 4);
		registros->CX[sizeof(registros->CX) - 1] = '\0';
	}
	if (string_equals_ignore_case(registro, "DX"))
	{
		strncpy(registros->DX, nuevo_valor, 4);
		registros->DX[sizeof(registros->DX) - 1] = '\0';
	}
	if (string_equals_ignore_case(registro, "EAX"))
	{
		strncpy(registros->EAX, nuevo_valor, 8);
		registros->EAX[sizeof(registros->EAX) - 1] = '\0';
	}
	if (string_equals_ignore_case(registro, "EBX"))
	{
		strncpy(registros->EBX, nuevo_valor, 8);
		registros->EBX[sizeof(registros->EBX) - 1] = '\0';
	}
	if (string_equals_ignore_case(registro, "ECX"))
	{
		strncpy(registros->ECX, nuevo_valor, 8);
		registros->ECX[sizeof(registros->ECX) - 1] = '\0';
	}
	if (string_equals_ignore_case(registro, "EDX"))
	{
		strncpy(registros->EDX, nuevo_valor, 8);
		registros->EDX[sizeof(registros->EDX) - 1] = '\0';
	}
	if (string_equals_ignore_case(registro, "RAX"))
	{
		strncpy(registros->RAX, nuevo_valor, 16);
		registros->RAX[sizeof(registros->RAX) - 1] = '\0';
	}
	if (string_equals_ignore_case(registro, "RBX"))
	{
		strncpy(registros->RBX, nuevo_valor, 16);
		registros->RBX[sizeof(registros->RBX) - 1] = '\0';
	}
	if (string_equals_ignore_case(registro, "RCX"))
	{
		strncpy(registros->RCX, nuevo_valor, 16);
		registros->RCX[sizeof(registros->RCX) - 1] = '\0';
	}
	if (string_equals_ignore_case(registro, "RDX"))
	{
		strncpy(registros->RDX, nuevo_valor, 16);
		registros->RDX[sizeof(registros->RDX) - 1] = '\0';
	}
}


char* consultar_valor_registro(t_registros *registros, char *registro) 
{ 
	if (string_equals_ignore_case(registro, "AX"))
	{
		return registros->AX;
	} 
	if (string_equals_ignore_case(registro, "BX"))
	{
		return registros->BX;
	}
	if (string_equals_ignore_case(registro, "CX"))
	{
		return registros->CX;
	}
	if (string_equals_ignore_case(registro, "DX"))
	{
		return registros->DX;
	}
	if (string_equals_ignore_case(registro, "EAX"))
	{
		return registros->EAX;
	}
	if (string_equals_ignore_case(registro, "EBX"))
	{
		return registros->EBX;
	}
	if (string_equals_ignore_case(registro, "ECX"))
	{
		return registros->ECX;
	}
	if (string_equals_ignore_case(registro, "EDX"))
	{
		return registros->EDX;
	}
	if (string_equals_ignore_case(registro, "RAX"))
	{
		return registros->RAX;
	}
	if (string_equals_ignore_case(registro, "RBX"))
	{
		return registros->RBX;
	}
	if (string_equals_ignore_case(registro, "RCX"))
	{
		return registros->RCX;
	}
	if (string_equals_ignore_case(registro, "RDX"))
	{
		return registros->RDX;
	}
}


int consultar_tamanio_registro(t_registros registros, char *registro) 
{ 
	if (string_equals_ignore_case(registro, "AX"))
	{
		return 4;
	} 
	if (string_equals_ignore_case(registro, "BX"))
	{
		return 4;
	}
	if (string_equals_ignore_case(registro, "CX"))
	{
		return 4;
	}
	if (string_equals_ignore_case(registro, "DX"))
	{
		return 4;
	}
	if (string_equals_ignore_case(registro, "EAX"))
	{
		return 8;
	}
	if (string_equals_ignore_case(registro, "EBX"))
	{
		return 8;
	}
	if (string_equals_ignore_case(registro, "ECX"))
	{
		return 8;
	}
	if (string_equals_ignore_case(registro, "EDX"))
	{
		return 8;
	}
	if (string_equals_ignore_case(registro, "RAX"))
	{
		return 16;
	}
	if (string_equals_ignore_case(registro, "RBX"))
	{
		return 16;
	}
	if (string_equals_ignore_case(registro, "RCX"))
	{
		return 16;
	}
	if (string_equals_ignore_case(registro, "RDX"))
	{
		return 16;
	}
}


void imprimir_registros_por_pantalla(t_registros registros)
{
	printf("-------------------\n");
	printf("Valores de los Registros:\n");
	printf(" + AX = %s\n", registros.AX);
	printf(" + BX = %s\n", registros.BX);
	printf(" + CX = %s\n", registros.CX);
	printf(" + DX = %s\n", registros.DX);
	printf(" + EAX = %s\n", registros.EAX);
	printf(" + EBX = %s\n", registros.EBX);
	printf(" + ECX = %s\n", registros.ECX);
	printf(" + EDX = %s\n", registros.EDX);
	printf(" + RAX = %s\n", registros.RAX);
	printf(" + RBX = %s\n", registros.RBX);
	printf(" + RCX = %s\n", registros.RCX);
	printf(" + RDX = %s\n", registros.RDX);
	printf("-------------------\n");
}



// ------------------------------------------------------------------------------------------
// -- Ciclos de instruccion --
// ------------------------------------------------------------------------------------------



void solicitar_MOV_OUT(uint32_t pid, int32_t direccion_fisica, char* registro) 
{ 
    t_list* parametros = list_create();
	char* pidString = string_itoa(pid);
    char* direccion_fisica_string = string_itoa(direccion_fisica);
	list_add(parametros, pidString);
	list_add(parametros, direccion_fisica_string);
	list_add(parametros, registro);
    t_instruccion* pedidoMemoria = crear_instruccion(MOV_OUT, parametros);
    enviarPedidoAMemoria(pedidoMemoria, conexion_memoria, logger_cpu);

	t_paquete *paquete = get_paquete(conexion_memoria, logger_cpu);
    if(paquete->codigo_operacion == MOV_OUT_OK) log_info(logger_cpu, "El MOV_OUT resulto exitoso");
    destruir_paquete(paquete);
	list_destroy(parametros);
	free(pidString);
	free(direccion_fisica_string);
	return;
}



char* solicitar_MOV_IN(uint32_t pid, int32_t direccion_fisica, uint32_t tamanio_registro) 
{ 
    t_list* parametros = list_create();
	char* pidString = string_itoa(pid);
	char* tamanio_registro_string = string_itoa(tamanio_registro);
    char* direccion_fisica_string = string_itoa(direccion_fisica);
	list_add(parametros, pidString);
    list_add(parametros, direccion_fisica_string);
	list_add(parametros, tamanio_registro_string);
    t_instruccion* pedidoMemoria = crear_instruccion(MOV_IN, parametros);
    enviarPedidoAMemoria(pedidoMemoria, conexion_memoria, logger_cpu);

	t_paquete *paquete = get_paquete(conexion_memoria, logger_cpu);
	int offset=0;
    t_instruccion* respuestaMemoria = crear_instruccion_para_el_buffer(paquete->buffer,&offset);
	list_destroy(parametros);
	free(pidString);
	free(direccion_fisica_string);
	free(tamanio_registro_string);
    destruir_paquete(paquete);
	return respuestaMemoria->parametros[0];

}


void liberar_registros(t_registros *registros)
{
	free(registros);
}


void leer_instruccion_y_ejecutar_pcb(t_instruccion *instruccion_aux, t_pcb *pcb, int conexion_kernel)
{
	int32_t direccion_fisica, numeroSegmento;
	uint32_t tamanio;
	int tamanio_registro;

	switch (instruccion_aux->identificador)
	{
	case SET:
		usleep(retardo_instruccion * 1000); // utilizamos este ya que recibe por parametro al tiempo en milisegundos
		log_info(logger_cpu, "PID: %u - Ejecutando: SET - %s - %s", pcb->pid, instruccion_aux->parametros[0], instruccion_aux->parametros[1]);
		insertar_nuevo_valor_registro(&pcb->registros, instruccion_aux->parametros[0], instruccion_aux->parametros[1]);
		imprimir_registros_por_pantalla(pcb->registros);
		break;

	case MOV_IN:
		log_info(logger_cpu, "PID: %u - Ejecutando: MOV_IN - %s - %s", pcb->pid, instruccion_aux->parametros[0], instruccion_aux->parametros[1]);
		tamanio_registro = consultar_tamanio_registro(pcb->registros, instruccion_aux->parametros[0]) + 1;
		direccion_fisica = traducir_direccion(atoi(instruccion_aux->parametros[1]), pcb, conexion_kernel, (uint32_t)tamanio_registro);
		if(direccion_fisica==-1) return;
		char* valor_leido = solicitar_MOV_IN(pcb->pid, direccion_fisica, tamanio_registro);
		log_info(logger_cpu, "PID: %u - Acción: LEER - Segmento: %u - Direccion Fisica: %d - Valor: %s", pcb->pid, obtener_numero_segmento(atoi(instruccion_aux->parametros[1])), direccion_fisica, valor_leido);
		insertar_nuevo_valor_registro(&pcb->registros, instruccion_aux->parametros[0], valor_leido);
		imprimir_registros_por_pantalla(pcb->registros);
		break;

	case MOV_OUT:
		log_info(logger_cpu, "PID: %u - Ejecutando: MOV_OUT - %s - %s", pcb->pid, instruccion_aux->parametros[0], instruccion_aux->parametros[1]);
		tamanio_registro = consultar_tamanio_registro(pcb->registros, instruccion_aux->parametros[1]) + 1;
		direccion_fisica = traducir_direccion(atoi(instruccion_aux->parametros[0]), pcb, conexion_kernel, (uint32_t)tamanio_registro);
		if(direccion_fisica == -1) return;
		char* valor_escrito = consultar_valor_registro(&(pcb->registros), instruccion_aux->parametros[1]);
		solicitar_MOV_OUT(pcb->pid, direccion_fisica, valor_escrito);
		log_info(logger_cpu, "PID: %u - Acción: ESCRIBIR - Segmento: %u - Direccion Fisica: %d - Valor: %s", pcb->pid, obtener_numero_segmento(atoi(instruccion_aux->parametros[0])), direccion_fisica, valor_escrito);
		break;

	case I_O:
		log_info(logger_cpu, "PID: %u - Ejecutando: IO ");
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;

	case F_OPEN:
    	log_info(logger_cpu, "PID: %u - Ejecutando: F_OPEN - %s", pcb->pid, instruccion_aux->parametros[0]);
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;

	case F_CLOSE:
    	log_info(logger_cpu, "PID: %u - Ejecutando: F_CLOSE - %s", pcb->pid, instruccion_aux->parametros[0]);
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;
	
	case F_SEEK:
    	log_info(logger_cpu, "PID: %u - Ejecutando: F_SEEK - %s - %s", pcb->pid, instruccion_aux->parametros[0], instruccion_aux->parametros[1]);
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;

	case F_READ:
    	log_info(logger_cpu, "PID: %u - Ejecutando: F_READ - %s - %s - %s", pcb->pid, instruccion_aux->parametros[0], instruccion_aux->parametros[1], instruccion_aux->parametros[2]);
		direccion_fisica = traducir_direccion((uint32_t)atoi(instruccion_aux->parametros[1]), pcb, conexion_kernel, (uint32_t)atoi(instruccion_aux->parametros[2]));
    	pcb->direccion_fisica = direccion_fisica;
		if(direccion_fisica==-1) return;
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;
	
	case F_WRITE:
    	log_info(logger_cpu, "PID: %u - Ejecutando: F_WRITE - %s - %s - %s", pcb->pid, instruccion_aux->parametros[0], instruccion_aux->parametros[1], instruccion_aux->parametros[2]);
		direccion_fisica = traducir_direccion((uint32_t)atoi(instruccion_aux->parametros[1]), pcb, conexion_kernel, (uint32_t)atoi(instruccion_aux->parametros[2]));
		pcb->direccion_fisica = direccion_fisica;
		if(direccion_fisica==-1) return;
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;

	case F_TRUNCATE:
    	log_info(logger_cpu, "PID: %u - Ejecutando: F_TRUNCATE - %s - %s", pcb->pid, instruccion_aux->parametros[0], instruccion_aux->parametros[1]);
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;
	
	case WAIT:
    	log_info(logger_cpu, "PID: %u - Ejecutando: WAIT - %s", pcb->pid, instruccion_aux->parametros[0]);		
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;
	
	case SIGNAL:
		log_info(logger_cpu, "PID: %u - Ejecutando: SIGNAL - %s", pcb->pid, instruccion_aux->parametros[0]);
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;
	
	case CREATE_SEGMENT:
    	log_info(logger_cpu, "PID: %u - Ejecutando: CREATE_SEGMENT - %s - %s", pcb->pid, instruccion_aux->parametros[0], instruccion_aux->parametros[1]);
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;
		
	case DELETE_SEGMENT:
    	log_info(logger_cpu, "PID: %u - Ejecutando: DELETE_SEGMENT - %s", pcb->pid, instruccion_aux->parametros[0]);
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;
	
	case YIELD:
		log_info(logger_cpu, "PID: %u - Ejecutando: YIELD ", pcb->pid);
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;

	case EXIT:
		log_info(logger_cpu, "PID: %u - Ejecutando: EXIT ", pcb->pid);
		enviar_pcb(pcb, conexion_kernel, logger_cpu);
		temporal_stop(temporizador);
		enviar_contador(temporizador, conexion_kernel);
		temporal_destroy(temporizador);
		return;
		break;

	default:
		log_error(logger_cpu, "Instruccion inválida :( ");
	}

	log_info(logger_cpu, "Actualización del Program Counter");
	comenzar_ciclo_instruccion(pcb, conexion_kernel);
}


void comenzar_ciclo_instruccion(t_pcb *pcb, int conexion_kernel)
{
	log_trace(logger_cpu, "Comienza ciclo de instrucción para ejecución de proceso %u", pcb->pid);
	t_instruccion *instruccion_aux = siguiente_instruccion(pcb);
	leer_instruccion_y_ejecutar_pcb(instruccion_aux, pcb, conexion_kernel);
}


t_instruccion *siguiente_instruccion(t_pcb *pcb)
{
	
	t_instruccion *instruccion_aux = list_get(pcb->instrucciones, pcb->program_counter); // busco en la lista a traves del program counter (determina la posicion de la instruccion a analizar)
	pcb->program_counter += 1;
	return instruccion_aux;
}


void enviar_contador(t_temporal *temporizador, int conexion_kernel)
{
	int64_t tiempoready = temporal_gettime(temporizador);
	char respuesta[20];
	sprintf(respuesta, "%" PRId64, tiempoready);
	send(conexion_kernel, respuesta, strlen(respuesta), 0);
}


void procesarsolicitud(int32_t direccionfisica, int conexion_kernel) 
{
	char* mensajeRamdom[10];
	recv(conexion_kernel, &mensajeRamdom, sizeof(mensajeRamdom), 0);	
	send(conexion_kernel, &direccionfisica, sizeof(direccionfisica), 0);
}
