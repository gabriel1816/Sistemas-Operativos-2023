#include "shared_utils.h"

t_log *logger;



// ------------------------------------------------------------------------------------------
// -- Paquete --
// ------------------------------------------------------------------------------------------

void *serializar_paquete(t_paquete *paquete, int bytes)
{
    void *magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento += paquete->buffer->size;

    return magic;
}


t_paquete *crear_paquete(t_buffer *buffer, int codigo_operacion)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = codigo_operacion;
    if (buffer == NULL)
        paquete->buffer = null_buffer();
    else
        paquete->buffer = buffer;
    return paquete;
}


uint32_t paquete_size(t_paquete *paquete)
{
    return paquete->buffer->size + sizeof(uint8_t) + sizeof(uint32_t);
}


bool enviar_paquete(int socket, t_paquete *paquete, t_log *logger)
{
    // Armamos el stream a enviar
    // Buffer Size + Size of Operation Code Var + Size of Buffer Size Var
    void *a_enviar = malloc(paquete->buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
    int offset = 0;
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    if (send(socket, a_enviar, paquete->buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0) == -1)
    {
        //log_error(logger, "Error al enviar el paquete");
        free(a_enviar);
        return false;
    }
    free(a_enviar);
    return true;
}


t_paquete *recibir_paquete(int socket, t_log *logger)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->stream = NULL;
    paquete->buffer->size = 0;
    paquete->codigo_operacion = 0;

    // Primero recibimos el codigo de operacion
    recv(socket, &(paquete->codigo_operacion), sizeof(uint8_t), 0);
    //log_info(logger, "Recibido codigo de operacion: %d", paquete->codigo_operacion);
    // Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
    recv(socket, &(paquete->buffer->size), sizeof(uint32_t), 0);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);

    return paquete;
}


void destruir_paquete(t_paquete *paquete)
{
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}


t_paquete *get_paquete(int socket, t_log *logger)
{
    t_paquete *paquete = recibir_paquete(socket, logger);
    return paquete;
}


int recibir_operacion(int socket_cliente)
{
    int cod_op;
    if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
        return cod_op;
    else
    {
        close(socket_cliente);
        return -1;
    }
}



// ------------------------------------------------------------------------------------------
// -- Buffer --
// ------------------------------------------------------------------------------------------

t_buffer *buffer_create()
{
    return malloc(sizeof(t_buffer));
}


void *recibir_buffer(int *size, int socket_cliente)
{
    void *buffer;

    recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
    buffer = malloc(*size);
    recv(socket_cliente, buffer, *size, MSG_WAITALL);

    return buffer;
}


t_buffer *null_buffer()
{
    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = 0;
    buffer->stream = NULL;
    return buffer;
}


void liberar_buffer(t_buffer *buffer)
{
    if (buffer != NULL)
    {
        if (buffer->stream != NULL)
        {
            free(buffer->stream);
            buffer->stream = NULL;
        }
        free(buffer);
    }
}


void destruir_buffer(t_buffer *buffer)
{
    free(buffer->stream);
    free(buffer);
}

int retardo_en_segundos(int retardo) 
{
    return retardo/1000;
}



// ------------------------------------------------------------------------------------------
// -- FileSystem --
// ------------------------------------------------------------------------------------------

