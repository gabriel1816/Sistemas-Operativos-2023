#include "file_system.h"

void levantar_config() {
    config_FileSystem = config_create(CONFIG_FILE_PATH); // con debug
    ip_file_system = config_get_string_value(config_FileSystem, "IP_FILESYSTEM");
    ip_memoria = config_get_string_value(config_FileSystem, "IP_MEMORIA");
    puerto_escucha = config_get_string_value(config_FileSystem, "PUERTO_ESCUCHA");
    puerto_memoria = config_get_string_value(config_FileSystem, "PUERTO_MEMORIA");
    path_superbloque = config_get_string_value(config_FileSystem, "PATH_SUPERBLOQUE");
    path_bitmap = config_get_string_value(config_FileSystem, "PATH_BITMAP");
    path_bloques = config_get_string_value(config_FileSystem, "PATH_BLOQUES");
    path_fcb = config_get_string_value(config_FileSystem, "PATH_FCB");
    retardo = config_get_int_value(config_FileSystem, "RETARDO_ACCESO_BLOQUE");

}

t_superBloque* levantar_superBloque(char* path) {
    
    t_config* configSuperbloque = config_create(path);
    t_superBloque* superbloque = malloc(sizeof(t_superBloque));
    superbloque->tamanio_bloque = config_get_int_value(configSuperbloque , "BLOCK_SIZE");
    superbloque->cant_bloques = config_get_int_value(configSuperbloque , "BLOCK_COUNT");
    config_destroy(configSuperbloque);
    return superbloque;
}


//agarrar los archivos de la carpeta

t_list *levantar_fcbs(const char *directorio_fcbs)
{
    t_list *lista_fcbs = list_create();
    DIR *dir = opendir(directorio_fcbs);
    struct dirent *ent;

    if (dir != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_type == DT_REG)
            { 
                char ruta_completa[1024];  
                sprintf(ruta_completa, "%s/%s", directorio_fcbs, ent->d_name);
                t_config *fcb_config = config_create(ruta_completa);
                t_fcb *fcb = malloc(sizeof(t_fcb));
                fcb->nombre_archivo = config_get_string_value(fcb_config, "NOMBRE_ARCHIVO");
                fcb->tamanio_archivo = config_get_int_value(fcb_config, "TAMANIO_ARCHIVO");
                fcb->puntero_directo = config_get_int_value(fcb_config, "PUNTERO_DIRECTO");
                fcb->puntero_indirecto = config_get_int_value(fcb_config, "PUNTERO_INDIRECTO");
                double tamanio_archivo = (double)fcb->tamanio_archivo;
                double tamanio_bloque = (double)superBloque->tamanio_bloque;
                fcb->cant_bloques_asignados = (int)ceil(tamanio_archivo/tamanio_bloque);
                list_add(lista_fcbs, fcb);
                //config_destroy(fcb_config);
            }
        }
        closedir(dir);
    }
    else
    {
        // Puede que no se pueda abrir el directorio
        log_error(logger_fileSystem, "no se puedo abrir el directorio");
    }
    return lista_fcbs;
}

void atender_kernel(void *conexion)
{
    intptr_t conexionKernel = (intptr_t) conexion;

    while (1) // ver de sacar
    {    
    t_paquete* paquete_recibido = recibir_paquete(conexionKernel, logger_fileSystem); 
    t_pedido_file_system* pedido = crear_pedido_file_system_para_el_buffer(paquete_recibido->buffer);
        sem_wait(&semBinSolicitudes);
        switch(pedido->instruccion->identificador) {
            case F_OPEN:
                buscar_archivo_pedido(pedido->instruccion->parametros[0], conexionKernel);
                break;
            case F_CREATE:
                crear_archivo(path_fcb, pedido->instruccion->parametros[0], conexionKernel);
                break;
            case F_TRUNCATE:
                truncar_archivo_pedido(pedido->instruccion->parametros[0], pedido->instruccion->parametros[1], conexionKernel);
                notificar_truncado(pedido->instruccion, conexionKernel);
                break;
            case F_WRITE:
                escribir_archivo_pedido(pedido , conexionKernel);
                break;
            case F_READ:
                leer_archivo_pedido(pedido, conexionKernel);
                break;
            default:
                log_info(logger_fileSystem, "Identificador no reconocido: %d", pedido->instruccion->identificador);
                break;
        }
        sem_post(&semBinSolicitudes);
        free(paquete_recibido);
        free(pedido->instruccion);
        free(pedido);
    }
    log_info(logger_fileSystem, "Se cerrará la conexión (Kernel - Filesystem)");
    close(conexionKernel);
}

