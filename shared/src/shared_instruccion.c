#include "shared_utils.h"



// ------------------------------------------------------------------------------------------
// -- Instruccion --
// ------------------------------------------------------------------------------------------

uint32_t espacio_de_array_parametros(t_instruccion *instruccion)
{
    uint32_t espacio = 0;
    for (int i = 0; i < instruccion->cant_parametros; i++)
        espacio += strlen(instruccion->parametros[i]) + 1;
    return espacio;
}


t_buffer *crear_buffer_para_t_instruccion(t_instruccion *instruccion) // creo un buffer para una instruccion
{
    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint32_t) +                         // identificador
                   sizeof(uint32_t) +                         // cant_parametros
                   espacio_de_array_parametros(instruccion) + // p1 + p2 + p3 + p4
                   sizeof(uint32_t) * 4;                      // p1_length, p2_length, p3_length, p4_length
    void *stream = calloc(buffer->size, 1);

    int offset = 0;
    memcpy(stream + offset, &instruccion->identificador, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &instruccion->cant_parametros, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &instruccion->p1_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &instruccion->p2_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &instruccion->p3_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &instruccion->p4_length, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    for (int i = 0; i < instruccion->cant_parametros; i++)
    {
        memcpy(stream + offset, instruccion->parametros[i], strlen(instruccion->parametros[i]) + 1);
        offset += strlen(instruccion->parametros[i]) + 1;
    }
    buffer->stream = stream;
    return buffer;
}


