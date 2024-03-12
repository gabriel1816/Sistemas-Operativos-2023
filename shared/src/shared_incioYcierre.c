#include "shared_utils.h"


// ------------------------------------------------------------------------------------------
// -- Inicio / Handshake / Cierre --
// ------------------------------------------------------------------------------------------

int iniciar_servidor(t_log *logger, const char *name, char *ip, char *puerto)
{
    int socket_servidor;
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(ip, puerto, &hints, &servinfo);
    bool conecto = false;
    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next)
    {
        socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_servidor == -1) 
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(socket_servidor);
            continue;
        }
        conecto = true;
        break;
    }
    int yes = 1;
    setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    if (!conecto)
    {
        free(servinfo);
        return 0;
    }
    listen(socket_servidor, SOMAXCONN); 
    log_info(logger, "Escuchando en %s:%s (%s)", ip, puerto, name);
    freeaddrinfo(servinfo); 
    return socket_servidor;
}


t_config *iniciar_config(char *path)
{
    t_config *nuevo_config = config_create(path);
    return nuevo_config;
}


int crear_conexion(char* ip, char* puerto, t_log* logger)
{
    struct addrinfo hints;
    struct addrinfo* server_info;
    uint32_t handshake = 1;
    uint32_t result = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(ip, puerto, &hints, &server_info) != 0) {
        log_error(logger, "Error al resolver la dirección y puerto.");
        return -1;
    }

    int socket_cliente = socket(server_info->ai_family,
                                server_info->ai_socktype,
                                server_info->ai_protocol);

    if (socket_cliente == -1) {
        log_error(logger, "Error al crear el socket.");
        freeaddrinfo(server_info);
        return -1;
    }

    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) != 0) {
        log_error(logger, "Error al conectar el socket.");
        close(socket_cliente);
        freeaddrinfo(server_info);
        return -1;
    }

    if (send(socket_cliente, &handshake, sizeof(handshake), 0) == -1) {
        log_error(logger, "Error al enviar el handshake.");
        close(socket_cliente);
        freeaddrinfo(server_info);
        return -1;
    }

    if (recv(socket_cliente, &result, sizeof(result), MSG_WAITALL) == -1) {
        log_error(logger, "Error al recibir la respuesta del handshake.");
        close(socket_cliente);
        freeaddrinfo(server_info);
        return -1;
    }

    freeaddrinfo(server_info);

    if (result == 0) {
        return socket_cliente;
    } else {
        log_error(logger, "Error en la conexión");
        close(socket_cliente);
        return -1;
    }
}


int esperar_cliente(int socket_servidor, t_log* logger)
{
    uint32_t handshake;
    uint32_t resultOk = 0;
    uint32_t resultError = -1;
    int socket_cliente = accept(socket_servidor, NULL, NULL);

    recv(socket_cliente, &handshake, sizeof(uint32_t), MSG_WAITALL);
    if (handshake == 1)
    {
        send(socket_cliente, &resultOk, sizeof(uint32_t), NULL);
        
    }
    else 
    {
        send(socket_cliente, &resultError, sizeof(uint32_t), NULL);
        log_info(logger,"Error en el Hanshake \n");
    } 
    return socket_cliente;
}


void liberar_conexion(int socket_cliente)
{
    close(socket_cliente);
    socket_cliente = -1;
}


void terminar_programa(int conexion, t_log *logger, t_config *config)
{
    /* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config)
      con las funciones de las commons y del TP mencionadas en el enunciado */
    if (logger != NULL)
    {
        log_destroy(logger);
    }
    if (config != NULL)
    {
        config_destroy(config);
    }
    if (conexion != NULL)
    {
        liberar_conexion(conexion);
    }
}


void terminar_programa_consola(int conexion, t_log *logger, t_config *config)
{
    if (logger != NULL)
    {
        //log_destroy(logger);
    }
    if (config != NULL)
    {
        config_destroy(config);
    }
    //if (conexion != NULL)
    //{
        liberar_conexion(conexion);
    //}
}



// ------------------------------------------------------------------------------------------
// -- Mensajes --
// ------------------------------------------------------------------------------------------

void enviar_mensaje(char *mensaje, int socket_cliente)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = MENSAJE;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    int bytes = paquete->buffer->size + 2 * sizeof(int);

    void *a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
    destruir_paquete(paquete);
}

void recibir_mensaje(int socket_cliente, t_log* logger) {
    int tamaño;
    int codigo_operacion;
    char *mensaje;
    if(recv(socket_cliente, &tamaño, sizeof(int), 0) <= 0) {
        log_error(logger, "Error al recibir tamaño del mensaje");
        return;
    }
    if(recv(socket_cliente, &codigo_operacion, sizeof(int), 0) <= 0) {
        log_error(logger, "Error al recibir código de operación");
        return;
    }
    mensaje = malloc(tamaño);
    if(recv(socket_cliente, mensaje, tamaño, 0) <= 0) {
        //log_error(logger, "Error al recibir mensaje");
        return;
    }
    printf("Mensaje recibido: %s\n", mensaje);
    free(mensaje);
}