void notificar_truncado(t_instruccion* instruccion, intptr_t conexionKernel) {
    t_buffer *buffer = crear_buffer_para_t_instruccion(instruccion);
    t_paquete *paquete = crear_paquete(buffer, CODIGO_INSTRUCCION);
    enviar_paquete(conexionKernel, paquete, logger_fileSystem);
}


void iniciar_bitmap(char* path, t_superBloque* superbloque) 
{
    int tamano_bitmap = ceil(superbloque->cant_bloques / 8); 

    int archivo_bitmap = open(path, O_CREAT | O_RDWR, 0644);
    ftruncate(archivo_bitmap, sizeof(t_superBloque) + tamano_bitmap);

    void *mmapBitmap = mmap(NULL, sizeof(t_superBloque) + tamano_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, archivo_bitmap, 0);
    bitmap = bitarray_create_with_mode(mmapBitmap + sizeof(t_superBloque), tamano_bitmap, LSB_FIRST);

    if (lseek(archivo_bitmap, 0, SEEK_END) == 0) { // Si el archivo estaba vacío
        for(int i = 0; i < bitarray_get_max_bit(bitmap); i++) {
            bitarray_clean_bit(bitmap, i); // Establecer todos los bits en 0
        }
    }

    msync(mmapBitmap, sizeof(t_superBloque) + tamano_bitmap, MS_SYNC);

    // munmap(mmapBitmap, sizeof(t_superBloque) + tamano_bitmap); 

    close(archivo_bitmap);
}


void buscar_archivo_pedido(char* nombre, intptr_t una_conexion) {
 
    if (abrir_archivo(nombre)) {
        send(una_conexion, "SI", strlen("SI"), 0);
        log_info(logger_fileSystem, "Abrir Archivo: %s",nombre);
    } else {
        send(una_conexion, "NO", strlen("NO"), 0);
        log_info(logger_fileSystem, "Archivo no encontrado: %s",nombre);
    }

  //  ssize_t bytesEnviados = send(una_conexion, respuesta, strlen(respuesta), 0);

    
}

bool abrir_archivo(char* nombre_archivo) {
    t_fcb* fcb = buscar_fcb_por_nombre(lista_fcbs, nombre_archivo);
    return (fcb != NULL);
}

t_fcb* buscar_fcb_por_nombre(t_list* lista_fcbs, char* nombre_archivo) {
    for(int i = 0; i < list_size(lista_fcbs); i++) {
        t_fcb* nuevo_fcb = list_get(lista_fcbs, i);
        if(strcmp(nuevo_fcb->nombre_archivo, nombre_archivo) == 0) {
            return nuevo_fcb;  // Retorna el FCB si el nombre coincide
        }
    }
    return NULL;  // Retorna NULL si no se encontró un FCB con el nombre
}


void truncar_archivo_pedido(char* nombre_archivo, char* tamanio, intptr_t una_conexion) {
   
    log_info(logger_fileSystem, "Truncar Archivo: %s - Tamaño: %s", nombre_archivo, tamanio);
    uint32_t tam_nuevo = atoi(tamanio);

    t_fcb* fcb = buscar_fcb_por_nombre(lista_fcbs, nombre_archivo);
    if(fcb->tamanio_archivo > tam_nuevo) {
       acortar_archivo(fcb, tam_nuevo); 
    }
    else {
        agrandar_archivo(fcb, tam_nuevo);
    }
    actualizarFCB (fcb);
    list_clean(lista_fcbs);
    lista_fcbs = levantar_fcbs(path_fcb);
}

void acortar_archivo(t_fcb* fcb, uint32_t tamanio_nuevo) {
    
    int bloques_a_liberar = calcular_bloques_a_liberar(tamanio_nuevo, fcb);

    liberar_bloques(fcb, bloques_a_liberar);

    fcb->tamanio_archivo = tamanio_nuevo;
} 

void agrandar_archivo(t_fcb* fcb, uint32_t tamanio_nuevo) {

    uint32_t bloques_adicionales = calcular_bloques_adicionales(tamanio_nuevo, fcb);

    if (bloques_adicionales > 0)
    {
        
            uint32_t bloque_libre; 
                if(fcb->tamanio_archivo == 0) { // si se asigna primer bloque, también se asigna el de punteros
                    bloque_libre = buscar_bloque_libre(bitmap);
                    fcb->puntero_directo = bloque_libre;
                    fcb->cant_bloques_asignados = 1;
                    bloque_libre = buscar_bloque_libre(bitmap);  
                    fcb->puntero_indirecto = bloque_libre; 
                    bloques_adicionales--;
                        } 
                    asignar_bloques(fcb, bloques_adicionales);
    }
    fcb->tamanio_archivo = tamanio_nuevo;
}

