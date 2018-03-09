// main.c
//
// Descripcion: crear un cliente web HTTP llamado mi_wget
//
// 1/03/2018
// Jaime Lopez Sanchez
// Copyrigth (c)
//
//	Compilacion: 
//		gcc main.c -o mi_wget -D DEBUG
//
//	Ejemplo de utilizacion:
//		./mi_wget 192.168.322.5 /index.html
//

// Seccion de cabeceras 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>


//Herramientas de depuracion
#ifdef DEBUG
#define log(fmt, args...)  fprintf(stderr, fmt "\n", ## args)
#else
#define log(fmt,args...)
#endif

void pexit(char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
} //de pexit

// Seccion de definiciones
#define HTTP_REQUEST_PATTERN "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: keep-alive\r\n\r\n"

//El verdadero cliente web(HTPP)
void clienteweb(char *dirIP, int puerto, char *recurso)
{
	int sockfd;
	static struct sockaddr_in serv_addr;
	char HTTP_request[200];
	char HTTP_response[500];
	int bLeidos;

	// SOCKET: obtener un canal por internet
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	
	if(sockfd < 0) {
		pexit("socket() falló");
	}	


	// CONNECT: conectar con un host(direccion IP) y una aplicacion(port)
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(puerto);
	serv_addr.sin_addr.s_addr = inet_addr(dirIP);

	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		pexit("connect() falló");	
	}

	// WRITE: nos comunicamos con el servidor web(HTTP)
	sprintf(HTTP_request,HTTP_REQUEST_PATTERN,recurso,dirIP,"mi_wget");
	log("* Enviando %d bytes: \n\n%s\n",(int) strlen(HTTP_request),HTTP_request);
	write(sockfd, HTTP_request,strlen(HTTP_request));
	

	// READ: recogemos lo que nos envie el servidor(HTTP)
	while( (bLeidos = read(sockfd,HTTP_response,sizeof(HTTP_response))) > 0)
		write(STDOUT_FILENO, HTTP_response,bLeidos);


	// CLOSE: cierro el canal
	close(sockfd);

} //de cliente web

int mostrarUso(char *aplicacion)
{
		fprintf(stderr, "Debe proporcionar una URL valida desglosada. \n");
		fprintf(stderr,"Uso: %s PROTOCOL HOST PORT RESOURCE recurso\n\n",aplicacion);
		fprintf(stderr, "Ejemplo: %s http www.dlsi.ua.es 0 /~dmarcos/sd/index.html\n", aplicacion);
		
} //fin de mostrar uso


// Punto de entrada:
int main(int argc, char *argv[])
{
	
	char *protocol;
	char *host;
	int port;
	char *resource;
	char *host_ip;
	
	struct servent *sp;  //para convertir protocolo en puerto
	struct hostent *host_struct; //para convertir DNS en IP
	
	//Verificamos los argumentos de entrada
	if(argc==2) {
		//declaro estas variables auxiliares ya que en la expresion regular no puede guardarlo directamente al puntero
		char p[100];
		char h[100];
		char r[100];
		
		sscanf(argv[1],"%99[^:]://%99[^/]/%99[^\n]",p,h,r); //problema con la barra!!!!
		
		protocol = p;
		host = h;
		resource = r;
		port = 0;
		
	}
	else if(argc != 5) {
		mostrarUso(argv[0]);
		return EXIT_FAILURE;	
	}
	else {
			//asignamos los argumentos
		protocol = argv[1];
		host = argv[2];
		port = atoi(argv[3]);
		resource = argv[4];
	}
	
	//si el puerto es 0 obtenemos el puerto a partir del protocolo
	if((protocol[0]!='\0') && (port == 0)) {
		sp = getservbyname(protocol,"tcp");
		if(!sp) {
			fprintf(stderr,"No se ha podido encontrar un puerto valido para el protocolo %s\n",protocol);
			return EXIT_FAILURE;
		}
		port = ntohs(sp->s_port);
	}
	
	//convertimos el DNS en direccion IP
	host_struct = gethostbyname(host);
	if(host_struct!=NULL)
		host_ip = inet_ntoa(*(struct in_addr *) host_struct->h_addr);
	
	//Mostrar todas las variables relevantes
	log(" * Protocolo = <%s>",protocol);
	log(" * Host = <%s>", host);
	log(" * Puerto = <%d>", port); //<-- se poner %d porque port es un entero mientras que el resto es char*
	log(" * Ruta = <%s>", resource);
	log(" * IP = <%s>", host_ip);
	
	if(!port || !host_ip || resource[0]=='\0') {
		mostrarUso(argv[0]);
		return EXIT_FAILURE;
	}
	
	//Invocar al verdadera cliente web(HTTP)
	clienteweb(host_ip,port,resource);

	return EXIT_SUCCESS;	
	
} //de main