t_instruccion *crear_instruccion_para_el_buffer(t_buffer *buffer, uint32_t *offset)
{
    t_instruccion *instruccion = malloc(sizeof(t_instruccion));
    void *stream = buffer->stream;
    uint32_t offset_aux = 0;
    stream += (*offset);

    memcpy(&(instruccion->identificador), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(instruccion->cant_parametros), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(instruccion->p1_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(instruccion->p2_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(instruccion->p3_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(instruccion->p4_length), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    offset_aux += sizeof(uint32_t) * 6;
    instruccion->parametros = NULL;
    if (instruccion->cant_parametros == 0)
    {
        *offset += offset_aux;
        return instruccion;
    }
    instruccion->parametros = (char **)malloc(instruccion->cant_parametros * sizeof(char *));
    instruccion->parametros[0] = malloc(instruccion->p1_length);
    memcpy(instruccion->parametros[0], stream, instruccion->p1_length);
    offset_aux += instruccion->p1_length;
    stream += instruccion->p1_length;
    if (instruccion->p1_length == 1)
    {
        offset_aux -= 1;
        stream -= 1;
    }
    if (instruccion->cant_parametros == 1)
    {
        *offset += offset_aux;
        return instruccion;
    }
    instruccion->parametros[1] = malloc(instruccion->p2_length);
    memcpy(instruccion->parametros[1], stream, instruccion->p2_length);
    offset_aux += instruccion->p2_length;
    stream += instruccion->p2_length;
    if (instruccion->p2_length == 1)
    {
        offset_aux -= 1;
        stream -= 1;
    }
    if (instruccion->cant_parametros == 2)
    {
        *offset += offset_aux;
        return instruccion;
    }
    instruccion->parametros[2] = malloc(instruccion->p3_length);
    memcpy(instruccion->parametros[2], stream, instruccion->p3_length);
    offset_aux += instruccion->p3_length;
    stream += instruccion->p3_length;
    if (instruccion->p3_length == 1)
    {
        offset_aux -= 1;
        stream -= 1;
    }
    if (instruccion->cant_parametros == 3)
    {
        *offset += offset_aux;
        return instruccion;
    }
    instruccion->parametros[3] = malloc(instruccion->p4_length);
    memcpy(instruccion->parametros[3], stream, instruccion->p4_length);
    offset_aux += instruccion->p4_length;
    stream += instruccion->p4_length;

    if (instruccion->p4_length == 1)
    {
        offset_aux -= 1;
        stream -= 1;
    }
    
    *offset += offset_aux;

    return instruccion;
}


void agregar_parametro_a_instruccion(t_list *parametros, t_instruccion *instruccion)
{
    int i = 0;
    if (parametros != NULL)
        while (i < instruccion->cant_parametros)
        {
            instruccion->parametros[i] = list_get(parametros, i);
            i++;
        }   
    instruccion->p1_length = 0;
    instruccion->p2_length = 0;
    instruccion->p3_length = 0;
    instruccion->p4_length = 0;
    if(instruccion->cant_parametros >= 1)
        instruccion->p1_length = strlen(instruccion->parametros[0]) + 1;
    if (instruccion->cant_parametros >= 2)
        instruccion->p2_length = strlen(instruccion->parametros[1]) + 1;
    if (instruccion->cant_parametros >= 3)
        instruccion->p3_length = strlen(instruccion->parametros[2]) + 1;
    if (instruccion->cant_parametros >= 4)
        instruccion->p4_length = strlen(instruccion->parametros[3]) + 1;
}


t_instruccion* crear_instruccion(t_identificador identificador, t_list* parametros)
{
    t_instruccion* tmp = malloc(sizeof(t_instruccion));

    tmp -> identificador = identificador;
    if (list_size(parametros) < 1 ) {
        tmp -> cant_parametros = 0;
        tmp -> parametros = NULL;
        tmp ->p1_length = 0;
        tmp->p2_length = 0;
        tmp -> p3_length=0;
        tmp ->p4_length = 0;
    } else {
        tmp -> cant_parametros = list_size(parametros);
        tmp -> parametros = malloc(sizeof(char*) * tmp->cant_parametros);
        agregar_parametro_a_instruccion(parametros, tmp);
    }
    return tmp;
}


void destruir_instruccion(t_instruccion *instruccion)
{
    if (instruccion == NULL)
    {
        return;
    }
    for (int i = 0; i < instruccion->cant_parametros; i++)
    {
        if (instruccion->parametros[i] != NULL)
        {
            free(instruccion->parametros[i]);
        }
    }
    free(instruccion->parametros);
    free(instruccion);
}


t_list *get_instrucciones(t_paquete *paquete)
{
    t_list *instrucciones = crear_lista_instrucciones_para_el_buffer(paquete->buffer);
    return instrucciones;
}


t_buffer *crear_buffer__para_t_lista_instrucciones(t_list *lista_instrucciones) // creo un buffer para muchas instrucciones
{
    t_buffer *buffer = malloc(sizeof(t_buffer));
    uint32_t size_total = 0;
    for (int i = 0; i < list_size(lista_instrucciones); i++)
    {
        t_instruccion *instruccion = list_get(lista_instrucciones, i);
        t_buffer *buffer_instruccion = crear_buffer_para_t_instruccion(instruccion); // 10 - 20 - 4
        size_total += buffer_instruccion->size;
        destruir_buffer(buffer_instruccion);
    }
    // creo el stream y copio los datos de cada buffer
    void *stream = malloc(size_total);
    buffer->size = size_total;
    uint32_t offset = 0;
    for (int i = 0; i < list_size(lista_instrucciones); i++)
    {
        t_buffer *buffer_instruccion = crear_buffer_para_t_instruccion(list_get(lista_instrucciones, i));
        uint32_t size = buffer_instruccion->size;
        void *stream_instruccion = buffer_instruccion->stream;
        memcpy(stream + offset, stream_instruccion, size);
        offset += size;
        destruir_buffer(buffer_instruccion);
    }
    buffer->stream = stream;
    return buffer;
}


t_list *crear_lista_instrucciones_para_el_buffer( t_buffer *buffer)
{
    t_list *lista_instrucciones = list_create();
    uint32_t offset = 0;
    while (offset < buffer->size)
    {
        t_instruccion *instruccion = crear_instruccion_para_el_buffer(buffer, &offset);
        list_add(lista_instrucciones, instruccion);
    }
    return lista_instrucciones;
}


bool enviar_instrucciones(int socket, t_list *lista_instrucciones, t_log *logger)
{
    t_buffer *buffer = crear_buffer__para_t_lista_instrucciones(lista_instrucciones);
    t_paquete *paquete = crear_paquete(buffer, CODIGO_INSTRUCCION);
    bool res = enviar_paquete(socket, paquete, logger);
    destruir_paquete(paquete);
    return res;
}