void leer_archivo_pedido(t_pedido_file_system* pedido, intptr_t una_conexion) {

    log_info(logger_fileSystem , "Leer Archivo: %s - Puntero: %s - Memoria: %d - Tamaño: %s", pedido->instruccion->parametros[0], pedido->instruccion->parametros[1], pedido->direccion_fisica, pedido->instruccion->parametros[2]);
    leer_archivo(pedido->instruccion->parametros[0], atoi(pedido->instruccion->parametros[1]), atoi(pedido->instruccion->parametros[2]), pedido->direccion_fisica, pedido->pid);
    send(una_conexion, "SI", strlen("SI"), 0);
}

void escribir_archivo_pedido(t_pedido_file_system* pedido, intptr_t una_conexion) {


    enviar_pedido_file_system(pedido, conexion_memoria, logger_fileSystem);
    t_paquete *paquete = get_paquete(conexion_memoria, logger_fileSystem);
	int offset=0;
    t_instruccion* respuestaMemoria = crear_instruccion_para_el_buffer(paquete->buffer,&offset);
    log_info(logger_fileSystem , "Escribir Archivo: %s - Puntero: %s - Memoria: %d - Tamaño: %s", pedido->instruccion->parametros[0], pedido->instruccion->parametros[1], pedido->direccion_fisica, pedido->instruccion->parametros[2]);
    escribir_archivo(pedido->instruccion->parametros[0], atoi(pedido->instruccion->parametros[1]), respuestaMemoria->parametros[0], atoi(pedido->instruccion->parametros[2])); 
    destruir_paquete(paquete);
    send(una_conexion, "SI", strlen("SI"), 0);

}

int crear_archivo(char* path_fcb, char *nombre, intptr_t una_conexion) {
    t_fcb *fcb = malloc(sizeof(t_fcb));
    fcb->nombre_archivo = malloc(strlen(nombre) + 1);

    strcpy(fcb->nombre_archivo, nombre);
    fcb->tamanio_archivo = 0;
    fcb->puntero_directo = 0;
    fcb->puntero_indirecto = 0;
    char *ruta_fcb = malloc(strlen(path_fcb) + strlen(nombre) + 2);
    strcpy(ruta_fcb, path_fcb);
    strcat(ruta_fcb, "/");
    strcat(ruta_fcb, nombre);

    FILE *file_fcb = fopen(ruta_fcb, "w"); // abro el archivo en modo de escritura
    if (file_fcb == NULL) {
        // hubo un error al abrir el archivo
        free(ruta_fcb);
        free(fcb->nombre_archivo);
        free(fcb); 
        return -1; // devuelvo -1 en caso de error
    }

    fprintf(file_fcb, "NOMBRE_ARCHIVO=%s\n", fcb->nombre_archivo);
    fprintf(file_fcb, "TAMANIO_ARCHIVO=%d\n", fcb->tamanio_archivo);
    fprintf(file_fcb, "PUNTERO_DIRECTO=%u\n", fcb->puntero_directo);
    fprintf(file_fcb, "PUNTERO_INDIRECTO=%u", fcb->puntero_indirecto);

    fclose(file_fcb); // cierro el archivo
    list_add(lista_fcbs, fcb);
    free(ruta_fcb);
    log_info(logger_fileSystem, "Crear Archivo: %s", nombre);
    send(una_conexion, "SI", strlen("SI"), 0);
    return 0; // devuelvo 0 siempre porque es exitoso, siempre se puede crear el archivo
}

void actualizarFCB (t_fcb *fcb) {
    char *ruta_fcb = malloc(strlen(path_fcb) + strlen(fcb->nombre_archivo) + 2);
    strcpy(ruta_fcb, path_fcb);
    strcat(ruta_fcb, "/");
    strcat(ruta_fcb, fcb->nombre_archivo);
    t_config *fcb_config = config_create(ruta_fcb);

    config_set_value(fcb_config, "NOMBRE_ARCHIVO", fcb->nombre_archivo);
    config_set_value(fcb_config, "TAMANIO_ARCHIVO", itoa(fcb->tamanio_archivo));
    config_set_value(fcb_config, "PUNTERO_DIRECTO", itoa(fcb->puntero_directo));
    config_set_value(fcb_config, "PUNTERO_INDIRECTO", itoa(fcb->puntero_indirecto));

    config_save(fcb_config);
}

char* itoa(uint32_t num) { //función robada que transforma ints en strng
    uint32_t size_needed = snprintf(NULL, 0, "%d", num);
    char* str = malloc(size_needed + 1);  // Agrega 1 para el carácter nulo al final
    if (str == NULL) {
        printf("Error: No se pudo asignar memoria\n");
        return NULL;
    }
    snprintf(str, size_needed + 1, "%d", num);
    return str;
}
