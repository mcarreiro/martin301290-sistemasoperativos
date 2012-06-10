#include "Backend_pthreads.h"
#include "pthread.h"

#include "RWSemaphoreLock.cpp"


using namespace std;


// variables globales de la conexión
int socket_servidor = -1;


// variables globales del juego
vector<vector<char> > tablero_letras; // tiene letras que aún no son palabras válidas
vector<vector<char> > tablero_palabras;	// solamente tiene las palabras válidas
vector< vector<RWSemaphoreLock> > semaforosLetras;// semaforo para posiciones del tablero de letras
RWSemaphoreLock semaforoPalabra;//semaforo para el tablero de palabras



unsigned int ancho = -1;
unsigned int alto = -1;

bool cargar_int(const char* numero, unsigned int& n) {
	char *eptr;
	n = static_cast<unsigned int>(strtol(numero, &eptr, 10));
	if(*eptr != '\0') {
		cerr << "error: " << numero << " no es un número: " << endl;
		return false;
	}
	return true;
}

int main(int argc, const char* argv[]) {
	// manejo la señal SIGINT para poder cerrar el socket cuando cierra el programa
	signal(SIGINT, cerrar_servidor);

	// parsear argumentos
	if (argc < 3) {
		cerr << "Faltan argumentos, la forma de uso es:" << endl <<
		argv[0] << " N M" << endl << "N = ancho del tablero , M = alto del tablero" << endl;
		return 3;
	}
	else {
		if (!cargar_int(argv[1], ancho)) {
			cerr << argv[1] << " debe ser un número" << endl;
			return 5;
		}
		if (!cargar_int(argv[2], alto)) {
			cerr << argv[2] << " debe ser un número" << endl;
			return 5;
		}
	}
	//creo las filas del tablero de semoforos de letras
	semaforosLetras = vector<vector<RWSemaphoreLock> >(alto); 
	// inicializar ambos tableros, se accede como tablero[fila][columna]
	tablero_letras = vector<vector<char> >(alto);
	for (unsigned int i = 0; i < alto; ++i) {
		//Creo lo semaforos de cada columna
		semaforosLetras[i] =  vector<RWSemaphoreLock>((ancho), RWSemaphoreLock());
		tablero_letras[i] = vector<char>(ancho, VACIO);
	}
	
	

	tablero_palabras = vector<vector<char> >(alto);
	for (unsigned int i = 0; i < alto; ++i) {
		
		tablero_palabras[i] = vector<char>(ancho, VACIO);
	}
	
	semaforoPalabra = RWSemaphoreLock();									/* Inicializo el lock del tablero_palabras */
	
	int socketfd_cliente, socket_size;
	struct sockaddr_in local, remoto;

	// crear un socket de tipo INET con TCP (SOCK_STREAM)
	if ((socket_servidor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		cerr << "Error creando socket" << endl;
	}
	// permito reusar el socket para que no tire el error "Address Already in Use"
	int flag = 1;
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	// crear nombre, usamos INADDR_ANY para indicar que cualquiera puede conectarse aquí
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(PORT);
	if (bind(socket_servidor, (struct sockaddr *)&local, sizeof(local)) == -1) {
		cerr << "Error haciendo bind!" << endl;
        return 1;
	}

	// escuchar en el socket 
	if (listen(socket_servidor, 1) == -1) {
		cerr << "Error escuchando socket!" << endl;
        return 1;
	}
	
	// aceptar conexiones entrantes.
	socket_size = sizeof(remoto);	
	while (true) {
		if ((socketfd_cliente = accept(socket_servidor, (struct sockaddr*) &remoto, (socklen_t*) &socket_size)) == -1)
			cerr << "Error al aceptar conexion" << endl;
		else {
			//creo el thread que va a handlear la nueva conexion
			pthread_t jugador_thread;
			pthread_create(&jugador_thread, NULL, atendedorDeJugadorWrapper, (void*) &socketfd_cliente);
									
		}
	}

	return 0;
}

void* atendedorDeJugadorWrapper(void* socket_fd)
{
	atendedor_de_jugador(*((int*) socket_fd));
	return NULL;
}

void atendedor_de_jugador(int socket_fd) {
	
	// variables locales del jugador
	char nombre_jugador[21];
	list<Casillero> palabra_actual; // lista de letras de la palabra aún no confirmada

	if (recibir_nombre(socket_fd, nombre_jugador) != 0) {
		// el cliente cortó la comunicación, o hubo un error. Cierro el thread.
		terminar_servidor_de_jugador(socket_fd, palabra_actual);
	}

	if (enviar_dimensiones(socket_fd) != 0) {
		// se produjo un error al enviar. Cierro el thread.
		terminar_servidor_de_jugador(socket_fd, palabra_actual);
	}

	cout << "Esperando que juegue " << nombre_jugador << endl;

	while (true) {
		// espera una letra o una confirmación de palabra
		char mensaje[MENSAJE_MAXIMO+1];
		int comando = recibir_comando(socket_fd, mensaje);
		if (comando == MSG_LETRA) {
			Casillero ficha;
			if (parsear_casillero(mensaje, ficha) != 0) {
				// no es un mensaje LETRA bien formado, hacer de cuenta que nunca llegó
				continue;
			}
			// ficha contiene la nueva letra a colocar
			// verificar si es una posición válida del tablero
			if (es_ficha_valida_en_palabra(ficha, palabra_actual)) {
				
				//Espero que se libere la casilla para poder escribir la letra			
				semaforosLetras[ficha.fila] [ficha.columna].wlock();
				
				if(tablero_letras[ficha.fila][ficha.columna] == VACIO)
				{
					/* Pongo la ficha */
										
					palabra_actual.push_back(ficha);
					tablero_letras[ficha.fila][ficha.columna] = ficha.letra;
					//libero la posicion
					semaforosLetras[ficha.fila] [ficha.columna].wunlock();
					
					/* Envio el OK */
					
					if (enviar_ok(socket_fd) != 0)
					{
						// se produjo un error al enviar. Cierro el thread.
						terminar_servidor_de_jugador(socket_fd, palabra_actual);
						break;
					}
					
					continue;				
				}
				else
				{
					/* No pongo la ficha */
					//libero la posicion					
					semaforosLetras[ficha.fila][ficha.columna].wunlock();
					quitar_letras(palabra_actual);
					
					// ERROR
					if (enviar_error(socket_fd) != 0)
					{
						// se produjo un error al enviar. Cierro el thread.
						terminar_servidor_de_jugador(socket_fd, palabra_actual);
						break;
					}
					
					continue;
				}
			}
			else {
								
				quitar_letras(palabra_actual);
				
				// ERROR
				if (enviar_error(socket_fd) != 0) {
					// se produjo un error al enviar. Cierro el thread.
					terminar_servidor_de_jugador(socket_fd, palabra_actual);
					break;
				}
				
				continue;
			}
		}
		else if (comando == MSG_PALABRA) {
			// las letras acumuladas conforman una palabra completa, escribirlas en el tablero de palabras y borrar las letras temporales
			//Espero que se libere el tablero para agregar la palabra
			semaforoPalabra.wlock();		
			
			for (list<Casillero>::const_iterator casillero = palabra_actual.begin(); casillero != palabra_actual.end(); casillero++) {
				
				tablero_palabras[casillero->fila][casillero->columna] = casillero->letra;
			}
			palabra_actual.clear();
			//libero el tablero
			semaforoPalabra.wunlock();

			if (enviar_ok(socket_fd) != 0) {
				// se produjo un error al enviar. Cierro el thread.
				terminar_servidor_de_jugador(socket_fd, palabra_actual);
				break;
			}
		}
		else if (comando == MSG_UPDATE) {
			if (enviar_tablero(socket_fd) != 0) {
				// se produjo un error al enviar. Cierro el thread.
				terminar_servidor_de_jugador(socket_fd, palabra_actual);
				break;
			}
		}
		else if (comando == MSG_INVALID) {
			// no es un mensaje válido, hacer de cuenta que nunca llegó
			continue;
		}
		else {
			// se produjo un error al recibir. Cierro el thread.
			terminar_servidor_de_jugador(socket_fd, palabra_actual);
			break;
		}
	}
}


// mensajes recibidos por el server

int recibir_nombre(int socket_fd, char* nombre) {
	char buf[MENSAJE_MAXIMO+1];

	if (recibir(socket_fd, buf) != 0) {
		return -1;
	}

	int res = sscanf(buf, "SOY %20s", nombre);
	
	if (res == EOF || res != 1) {
		cerr << "ERROR: no se pudo leer el nombre del cliente" << endl;
		return -1;
	}

	return 0;
}

// informa el tipo de comando recibido (LETRA, PALABRA, UPDATE o si es inválido) y deja el mensaje en mensaje por si necesita seguir parseando
int recibir_comando(int socket_fd, char* mensaje) {
	if (recibir(socket_fd, mensaje) != 0) {
		return -1;
	}

	char comando[MENSAJE_MAXIMO];
	sscanf(mensaje, "%s", comando);

	if (strcmp(comando, "LETRA") == 0) {
		// el mensaje es LETRA
		return MSG_LETRA;
	}
	else if (strcmp(comando, "PALABRA") == 0) {
		// el mensaje es PALABRA
		return MSG_PALABRA;
	}
	else if (strcmp(comando, "UPDATE") == 0) {
		// el mensaje es UPDATE
		return MSG_UPDATE;
	}
	else {
		cerr << "ERROR: mensaje no válido" << endl;
		return MSG_INVALID;
	}
}

int parsear_casillero(char* mensaje, Casillero& ficha) {
	int letra = sscanf(mensaje, "LETRA %d %d %c", &ficha.fila, &ficha.columna, &ficha.letra);

	if (letra == 3) {
		// el mensaje es LETRA y ficha contiene la ficha que desea colocar
		return 0;
	}
	else {
		cerr << "ERROR: " << mensaje << " no está bien formado. Debe ser LETRA <fila> <columna> <letra>" << endl;
		return -1;
	}
}

// mensajes enviados por el server

int enviar_dimensiones(int socket_fd) {
	char buf[MENSAJE_MAXIMO+1];
	sprintf(buf, "TABLERO %d %d", ancho, alto);
	return enviar(socket_fd, buf);
}

int enviar_tablero(int socket_fd) {
		
	char buf[MENSAJE_MAXIMO+1];
	sprintf(buf, "STATUS ");
	int pos = 7;
	//espero para podeer leer el tablero de palabras
	semaforoPalabra.rlock();
	
	for (unsigned int fila = 0; fila < alto; ++fila) {
		for (unsigned int col = 0; col < ancho; ++col) {
			char letra = tablero_palabras[fila][col];
			buf[pos] = (letra == VACIO)? '-' : letra;
			pos++;
		}
	}
	//Libero el tablero de palabars
	semaforoPalabra.runlock();	
		
	buf[pos] = 0; //end of buffer

	return enviar(socket_fd, buf);
}

int enviar_ok(int socket_fd) {
	char buf[MENSAJE_MAXIMO+1];
	sprintf(buf, "OK");
	return enviar(socket_fd, buf);
}

int enviar_error(int socket_fd) {
	char buf[MENSAJE_MAXIMO+1];
	sprintf(buf, "ERROR");
	return enviar(socket_fd, buf);
}


// otras funciones

void cerrar_servidor(int signal) {
	cout << "¡Adiós mundo cruel!" << endl;
	if (socket_servidor != -1)
		close(socket_servidor);
	exit(EXIT_SUCCESS);
}

void terminar_servidor_de_jugador(int socket_fd, list<Casillero>& palabra_actual) {
	cout << "Se interrumpió la comunicación con un cliente" << endl;

	close(socket_fd);

	quitar_letras(palabra_actual);
	
	pthread_exit(NULL);		/* Detengo el thread */

	// exit(-1);	/* Saco el exit porque no quiero que el thread termine por aca */
}


void quitar_letras(list<Casillero>& palabra_actual) {
	for (list<Casillero>::const_iterator casillero = palabra_actual.begin(); casillero != palabra_actual.end(); casillero++) {
		//Espero el recurso para escribir en la posicion correspondiente del tablero de letras
		semaforosLetras[casillero->fila][casillero->columna].wlock();
		
		
		tablero_letras[casillero->fila][casillero->columna] = VACIO;
		
		//Libero la posicion del tablero de letras
		semaforosLetras[casillero->fila ][ casillero->columna].wunlock();
	}
	palabra_actual.clear();
}


bool es_ficha_valida_en_palabra(const Casillero& ficha, const list<Casillero>& palabra_actual) {
		
	// si está fuera del tablero, no es válida
	if (ficha.fila < 0 || ficha.fila > alto - 1 || ficha.columna < 0 || ficha.columna > ancho - 1) {
		return false;
	}
	
	/* Veo primero si el casillero esta ocupado */
	
	semaforosLetras[ficha.fila ][ficha.columna].rlock();			/* Pido un lock de LECTURA */
	if(tablero_letras[ficha.fila][ficha.columna] != VACIO)
	{
		/* Si el casillero estaba ocupado, devuelvo falso */
		
		semaforosLetras[ficha.fila][ ficha.columna].runlock();	/* Libero el lock de LECTURA */
		return false;
	}
	
	semaforosLetras[ficha.fila][ ficha.columna].runlock();		/* Libero el lock de LECTURA */
					 
	if (palabra_actual.size() > 0) {
		
		// no es la primera letra de la palabra, ya hay fichas colocadas para esta palabra
		Casillero mas_distante = casillero_mas_distante_de(ficha, palabra_actual);
		int distancia_vertical = ficha.fila - mas_distante.fila;
		int distancia_horizontal = ficha.columna - mas_distante.columna;

		if (distancia_vertical == 0) {
			// la palabra es horizontal
			for (list<Casillero>::const_iterator casillero = palabra_actual.begin(); casillero != palabra_actual.end(); casillero++) {
				if (ficha.fila - casillero->fila != 0) {
					// no están alineadas horizontalmente										
					return false;
				}
			}
						
			int paso = distancia_horizontal / abs(distancia_horizontal);
			for (unsigned int columna = mas_distante.columna; columna != ficha.columna; columna += paso) {
				// el casillero DEBE estar ocupado en el tablero de palabras
				if (!(puso_letra_en(ficha.fila, columna, palabra_actual)) && tablero_palabras[ficha.fila][columna] == VACIO) {					
					return false;
				}
			}
			
		} else if (distancia_horizontal == 0) {
			// la palabra es vertical
			for (list<Casillero>::const_iterator casillero = palabra_actual.begin(); casillero != palabra_actual.end(); casillero++) {
				if (ficha.columna - casillero->columna != 0) {
					// no están alineadas verticalmente					
					return false;
				}
			}

			int paso = distancia_vertical / abs(distancia_vertical);
			for (unsigned int fila = mas_distante.fila; fila != ficha.fila; fila += paso) {
				// el casillero DEBE estar ocupado en el tablero de palabras
				if (!(puso_letra_en(fila, ficha.columna, palabra_actual)) && tablero_palabras[fila][ficha.columna] == VACIO) {					
					return false;
				}
			}
		}
		else {
			// no están alineadas ni horizontal ni verticalmente			
			return false;
		}
	}

	return true;
}


Casillero casillero_mas_distante_de(const Casillero& ficha, const list<Casillero>& palabra_actual) {
	const Casillero* mas_distante;
	int max_distancia = -1;
	for (list<Casillero>::const_iterator casillero = palabra_actual.begin(); casillero != palabra_actual.end(); casillero++) {
		int distancia = max<unsigned int>(abs((int)(casillero->fila - ficha.fila)), abs((int)(casillero->columna - ficha.columna)));
		if (distancia > max_distancia) {
			max_distancia = distancia;
			mas_distante = &*casillero;
		}
	}

	return *mas_distante;
}


bool puso_letra_en(unsigned int fila, unsigned int columna, const list<Casillero>& letras) {
	for (list<Casillero>::const_iterator casillero = letras.begin(); casillero != letras.end(); casillero++) {
		if (casillero->fila == fila && casillero->columna == columna)
			return true;
	}
	// si no encontró
	return false;
}

