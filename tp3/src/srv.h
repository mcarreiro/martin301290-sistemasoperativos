#ifndef __srv_h__
#define __srv_h__

#include "tp3.h"
#define TAG_REQUEST           60    
#define TAG_REPLY             70    
#define TAG_SERVER_DOWN       80    
#define TAG_GOAWAY		      90  

void servidor(int mi_cliente);

void handle(int tag, int pendingResponses[], int statusServers[], int origen, int mi_cliente,MPI_Status status);
#endif
