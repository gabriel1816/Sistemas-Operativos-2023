#include "pcb.h"

uint32_t size_registros = sizeof(char[5])*4 + sizeof(char[9])*4 + sizeof(char[17])*4;



// ------------------------------------------------------------------------------------------
// -- Registros --
// ------------------------------------------------------------------------------------------

t_registros iniciar_registros()
{
	t_registros registros;
    strcpy(registros.AX,"\0");
	strcpy(registros.BX,"\0");
	strcpy(registros.CX,"\0");
	strcpy(registros.DX,"\0");
	strcpy(registros.EAX,"\0");
	strcpy(registros.EBX,"\0");
	strcpy(registros.ECX,"\0");
	strcpy(registros.EDX,"\0");
	strcpy(registros.RAX,"\0");
	strcpy(registros.RBX,"\0");
	strcpy(registros.RCX,"\0");
	strcpy(registros.RDX,"\0");
	return registros;
}



// ------------------------------------------------------------------------------------------
// -- Creacion / Eliminacion --
// ------------------------------------------------------------------------------------------

t_pcb* crear_pcb(intptr_t pid, t_list* lista_instrucciones, t_cod_estado estado, int32_t estimado_rafaga)
{
	t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->pid = pid;
    pcb->instrucciones = lista_instrucciones; // le paso por parámetro la lista de instrucciones
	pcb->program_counter = 0; // ip comienza en 0 arbitrariamente
	pcb->estado = estado; // pasa por parámetros de momento desconocemos origen
    pcb->registros = iniciar_registros();
	pcb->tabla_segmentos = NULL; 
	pcb->tamanio_tabla = 0;
	pcb->estimado_rafaga = estimado_rafaga; 
    //pcb->temporizador; no la uso aca, la uso en plani
    pcb->tabla_archivos = list_create(); // ver, por ahora no es algo que sepamos bien la estructura
	pcb->direccion_fisica = 0;
	return pcb;
}


void destruir_pcb(t_pcb* pcb)
{
	
    list_destroy_and_destroy_elements(pcb->instrucciones, (void*)destruir_instruccion);
    free(pcb->tabla_segmentos); 
	list_destroy(pcb->tabla_archivos);
	free(pcb);
}



// ------------------------------------------------------------------------------------------
// -- Envío de pcbs --
// ------------------------------------------------------------------------------------------

void enviar_pcb(t_pcb* pcb, int socket, t_log* logger) 
{
	t_buffer* buffer = crear_buffer_envio_pcb(pcb, logger);
	t_paquete* paquete = crear_paquete(buffer, CODIGO_PCB);
	enviar_paquete(socket, paquete, logger);
	destruir_paquete(paquete);
}



// ------------------------------------------------------------------------------------------
// -- Serializar pcb --
// ------------------------------------------------------------------------------------------

