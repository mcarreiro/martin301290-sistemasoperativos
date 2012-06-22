#include "srv.h"

/*
 *  Ejemplo de servidor que tiene el "sí fácil" para con su
 *  cliente y no se lleva bien con los demás servidores.
 *
 */

int mi_numero_muerte;	
int numero_muerte_global;		
char str [45];
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
	int goAwayPendientes[cantRanks/2];
	int i = 0;
	int servidores = cant_ranks / 2;
	int estadoServidores[cantRanks/2];
	numero_muerte_global = 0;
	int Highest_Sequence_Number = 0;

	int cantRespuestas = 0;
	
	while(i<cantRanks/2){
		replyPendientes[i] = FALSE;	
		goAwayPendientes[i] = FALSE;
		i++;
	}
	for(i = 0; i < cant_ranks / 2; i++) estadoServidores[i] = TRUE;
	
	while( ! listo_para_salir ) {

		MPI_Recv(&buffer, count, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
		origen = status.MPI_SOURCE;
		tag = status.MPI_TAG;
		
		if (tag == TAG_PEDIDO) {
			assert(origen == mi_cliente);
			debug("Mi cliente solicita acceso exclusivo");
			assert(hay_pedido_local == FALSE);
			hay_pedido_local = TRUE;
			i = 0;
			clock = Highest_Sequence_Number + 1;
			while(i<cantRanks){
				
				if(i!=mi_cliente-1 && estadoServidores[i/2] == TRUE) //No me lo mando a mi mismo y se lo mande alguien que este vivo
					MPI_Send(&clock, 1, MPI_INT, i, TAG_REQUEST, COMM_WORLD);				
				i+=2;
			}
			
			cantReply = 0;	
			cantRespuestas = (servidores - 1);
			while(cantReply < cantRespuestas){
				MPI_Recv(&buffer, count, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);	
				tagServidor = status.MPI_TAG;
				origenServidor = status.MPI_SOURCE;
				switch(tagServidor){
					case TAG_REQUEST:
							debug("ME LLEGO REQUEST DE" + origenServidor);
							if(buffer<clock){
								MPI_Send(&clock, 1, MPI_INT, origenServidor, TAG_REPLY, COMM_WORLD);											
							} else if(buffer>clock) {								
								Highest_Sequence_Number = buffer;
								replyPendientes[origenServidor/2] = TRUE;											
							}else{
								if(origenServidor < mi_rank) MPI_Send(NULL, 0, MPI_INT, origenServidor, TAG_REPLY, COMM_WORLD);	/* Le doy el OK */
										else replyPendientes[origenServidor/2] = TRUE;
							}
							break;
					case TAG_REPLY: debug("ME LLEGO REPLY DE" + origenServidor); cantReply++;
							break;
					case TAG_SERVER_DOWN:
							servidores--;											
							estadoServidores[origenServidor/2] = FALSE;
							MPI_Send(NULL, 0, MPI_INT, origenServidor, TAG_GOAWAY, COMM_WORLD);
							if(buffer > numero_muerte_global) numero_muerte_global = buffer;
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
					replyPendientes[i] = FALSE;
					MPI_Send(&clock, 1, MPI_INT, i*2, TAG_REPLY, COMM_WORLD);																
				}	
				i++;
			}
		}

		else if (tag == TAG_TERMINE) {
			assert(origen == mi_cliente);
				debug("Mi cliente avisa que terminó");
				listo_para_salir = TRUE;
				assert(hay_pedido_local == FALSE);
				
				//Les digo a mis amigos que me quiero ir
				i = 0;
				mi_numero_muerte = numero_muerte_global + 1;
				while(i<cantRanks){					
					if(i!=mi_cliente-1 && estadoServidores[i/2] == TRUE) 
						MPI_Send(&mi_numero_muerte, 1, MPI_INT, i, TAG_SERVER_DOWN, COMM_WORLD);				
					i+=2;
				}
				cantReply = 0;
				sprintf(str, "Le tengo que avisar a %d servidores que termine", (int)(servidores - 1));
				debug(str);
				cantRespuestas = (servidores - 1);
				//Espero que me dejen ir los muy garcas
				while(cantReply < cantRespuestas){
					debug("estoy esperando que me dejen terminar");				
					MPI_Recv(&buffer, 1, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
					origen = status.MPI_SOURCE;
					tag = status.MPI_TAG;
					
					switch(tag){
						case(TAG_GOAWAY):
						{
							//se copo
							cantReply++;
							break;
						}
						
						case(TAG_REQUEST):
						{
							//Me pide algo y le digo de una q si, total me quiero ir far far away
							MPI_Send(NULL, 0, MPI_INT, origen, TAG_REPLY, COMM_WORLD);						
							break;
						}
						
						case(TAG_SERVER_DOWN):
						{
							//Llega otro flaco que tambien quiere irse, vamos a ver quien es mas importante!
							if(buffer < mi_numero_muerte){									
								MPI_Send(NULL, 0, MPI_INT, origen, TAG_GOAWAY, COMM_WORLD);
								estadoServidores[origen / 2] = FALSE;
							}	
							else
							{
								if(buffer > mi_numero_muerte){
									goAwayPendientes[origen / 2] = TRUE;
									numero_muerte_global = buffer;
								}
								else{
									if(origen < mi_rank){										
										MPI_Send(NULL, 0, MPI_INT, origen, TAG_GOAWAY, COMM_WORLD);
										estadoServidores[origen / 2] = FALSE;
									}
									else goAwayPendientes[origen / 2] = TRUE;
								}	
							}						

							break;
						}
					}
				}
				debug("ya les avise");
				//ya me puedo ir, pero como soy buena onda les aviso a los demas que tambien pueden irse
				for(i = 0; i < cant_ranks / 2; i++)
				{
					if(goAwayPendientes[i] == TRUE && estadoServidores[i] == TRUE)
					{
						assert(i * 2 != mi_rank);			
						servidores--;
						goAwayPendientes[i] = FALSE;
						MPI_Send(NULL, 0, MPI_INT, origen, TAG_GOAWAY, COMM_WORLD);
					}
				}	
		}
		else if(tag == TAG_REQUEST){
			if(!hay_pedido_local){
				debug("LLEGO UN REQUEST QUE ACEPTO Y CONTESTO CON REPLY");
				
				MPI_Send(NULL, 0, MPI_INT, origen, TAG_REPLY, COMM_WORLD);
			}else{
				debug("MI CLIENTE NO LIBERO TODAVIA, WAIT");
				replyPendientes[origen/2] = TRUE;	
			}
			
			if(Highest_Sequence_Number < buffer) Highest_Sequence_Number = buffer;								
		}
		else if(tag == TAG_SERVER_DOWN){
			servidores--;
			estadoServidores[origen/2] = FALSE;
			MPI_Send(NULL, 0, MPI_INT, origen, TAG_GOAWAY, COMM_WORLD);						
			if(buffer > numero_muerte_global) numero_muerte_global = buffer;
		}
	}

}