t_buffer *crear_buffer_para_nombreArchivo(char *nombreArchivo) 
{
    t_buffer *buffer = malloc(sizeof(t_buffer));
    
    uint32_t nombreArchivo_length = strlen(nombreArchivo) + 1; // +1 para incluir el caracter nulo al final

    buffer->size = sizeof(uint32_t) +                         // para almacenar el tamaño de la cadena
                   nombreArchivo_length;                      // para almacenar la cadena en sí

    void *stream = calloc(buffer->size, 1);
    
    int offset = 0;
    
    memcpy(stream + offset, &nombreArchivo_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    memcpy(stream + offset, nombreArchivo, nombreArchivo_length);
    offset += nombreArchivo_length;
    
    buffer->stream = stream;
    return buffer;
}


char* crear_nombreArchivo_para_el_buffer(t_buffer *buffer, uint32_t *offset) 
{
    void *stream = buffer->stream;
    uint32_t offset_aux = 0;
    stream += (*offset);

    uint32_t nombreArchivo_length;
    memcpy(&nombreArchivo_length, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    offset_aux += sizeof(uint32_t);

    char* nombreArchivo = malloc(nombreArchivo_length);
    memcpy(nombreArchivo, stream, nombreArchivo_length);

    offset_aux += nombreArchivo_length;
    stream += nombreArchivo_length;

    *offset += offset_aux;

    return nombreArchivo;
}


t_buffer* crear_buffer_para_t_pedido_file_system(t_pedido_file_system* pedido_filesystem) 
{
    t_buffer* buffer = malloc(sizeof(t_buffer));

    uint32_t tam_instruccion = sizeof(uint32_t) + sizeof(int32_t) + sizeof(uint32_t) * 4 + 
                              espacio_de_array_parametros(pedido_filesystem->instruccion);
    buffer->size = sizeof(uint32_t) + sizeof(int32_t) + tam_instruccion;

    void* stream = malloc(buffer->size);
    int offset = 0;
    memcpy(stream + offset, &pedido_filesystem->pid, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &pedido_filesystem->direccion_fisica, sizeof(int32_t));
    offset += sizeof(int32_t);

    t_buffer* buffer_instruccion = crear_buffer_para_t_instruccion(pedido_filesystem->instruccion);
    void *stream_instruccion = buffer_instruccion->stream;
    memcpy(stream + offset, stream_instruccion, buffer_instruccion->size);
    offset += buffer_instruccion->size;
    buffer->stream = stream;
    
    free(buffer_instruccion);
    return buffer;
}


void enviar_pedido_file_system(t_pedido_file_system* pedido_filesystem, int conexion, t_log* logger_kernel) 
{
    t_buffer* buffer = crear_buffer_para_t_pedido_file_system(pedido_filesystem);
    t_paquete *paquete = crear_paquete(buffer, CODIGO_INFORMACION_FILE_SYSTEM);
    enviar_paquete(conexion, paquete, logger_kernel);
}


t_pedido_file_system* crear_pedido_file_system_para_el_buffer(t_buffer* buffer) 
{
    t_pedido_file_system* pedido_filesystem = malloc(sizeof(t_pedido_file_system));
    void* stream = buffer->stream;
    uint32_t offset = 0;
    memcpy(&(pedido_filesystem->pid), stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(pedido_filesystem->direccion_fisica), stream + offset, sizeof(int32_t));
    offset += sizeof(int32_t);
    pedido_filesystem->instruccion = crear_instruccion_para_el_buffer(buffer, &offset);
    return pedido_filesystem;
}



// ------------------------------------------------------------------------------------------
// -- Memoria --
// ------------------------------------------------------------------------------------------

t_procesoMemoria* deserializar_buffer_para_proceso_memoria(t_buffer* buffer) 
{
    t_procesoMemoria* proceso = malloc(sizeof(t_procesoMemoria));

    void* stream = buffer->stream;

    memcpy(&(proceso->pid), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(proceso->tamanio_tabla), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    proceso->tabla_segmento = malloc(sizeof(t_segmento) * proceso->tamanio_tabla);
    t_segmento* tabla_segmento = proceso->tabla_segmento;

    for (int i = 0; i < proceso->tamanio_tabla; i++) {
        memcpy(&(tabla_segmento[i].id), stream, sizeof(int32_t));
        stream += sizeof(int32_t);
        memcpy(&(tabla_segmento[i].base), stream, sizeof(uint32_t));
        stream += sizeof(uint32_t);
        memcpy(&(tabla_segmento[i].limite), stream, sizeof(uint32_t));
        stream += sizeof(uint32_t);
    }
    return proceso;
}


t_buffer *crear_buffer_para_proceso_memoria(t_procesoMemoria* proceso)
{
    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint32_t) +                         // pid
                   sizeof(uint32_t) +                         // tamanio_tabla
                   (sizeof(int32_t) + sizeof(uint32_t) * 2) * proceso->tamanio_tabla;  // (id + base + segmento) * cuan grande es la tabla
    
    void *stream = malloc(buffer->size);

    int offset = 0;
    
    memcpy(stream + offset, &proceso->pid, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &proceso->tamanio_tabla, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    for(int i = 0; i < proceso->tamanio_tabla; i++)
    {
        t_segmento segmento = proceso->tabla_segmento[i];
        memcpy(stream + offset, &segmento.id, sizeof(int32_t));
        offset += sizeof(int32_t);
        memcpy(stream + offset, &segmento.base, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(stream + offset, &segmento.limite, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    }
    buffer->stream = stream;
    return buffer;
}
 

void enviarPedidoAMemoria(t_instruccion* pedidoMemoria, uint32_t socket, t_log* loggercito) 
{
    t_buffer* buffer = crear_buffer_para_t_instruccion(pedidoMemoria); 
    t_paquete* paquete = crear_paquete(buffer, CODIGO_INSTRUCCION);
    enviar_paquete(socket, paquete, loggercito);
    destruir_paquete(paquete);
}


t_procesoMemoria* recibirProcesoDeMemoria(uint32_t socket, t_log *logger) 
{
    t_paquete *paquete = get_paquete(socket, logger);
    t_procesoMemoria* procesoDeMemoria = deserializar_buffer_para_proceso_memoria(paquete->buffer);
    destruir_paquete(paquete);
	return procesoDeMemoria;
}