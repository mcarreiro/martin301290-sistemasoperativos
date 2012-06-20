#include "srv.h"

#define ACTIVO	TRUE
#define CAIDO	FALSE

#define TAG_REQUEST		60
#define TAG_REPLY		70
#define TAG_SHUT_DOWN	80
#define TAG_PODES_IRTE	90

//request number
int myRequestNumber;
//request number global
int requestNumber;


int deathNumber;		
int deathNumberGlobal;

int hay_pedido_local;
int listo_para_salir;

int activeServers;

int waitingReplys;
int dying;
/* El siguiente int lo vamos a usar para saber el numero de solicitud de los REQUEST que recibo */
int numberRequestRecived;

int cantResponses;
/*
 *  Ejemplo de servidor que tiene el "sí fácil" para con su
 *  cliente y no se lleva bien con los demás servidores.
 *
 */

void servidor(int mi_cliente)
{
    MPI_Status status;
    int origen, tag;
    hay_pedido_local = FALSE;
    listo_para_salir = FALSE;
    
	int i;
    waitingReplys = FALSE;
    dying = FALSE;
    requestNumber = 0;					
    deathNumberGlobal = 0;
    cantResponses = 0;
    int pendingResponses[cant_ranks / 2];		/* Reply_Deferred */
    for(i = 0; i < cant_ranks / 2; i++) pendingResponses[i] = FALSE;
    
	activeServers = cant_ranks / 2;	/* Para saber la cantidad de servidores activos (incluyendome) */
    int statusServers[cant_ranks / 2];			/* Para saber si el i-esimo servidor esta o no activo */
    for(i = 0; i < cant_ranks / 2; i++) statusServers[i] = ACTIVO;
    
    	
    
    while( ! listo_para_salir ) {
        
        MPI_Recv(NULL, 0, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
        origen = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        
        handle(tag,pendingResponses,statusServers, origen, mi_cliente,status);
        
    }
    
}

void handle(int tag, int pendingResponses[], int statusServers[], int origen, int mi_cliente,MPI_Status status){
	int i;
	switch(tag)
        {
			case(TAG_PEDIDO):
			{
				assert(origen == mi_cliente);
				debug("Mi cliente solicita acceso exclusivo");
				assert(hay_pedido_local == FALSE);
				hay_pedido_local = TRUE;
				debug("Dándole permiso");
				
				/* Seteo el numero de solicitud y la cantidad de respuestas que necesito */
				
				myRequestNumber = requestNumber + 1;
				
						   
				/* Ahora mando un REQUEST a todos los servidores activos, si los hay */
											
				if ((activeServers -1) != 0)
				{
					for(i = 0; i < cant_ranks / 2; i++)
					{
						if(statusServers[i] == ACTIVO && i * 2 != mi_rank) MPI_Send(&myRequestNumber, 1, MPI_INT, i * 2, TAG_REQUEST, COMM_WORLD);
					}
					waitingReplys = TRUE;
					/* En vez de enviar los REQUEST y hacer un break para volver al ciclo principal (el while(!listo_para_salir)) me quedo loopeando aca esperando a tener todas las respuestas.
					 * Por esto, puede ser que mientras este esperando, me lleguen otro tipo de pedidos, como REQUEST o SHUT_DOWN, asi que tengo que decir como atenderlos 
					 * Lo hacemos de esta forma para que el codigo sea lo mas declarativo posible, aunque esto haga que el codigo sea mas largo que si se resolviese todo en el ciclo principal */
					 
					/* Con lo aclarado anteriormente, nosotros mientras estemos en el proximo ciclo, sabemos que estamos queriendo tener acceso exclusivo y actuamos en consecuencia ante los distintos pedidos que nos llegan */
							
					while(cantResponses < (activeServers -1))
					{
						MPI_Recv(&numberRequestRecived, 1, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
						origen = status.MPI_SOURCE;
						tag = status.MPI_TAG;
						
						/* Ya recibi algo, tengo que ver si fue un REQUEST, un REPLY o un SHUT_DOWN (otra cosa no puede ser xq a esta altura mi cliente esta bloqueado esperando que le de el OK) */
						
						handle(tag,pendingResponses,statusServers, origen, mi_cliente,status);
					}
					waitingReplys = FALSE;
				}
							
				/* Si llegue hasta aca, ya tengo todos mis REPLY (si los necesitaba), por lo que ya tengo acceso exclusivo! */
								
				MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);	/* Se lo informo al cliente */		
								
				break;
			}
			case(TAG_PODES_IRTE):
			case(TAG_REPLY):
			{							
				cantResponses++;	/* Si es un REPLY o aviso de q puedo irme sumo a la canitdad de respuestas necesarias */
						
				break;
			}
			case(TAG_LIBERO):
			{
				assert(origen == mi_cliente);
				debug("Mi cliente libera su acceso exclusivo");
				assert(hay_pedido_local == TRUE);
				hay_pedido_local = FALSE;
				
				/* Como ya no estoy en la seccion critica, respondo todos los REQUEST que tenia pospuestos */
				
				for(i = 0; i < cant_ranks / 2; i++)
				{
					if(pendingResponses[i] == TRUE)
					{
						assert(i * 2 != mi_rank);					/* Por si me mande un moco, yo mismo no deberia tener TRUE */
						assert(statusServers[i] == ACTIVO);		/* Si le debo una respuesta, el servidor deberia estar esperandola */
						
						pendingResponses[i] = FALSE;
						MPI_Send(NULL, 0, MPI_INT, i * 2, TAG_REPLY, COMM_WORLD);
					}
				}				
				
				break;
			}
			
			case(TAG_REQUEST):
			{
				if(waitingReplys){
					if(numberRequestRecived < myRequestNumber){
						/* Si pasa esto, tiene prioridad el que me mando el REQUEST, asi que le mando el REPLY y yo sigo 
						* esperando a que me respondan */
						MPI_Send(NULL, 0, MPI_INT, origen, TAG_REPLY, COMM_WORLD);	/* Le doy el OK */
					}else{
						if(numberRequestRecived > myRequestNumber){
							/* Si pasa esto, yo tengo prioridad, por lo que pospongo su REPLY */
							requestNumber = numberRequestRecived;	/* Actualizo el numero de solicitud */
							pendingResponses[origen/2] = TRUE;
						}else{
							/* Si el numero de pedido es igual que el mio, tiene prioridad el que tiene menor rank */
							if(origen < mi_rank) MPI_Send(NULL, 0, MPI_INT, origen, TAG_REPLY, COMM_WORLD);	/* Le doy el OK */
							else pendingResponses[origen/2] = TRUE;									/* Sino, pospongo su REPLY */
						}
					}
				}else{
					/* Se que si estoy aca no voy a estar esperando por obtener la seccion critica...
					* Entonces puede suceder o que no este en ella (xq no la pedi) o que si lo este (xq ya me la otorgaron) */
				
					if(hay_pedido_local) pendingResponses[origen/2] = TRUE; 		/* Yo estoy en la seccion critica, pospongo el REPLY */
					else MPI_Send(NULL, 0, MPI_INT, origen, TAG_REPLY, COMM_WORLD);		/* Si no estoy en la seccion critica, le doy el OK */
					
					if(requestNumber < numberRequestRecived) requestNumber = numberRequestRecived;		/* Actualizo la variable */
				}
				break;
				
			}
			
			case(TAG_SHUT_DOWN):
			{
				/* Un server se dio de baja. No me importa si estoy o no en mi seccion critica */
				if(dying){
					if(numberRequestRecived < deathNumber){
						MPI_Send(NULL, 0, MPI_INT, origen, TAG_PODES_IRTE, COMM_WORLD);
						statusServers[origen / 2] = CAIDO;
					}else{
						if(numberRequestRecived > deathNumber)	{
							pendingResponses[origen / 2] = TRUE;
							deathNumberGlobal = numberRequestRecived;
						}else{
							if(origen < mi_rank){
								MPI_Send(NULL, 0, MPI_INT, origen, TAG_PODES_IRTE, COMM_WORLD);
								statusServers[origen / 2] = CAIDO;
							}else statusServers[origen / 2] = TRUE;
						}	
					}	
				}else{
					activeServers--;
					MPI_Send(NULL, 0, MPI_INT, origen, TAG_PODES_IRTE, COMM_WORLD);		/* Lo dejo irse */
					statusServers[origen/2] = CAIDO;
					
					if(numberRequestRecived > deathNumberGlobal) deathNumberGlobal = numberRequestRecived;
				}
								
				break;
			}
			
			case(TAG_TERMINE):
			{
				assert(origen == mi_cliente);
				debug("Mi cliente avisa que terminó");
				listo_para_salir = TRUE;
				assert(hay_pedido_local == FALSE);
				
				/* Aviso a todos los servidores activos que me voy */
				/* Es el mismo algoritmo de que para los REQUEST */
				
				cantResponses = activeServers - 1;
				deathNumber = deathNumberGlobal + 1;
				
				for(i = 0; i < cant_ranks / 2; i++)
				{
					if(statusServers[i] == ACTIVO && i * 2 != mi_rank)
					{
						MPI_Send(&deathNumber, 1, MPI_INT, i * 2, TAG_SHUT_DOWN, COMM_WORLD);
						
						assert(pendingResponses[i] == FALSE);
					}
				}
				
				/* Ahora espero que los demas servidores me den el OK para poder irme */
				/* De esta forma, si habia algun mensaje en el aire para mi cuando decido irme, lo voy a agarrar antes de salir ssh */
				dying = TRUE;
				while(cantResponses < (activeServers -1))
				{
					MPI_Recv(&numberRequestRecived, 1, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
					origen = status.MPI_SOURCE;
					tag = status.MPI_TAG;
					handle(tag,pendingResponses,statusServers, origen, mi_cliente,status);					
				}
				
				/* Ya tengo todas mis respuestas para poder morir tranquilo */
				
				/* Como ultima voluntad, le envio todos los PODES_IRTE que tengo pendientes */
				
				for(i = 0; i < cant_ranks / 2; i++)
				{
					if(pendingResponses[i] == TRUE)
					{
						assert(i * 2 != mi_rank);					/* Por si me mande un moco, yo mismo no deberia tener TRUE */
						assert(statusServers[i] == ACTIVO);		/* Si le debo una respuesta, el servidor deberia estar esperandola */
						
						MPI_Send(NULL, 0, MPI_INT, origen, TAG_PODES_IRTE, COMM_WORLD);
					}
				}				
				
				break;
			}			
		}
}
