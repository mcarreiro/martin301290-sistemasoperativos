#include "srv.h"

/*
 *  Ejemplo de servidor que tiene el "sí fácil" para con su
 *  cliente y no se lleva bien con los demás servidores.
 *
 */

		

void servidor(int mi_cliente)
{

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

	int Highest_Sequence_Number = 0;
	char str [100];
	int cantRespuestas = 0;
	//Al empezar nadie pidió nada, nadie se quiere ir y están todos arriba
	while(i<cantRanks/2){
		replyPendientes[i] = FALSE;	
		goAwayPendientes[i] = FALSE;
		estadoServidores[i] = TRUE;
		i++;
	}
	
	while( ! listo_para_salir ) { //Hasta que no me quiera ir y me dejen ir, sigo en juego

		MPI_Recv(&buffer, count, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
		origen = status.MPI_SOURCE;
		tag = status.MPI_TAG;
		//Recibí un mensaje, hay que ver qué era...
		if (tag == TAG_PEDIDO) {
			//Pedido de sección crítica de mi cliente
			assert(origen == mi_cliente);
			debug("Mi cliente solicita acceso exclusivo");
			assert(hay_pedido_local == FALSE);
			hay_pedido_local = TRUE;
			i = 0;
			//Al ser un pedido seteo mi clock con el último que guardé del último request que recibí
			clock = Highest_Sequence_Number + 1;
			while(i<cantRanks){
				//Mando el pedido al resto de los servidores activos
				if(i!=mi_cliente-1 && estadoServidores[i/2] == TRUE) //No me lo mando a mi mismo y se lo mande alguien que este vivo
					MPI_Send(&clock, 1, MPI_INT, i, TAG_REQUEST, COMM_WORLD);				
				i+=2;
			}
			
			cantReply = 0;	
			cantRespuestas = (servidores - 1);
			while(cantReply < cantRespuestas){
				//Espero las respuestas antes de darle el OK a mi cliente
				MPI_Recv(&buffer, count, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);	
				tagServidor = status.MPI_TAG;
				origenServidor = status.MPI_SOURCE;
				//Mientras espero las respuestas veo qué tipo de mensajes recibo
				switch(tagServidor){
					case TAG_REQUEST:
							//Recibí otro request, hay que ver la prioridad basado en el clock de cada uno y en caso de ser iguales el número de rank
							debug("ME LLEGO REQUEST DE" + origenServidor);
							if(buffer<clock){
								//Tiene menor clock, entonces tiene prioridad
								MPI_Send(&clock, 1, MPI_INT, origenServidor, TAG_REPLY, COMM_WORLD);											
							} else if(buffer>clock) {				
								//Tengo más prioridad, pero entonces encontré hasta ahora el clock más alto, 	
								//entonces mi proximo pedido debe tener uno más que este			
								Highest_Sequence_Number = buffer;
								replyPendientes[origenServidor/2] = TRUE;											
							}else{
								if(origenServidor < mi_rank) MPI_Send(NULL, 0, MPI_INT, origenServidor, TAG_REPLY, COMM_WORLD);	/* Le doy el OK */
										else replyPendientes[origenServidor/2] = TRUE;
							}
							break;
					case TAG_REPLY: debug("ME LLEGO REPLY DE" + origenServidor); cantReply++; //Cuento nuevo reply
							break;
					case TAG_SERVER_DOWN: //Si se quiere ir, resto la cantidad de servidores pero no sumo cantReply, porque estaría avanzando en dos los OK
							servidores--;											
							estadoServidores[origenServidor/2] = FALSE;
							MPI_Send(NULL, 0, MPI_INT, origenServidor, TAG_GOAWAY, COMM_WORLD);

							break;			
				}

			}
			debug("Dándole permiso (frutesco por ahora) => COPADO");
			//Recibí todos los reply necesarios, entonces mi cliente tiene exclusión mutua sobre el recurso
			MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
		}

		else if (tag == TAG_LIBERO) {
			//Mi cliente terminó debo liberar y darle el REPLY a los que antes retrasé
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
			//Como mi cliente terminó, entonces no tengo que hacer nada más como servidor y me quiero ir
			assert(origen == mi_cliente);
				debug("Mi cliente avisa que terminó");
				listo_para_salir = TRUE;
				assert(hay_pedido_local == FALSE);
				
				//Les digo a mis amigos que me quiero ir
				i = 0;
				while(i<cantRanks){					
					if(i!=mi_cliente-1 && estadoServidores[i/2] == TRUE) 
						MPI_Send(&clock, 1, MPI_INT, i, TAG_SERVER_DOWN, COMM_WORLD);				
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
							if(buffer < clock){									
								MPI_Send(NULL, 0, MPI_INT, origen, TAG_GOAWAY, COMM_WORLD);
								estadoServidores[origen / 2] = FALSE;
							}	
							else
							{
								if(buffer > clock){
									goAwayPendientes[origen / 2] = TRUE;

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
				//ya me puedo ir, pero les aviso a los demas que tambien pueden irse
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
			//Me llegó un pedido y no estaba haciendo nada entoces veo si mi cliente sigue laburando
			if(!hay_pedido_local){
				debug("LLEGO UN REQUEST QUE ACEPTO Y CONTESTO CON REPLY");
				
				MPI_Send(NULL, 0, MPI_INT, origen, TAG_REPLY, COMM_WORLD);
			}else{
				debug("MI CLIENTE NO LIBERO TODAVIA, WAIT");
				replyPendientes[origen/2] = TRUE;	
			}
			//Al igual que antes encontré un clock más alto, entonces lo guardo para el próximo request
			if(Highest_Sequence_Number < buffer) Highest_Sequence_Number = buffer;								
		}
		else if(tag == TAG_SERVER_DOWN){
			//Alguien se quiere ir
			servidores--;
			estadoServidores[origen/2] = FALSE;
			MPI_Send(NULL, 0, MPI_INT, origen, TAG_GOAWAY, COMM_WORLD);						

		}
	}

}