t_buffer* crear_buffer_envio_pcb(t_pcb* pcb, t_log* logger)
{

	t_buffer* buffer = buffer_create();
	t_buffer* buffer_instrucciones = crear_buffer__para_t_lista_instrucciones(pcb->instrucciones);
	
	uint32_t size_total = sizeof(uint32_t)*4  
						//+ sizeof(int32_t)*2
						+ size_registros
						+ sizeof(int32_t)
						+ (sizeof(int32_t) + sizeof(uint32_t) * 2) * pcb->tamanio_tabla  // (id + base + segmento) * cuan grande es la tabla 
						+ buffer_instrucciones->size; 

	buffer->size = size_total;
	
	//TODO el stream de las instrucciones tiene que estar  al final del buffer del pcb ya que no conocemos el offset de las instrucciones

	void* stream = malloc(buffer->size);
	int offset = 0; // Desplazamiento
	memcpy(stream + offset, &pcb->pid, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &pcb->program_counter, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &pcb->estado, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &pcb->direccion_fisica, sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(stream + offset, &pcb->tamanio_tabla, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    for(int i = 0; i < pcb->tamanio_tabla; i++)
    {
        t_segmento segmento = pcb->tabla_segmentos[i];
        memcpy(stream + offset, &segmento.id, sizeof(int32_t));
        offset += sizeof(int32_t);
        memcpy(stream + offset, &segmento.base, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(stream + offset, &segmento.limite, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    }
	//memcpy(stream + offset, &pcb->tabla_archivos, sizeof(int));
	//offset += sizeof(int);
	//memcpy(stream + offset, &pcb->estimado_rafaga, sizeof(int32_t));
	//offset += sizeof(int32_t);
	//memcpy(stream + offset, &pcb->tiempo_llegada_ready, sizeof(int32_t));
	//offset += sizeof(int32_t);
	memcpy(stream + offset, &pcb->registros, size_registros);
	offset += size_registros;
	memcpy(stream + offset, buffer_instrucciones->stream, buffer_instrucciones->size);
	offset += buffer_instrucciones->size;

	buffer->stream = stream;
	destruir_buffer(buffer_instrucciones);
	return buffer;
}



// ------------------------------------------------------------------------------------------
// -- Para la recepción de pcbs --
// ------------------------------------------------------------------------------------------

t_pcb *recibir_pcb(int socket_cliente, t_log *logger)
{

	t_paquete *paquete = get_paquete(socket_cliente, logger);

	t_pcb *pcb = deserializar_buffer_paquete_pcb(paquete->buffer, logger);

	destruir_paquete(paquete);

	// log_info(logger, "El ID del PCB recibido es: %d", pcb->pid);

	return pcb;
}

t_pcb* deserializar_buffer_paquete_pcb(t_buffer* buffer,  t_log* logger) 
{
	t_pcb* pcb = malloc(sizeof(t_pcb));

	void* stream = buffer->stream;
	int size_restante = buffer->size;

    memcpy(&(pcb->pid), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	size_restante -= sizeof(uint32_t);
    memcpy(&(pcb->program_counter), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	size_restante -= sizeof(uint32_t);
    memcpy(&(pcb->estado), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	size_restante -= sizeof(uint32_t);
	memcpy(&(pcb->direccion_fisica), stream, sizeof(int32_t));
	stream += sizeof(int32_t);
	size_restante -= sizeof(int32_t);
    memcpy(&(pcb->tamanio_tabla), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
	size_restante -= sizeof(uint32_t);
    pcb->tabla_segmentos = malloc(sizeof(t_segmento) * pcb->tamanio_tabla);
    t_segmento* tabla_segmento = pcb->tabla_segmentos;
    for (int i = 0; i < pcb->tamanio_tabla; i++) {
        memcpy(&(tabla_segmento[i].id), stream, sizeof(int32_t));
        stream += sizeof(int32_t);
		size_restante -= sizeof(int32_t);
        memcpy(&(tabla_segmento[i].base), stream, sizeof(uint32_t));
        stream += sizeof(uint32_t);
		size_restante -= sizeof(uint32_t);
        memcpy(&(tabla_segmento[i].limite), stream, sizeof(uint32_t));
        stream += sizeof(uint32_t);
		size_restante -= sizeof(uint32_t);
    }
    //memcpy(&(pcb->tabla_archivos), stream, sizeof(int));
	//stream += sizeof(int);
	//size_restante -= sizeof(int);
	//memcpy(&(pcb->estimado_rafaga), stream, sizeof(int32_t));
	//stream += sizeof(int32_t);
	//size_restante -= sizeof(int32_t);
	//memcpy(&(pcb->tiempo_llegada_ready), stream, sizeof(int32_t));
	//stream += sizeof(int32_t);
	//size_restante -= sizeof(int32_t);
	memcpy(&(pcb->registros), stream, size_registros);
	stream += size_registros;
	size_restante -= size_registros;
	memcpy(buffer->stream, stream, size_restante);
	//free(stream);  
	buffer->size=size_restante;
	pcb->tabla_archivos = list_create();
	pcb->instrucciones = list_create();
	t_list *lista_instrucciones = crear_lista_instrucciones_para_el_buffer(buffer);
	list_add_all(pcb->instrucciones, lista_instrucciones);
	list_destroy(lista_instrucciones);
	return pcb;
}