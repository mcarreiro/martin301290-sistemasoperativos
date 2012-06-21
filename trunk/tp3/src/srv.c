#include "srv.h"

/*
 *  Ejemplo de servidor que tiene el "sí fácil" para con su
 *  cliente y no se lleva bien con los demás servidores.
 *
 */

int mi_numero_muerte;			/* Our_Sequence_Number para cuando cierro el servidor */
int numero_muerte_global;		/* Highest_Sequence_Number para cuando cierro el servidor */

void servidor(int mi_cliente)
{
	numero_muerte_global = 0;
	int clock = 0;
	int cantRanks = cant_ranks;
	int cantReply = 0;
	MPI_Status status; int origen, tag, tagServidor, origenServidor;
	int hay_pedido_local = FALSE;
	int listo_para_salir = FALSE;
	int buffer, count = 1;
	int replyPendientes[cantRanks/2];
	int i = 0;
	int servidores = cant_ranks / 2;
	int estadoServidores[cantRanks/2];
	while(i<cantRanks/2){
		replyPendientes[i] = FALSE;	
		i++;
	}
	for(i = 0; i < cant_ranks / 2; i++) estadoServidores[i] = TRUE;
	
	while( ! listo_para_salir ) {

		MPI_Recv(&buffer, count, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
		origen = status.MPI_SOURCE;
		tag = status.MPI_TAG;
		clock++;
		if (tag == TAG_PEDIDO) {
			assert(origen == mi_cliente);
			debug("Mi cliente solicita acceso exclusivo");
			assert(hay_pedido_local == FALSE);
			hay_pedido_local = TRUE;
			i = 0;
			while(i<cantRanks){
				clock++;
				if(i!=mi_cliente-1) //No me lo mando a mi mismo
					MPI_Send(&clock, 1, MPI_INT, i, TAG_REQUEST, COMM_WORLD);				
				i+=2;
			}
			while(cantReply != (cantRanks-1)/2){
				MPI_Recv(&buffer, count, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);	
				tagServidor = status.MPI_TAG;
				origenServidor = status.MPI_SOURCE;
				switch(tagServidor){
					case TAG_REQUEST:
							debug("ME LLEGO REQUEST DE" + origenServidor);
							if(buffer<clock){
								MPI_Send(&clock, 1, MPI_INT, origenServidor, TAG_REPLY, COMM_WORLD);											
							} else if(buffer>clock) {
								clock = buffer + 1;	
								replyPendientes[origenServidor/2] = TRUE;											
							}else{
								if(origen < mi_rank) MPI_Send(NULL, 0, MPI_INT, origen, TAG_REPLY, COMM_WORLD);	/* Le doy el OK */
										else replyPendientes[origenServidor/2] = TRUE;
							}
							break;
					case TAG_REPLY: debug("ME LLEGO REPLY DE" + origenServidor); cantReply++;
							break;
					case TAG_SERVER_DOWN:

							servidores--;
							estadoServidores[origen/2] = FALSE;
							MPI_Send(NULL, 0, MPI_INT, origen, TAG_GOAWAY, COMM_WORLD);
								
							if(clock > numero_muerte_global) numero_muerte_global = clock;
																							
							
							break;			
				}

			}
			debug("Dándole permiso (frutesco por ahora) => COPADO");
			MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
		}

		else if (tag == TAG_LIBERO) {
			assert(origen == mi_cliente);
			debug("Mi cliente libera su acceso exclusivo");
			assert(hay_pedido_local == TRUE);
			hay_pedido_local = FALSE;
			i = 0;
			while(i<cantRanks/2){
				if(replyPendientes[i]==TRUE){
					debug("REPLY PENDIENTE A" + i*2);
					MPI_Send(&clock, 1, MPI_INT, i*2, TAG_REPLY, COMM_WORLD);																
				}	
				i++;
			}
		}

		else if (tag == TAG_TERMINE) {
			assert(origen == mi_cliente);
			debug("Mi cliente avisa que terminó");
			listo_para_salir = TRUE;
		}
		else if(tag == TAG_REQUEST){
			debug("ME LLEGO UN PUTO REQUEST SOLO");
			clock++;
			MPI_Send(&clock, 1, MPI_INT, origen, TAG_REPLY, COMM_WORLD);												
		}

	}

}

