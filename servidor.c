//  Proyecto Servidor HTTP para Sistemas Distribuidos
//	Compilación: gcc main.c -o mi_httpd

#include <stdio.h>	
#include <errno.h>  
#include <stdlib.h>	
#include <string.h>	
#include <ctype.h>	
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#define SERWEB_VERSION_NAME                 "mínima"

#define DEFAULT_PORT                        8080
#define DEFAULT_ROOT_DIRECTORY              "./html"
#define DEFAULT_DIRECTORY_INDEX             "index.html"
#define DEFAULT_MAX_CLIENTS                 15

#define REQUEST  "Hola a todos\n"

#define MAX_REQUEST_SIZE                    4048
#define MAX_PATH_SIZE                       256
#define BUFSIZE                             8096


int AtenderProtocoloHTTP(int sd)
{
    char buffer[MAX_REQUEST_SIZE] = "\0";
    long bLeidos = 0;
    int  result;

    // HTTP REQUEST: leemos la cabecera (debe ser siempre menor que MAX_REQUEST_SIZE)
    bLeidos = read(sd, buffer, sizeof(buffer)-1);
    buffer[bLeidos] = '\0';

    // HTTP RESPONSE: respondemos con un mensaje
    result = write(sd, REQUEST, strlen(REQUEST));

    return result;
}   /* de AtenderProtocoloHTTP */


int ObtenerSocket(int puerto)
{
    int                 sfdServidor = -1;   // socket para esperar peticiones
    struct sockaddr_in  addr;
    int                 error;

    // SOCKET: Abrimos un socket donde esperar peticiones de servicio
    sfdServidor = socket(AF_INET, SOCK_STREAM, 0);

    // BIND: Asociamos el puerto en el que deseamos esperar con el socket obtenido
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_port=htons((u_short)puerto);

    error = bind(sfdServidor, (struct sockaddr *)&addr, sizeof(addr));

    // LISTEN: Nos ponemos a la escucha en el socket, indicando cuántas atendemos a la vez
    error = listen(sfdServidor, DEFAULT_MAX_CLIENTS);

    return sfdServidor;
}   // de ObtenerSocket


int ConectarConCliente(int sfdServidor)
{
    int                 sfdCliente = -1;    // socket del hijo. Atiende cada nueva petición
    struct sockaddr_in  cliente;
           socklen_t    tamCliente = sizeof (cliente);

    // ACCEPT: Esperamos hasta que llegue una comunicación, para la que nos dan un nuevo socket
    sfdCliente = accept(sfdServidor, (struct sockaddr *)&cliente, &tamCliente);

    return sfdCliente;
}   // de ConectarConCliente


int main()
{
    int sfdServidor = -1;   // socket del padre. Escucha nuevas peticiones
    int sfdCliente = -1;    // socket del hijo. Atiende cada nueva petición
    int result;

    sfdServidor = ObtenerSocket(DEFAULT_PORT);

    while (1) {
        sfdCliente = ConectarConCliente(sfdServidor);

        switch (fork()) {
            case -1:    // Error
                close(sfdCliente);
                close(sfdServidor);
                exit (-1);

            case  0:    // El hijo atiende la nueva solicitud
                close(sfdServidor);
                result = AtenderProtocoloHTTP(sfdCliente);      // Atendemos el PROTOCOLO concreto
                close(sfdCliente);                              
                exit (result);

            default:    // El padre se queda a la espera de nuevas conexiones
                close(sfdCliente);
        }
    }   /* de while infinito */
}	/* de main */
