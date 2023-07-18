#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>


void Dispatcher(void *arg);
task_t *scheduler();
void inicia_tempo();
void tratador(int signum);
void diskDispatcher(void *arg);
