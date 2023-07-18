//Projeto P6 - Sistemas Operacionais
//Professor - Carlos Maziero
//Desenvolvido por Thauan Tavares - GRR20171591 
//Data de criação: 30/05/2023
//Data de atualização: 31/05/2023



#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include "dispatcher.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <stdatomic.h>
#include <strings.h>


#define QUANTUM 10

static int qntTaskId = 0;
int tempoTotal; // variavel global para contar os ticks

int flag;
task_t *atual;
task_t *mainIni;

task_t *filaProntos=NULL;
task_t *filaSuspensas=NULL;
task_t *filaDormindo=NULL;

task_t dispatcher;
task_t *taskDispacher;
int lock;

void ppos_init (){

    inicia_tempo();

    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0);

    tempoTotal=0;
    task_t *main = malloc (sizeof(task_t));
/*Set do main*/
    main->next = NULL;
    main->prev = NULL;
    main->id = 0;
    main->tempoDormindo=-1;
    //main->prioridadeInicial=-20;
    main->status = PRONTA;
    atual=main;
    mainIni=main;
    main->ativacao++;
    main->tempoNascimento=systime();


    /*inicia o dispatcher*/

    task_init(&dispatcher,Dispatcher,"dispacher");

    
}

int task_init (task_t *task, void (*start_routine)(void *),  void *arg){
    /*Set das tasks e contador de tarefas e faz o append na fila de tarefas prontas*/
    char* stack;

    qntTaskId++;
    

    task->next = NULL;
    task->prev = NULL;
    task->id = qntTaskId;
    task->status = PRONTA;
    task->prioridadeInicial = 0;
    task->ativacao=0;
    task->tempoNascimento = systime();
    task->suspendeuTarefa=-1;
    task->tempoDormindo=-1;

    getcontext(&task->context);

    stack = malloc(STACKSIZE);

    if (stack){

      task->context.uc_stack.ss_sp = stack;
      task->context.uc_stack.ss_size = STACKSIZE;
      task->context.uc_stack.ss_flags = 0;
      task->context.uc_link = 0;
   
    }else{
   
      perror ("Erro na criação da pilha: ");
      return -1;
   
    }
    
    makecontext(&task->context,(void*)(*start_routine), 1, arg);
    
    queue_append((queue_t**)&filaProntos,(queue_t*)task);
    

    return task->id;

}

int task_switch (task_t *task){

    /*faz a troca de contexto do atual e do proximo*/
    task_t* aux;

    aux=atual;
    atual=task;
    
    
    atual->ativacao++;

    atual->quantum=QUANTUM;
    if(swapcontext(&aux->context, &task->context)== -1){
        return -1;
    }

    return 0;

}

void task_exit (int exit_code){

    /*Termina as tarefas*/


    //calcula o tempo final de execução e imprime valores solicitados
    atual->tempoExit=systime();
    printf("\ntask %d exit: execution time %d ms, processor time %d ms, %d activations \n", atual->id, atual->tempoExit, atual->tempoProcessador,atual->ativacao);

    /*Verifica se a tarefa suspendeu alguma e acorda qual foi na fila de Suspensas*/
    if(atual->suspendeuTarefa!=-1 && atual->id!=0){
       
        task_t *aux;
        int tam=-1;
        tam = queue_size((queue_t*)filaSuspensas);

        for(int i=0;i<tam;i++){
            aux=filaSuspensas;

            if(aux->id==atual->suspendeuTarefa){

                task_resume(aux,&filaSuspensas);
            }
            aux=aux->next;
        }
    }

    if (atual->id >= 0) {
       
        if(atual->id == 0){
            atual->status = TERMINADA;
            task_switch(mainIni);
            //task_switch(&dispatcher);
        }

        atual->status = TERMINADA;

        task_switch(&dispatcher);
    }
}   

int task_id (){
    return atual->id;

}

void task_yield(){
    task_switch(&dispatcher);
}

void task_setprio (task_t *task, int prio){
    /*Coloca a prioridade da tarefa verificando se é um valor valido
    se for NULL coloca na atual*/
    if(prio> 20 || prio < -20){
        printf("Prioridade Invalida");
        return;
    }
    if(task == NULL){
        atual->prioridadeInicial = prio;
        atual->prioridadeDinamica = prio;
    }else{
        task->prioridadeInicial=prio;
        task->prioridadeDinamica = prio;

    }
    return;
}

int task_getprio (task_t *task){
    /*Retorna o valor da prioridade, se NULL retorna da atual*/
    if(task == NULL){
        return atual->prioridadeInicial;
    }else{
        return task->prioridadeInicial;
    }
}

void task_suspend (task_t **queue){
  
    /*Suspende tarefa removendo da fila de prontos, colocando o status supensa e adicionando na fila de suspensas*/

    queue_remove((queue_t**)&filaProntos, (queue_t*)atual);
    atual->status = SUSPENSA;


    queue_append((queue_t**)&filaSuspensas,(queue_t*)atual);

    task_yield();
}

void task_resume (task_t *task, task_t **queue){
    
    

    /*Resume tarefa tirando da fila de suspensas e colocando na fila de prontas*/
    queue_remove((queue_t**)&filaSuspensas, (queue_t*)task);

    task->status=PRONTA;
    queue_append((queue_t**)&filaProntos,(queue_t*)task);
    //printf("Entreeei\n");


}
int task_wait (task_t *task){

    //Suspende a tarefa atual e salva o id ta tarefa suspendida
    if(task == NULL || task->status==TERMINADA){
        return -1;
    }

    
    task->suspendeuTarefa=atual->id;
    task_suspend(&filaProntos);
    
    return task->id;
}

void task_sleep (int t){

    int aux;

    aux=t+systime();


    atual->tempoDormindo=aux;
    
    task_suspend(&filaProntos);

   
}

void Dispatcher(void *arg){
    task_t *next;
    unsigned int entrada;
    unsigned int saida;
    int teste=1;
    task_t *aux;
    int tam=-1;

   
    
   /*Dispatcher controla as tasks pegando elas  da fila de prontos
   caso pronta nao faz nada por hora, se TERMINADA remove da fila*/

    
    while (qntTaskId > 0){
        tam = queue_size((queue_t*)filaSuspensas);

  
        aux=filaSuspensas;
       
        for(int i=0;i<tam;i++){
           
           
            if((tempoTotal >= aux->tempoDormindo)){
        
                if(aux->id!=0){
                
                    task_resume(aux,&filaSuspensas);

                }

            }
            aux=aux->next;
        }

        //printf("\nTam %d ", tam);
        //printf("\nTamPROJNTAS %d ", tamProntas);
        //
        next=scheduler();
     
        //printf("id next %d",next->id);
        if(next != NULL){
            entrada=systime();
            task_switch(next);
            if(next->id ==1 ){
                next->ativacao--;
            }
            saida=systime()-entrada;
            next->tempoProcessador+=saida;
            switch (next->status){
                case PRONTA:
                    break;
                case TERMINADA:
                    queue_remove((queue_t**)&filaProntos, (queue_t*)next);
                    qntTaskId--;
                    break;
            }
        }

    }
    if(teste==1){
        teste=0;
        next=atual;
        task_switch(next);
    }

    task_exit(0);
    
}

task_t *scheduler(){
    
    /*pega a cabeça da fila e o proximo e então percorre a fila olhando as prioridade */
    /*No final retorna a tarefa que vai receber CPU para ser executada*/
    
    task_t* aux;
    task_t* aux2;
    
    aux = filaProntos;
    aux2 = filaProntos->next;
   
    for (int i = 1; i < qntTaskId; i++) {
        if (aux2->prioridadeDinamica < aux->prioridadeDinamica) {
            aux = aux2;
        }
        aux2 = aux2->next;
    }
    
    aux2 = filaProntos;
    for (int i = 1; i < qntTaskId; i++){
        aux2->prioridadeDinamica+=AGING;
        aux2 = aux2->next;
    }
    
    aux->prioridadeDinamica=aux->prioridadeInicial;    
    filaProntos=filaProntos->next;

    return aux;
    
}


void inicia_tempo(){
    struct itimerval timer;
    int interval_msec = 1; 

    // Configuração do temporizador
    timer.it_value.tv_sec = 0;          
    timer.it_value.tv_usec = interval_msec * 1000;
    timer.it_interval.tv_sec = 0;            
    timer.it_interval.tv_usec = interval_msec * 1000; 

    // Configura o sinal associado ao temporizador (SIGALRM)
    signal(SIGALRM, tratador);

    // Ativa o temporizador
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Erro em setitimer");
        exit(1);
    }
}

void tratador(int signum){
    //tick do SO
    tempoTotal++;
    
    //controla o tempo da tarefa    
    if (atual != NULL && atual->id > 1){
        atual->quantum--;

        //Verifica se quantum chegou a zero se sim, Retorna o controle para o dispatcher   
        if (atual->quantum == 0){
   
            task_switch(&dispatcher);
        }
    }

}

unsigned int systime(){
    //retorna o tempo total utilizando a variavel global variavel global
    return tempoTotal;
}

void enter_cs (int *lock)
{
  // atomic OR (Intel macro for GCC)
  while (__sync_fetch_and_or (lock, 1));   // busy waiting
}
 
void leave_cs (int *lock)
{
  (*lock) = 0 ;
}



int sem_init(semaphore_t *s, int value) {

    if (s == NULL) {
        return -1;
    }

    if(s->fila != NULL){
        return -1;
    }

    s->contador = value;
    s->fila = NULL;
    return 0;
}


int sem_down (semaphore_t *s){
    if(s == NULL){
        return -1;
    }
    
    enter_cs(&lock);
    s->contador--;
    leave_cs(&lock);
    if(s->contador < 0){    
        task_suspend(&s->fila);
        if(s == NULL){
            return -1;
        }
    }
    return 0;

}

int sem_up (semaphore_t *s){
    if(s == NULL){
        return -1;
    }
    enter_cs(&lock);
    s->contador++;

    if(s->fila != NULL){
        task_resume(s->fila,&s->fila);
    }
    leave_cs(&lock);

    return 0;
    
}
int sem_destroy (semaphore_t *s){

    if(s == NULL){
        return -1;
    }

    enter_cs(&lock);

    while (s->fila != NULL){
        task_resume(s->fila,&s->fila);
    }
    if(s->fila != NULL){
        return -1;
    }
    s = NULL;
    
    leave_cs(&lock);
    return 0;

}

int mqueue_init (mqueue_t *queue, int max, int size){
    
    if(queue == NULL){
        return -1;
    }
    
    queue->max = max;
    queue->tamanhoMensagem = size;
    queue->fila = NULL;
    queue->qntMensagem=0;

    if(sem_init(&queue->semFila,1)){
        return -1;
    }

    if(sem_init(&queue->semMensagem,0)){
        return -1;
    }
    
    if(sem_init(&queue->semVaga,max)){
        return -1;
    }

    return 0;
}


int mqueue_send (mqueue_t *queue, void *msg){
    
    if(queue->qntMensagem < 0){
        queue = NULL;
        return -1;
    }
    
    sem_down(&queue->semVaga);
    if(queue->qntMensagem < 0){
        queue = NULL;
        return -1;
    }

    sem_down(&queue->semFila);
    if(queue->qntMensagem < 0){
        queue = NULL;
        return -1;
    }

    struct itemFila *mensagem = malloc (sizeof(struct itemFila));

    mensagem->item=(void *) malloc (queue->tamanhoMensagem);
    mensagem->next =NULL;
    mensagem->prev = NULL;
    
    bcopy(msg, &mensagem->item, queue->tamanhoMensagem);
    queue_append((queue_t**)&queue->fila,(queue_t*)mensagem);
    queue->qntMensagem++;

    sem_up(&queue->semFila);
    if(queue->qntMensagem < 0){
        queue = NULL;
        return -1;
    }

    sem_up(&queue->semMensagem);
    if(queue->qntMensagem < 0){
        queue = NULL;
        return -1;
    }

    return 0;
}

int mqueue_recv (mqueue_t *queue, void *msg){
    if(queue->qntMensagem < 0){
        queue = NULL;
        return -1;
    }
    
    sem_down(&queue->semMensagem);
    if(queue->qntMensagem < 0){
        queue = NULL;
        return -1;
    }

    sem_down(&queue->semFila);
    if(queue->qntMensagem < 0){
        queue = NULL;
        return -1;
    }

    bcopy(&queue->fila->item, msg, queue->tamanhoMensagem);
    queue_remove((queue_t**)&queue->fila,(queue_t*)queue->fila);
    queue->qntMensagem--;

    sem_up(&queue->semFila);
    if(queue->qntMensagem < 0){
        queue = NULL;
        return -1;
    }

    sem_up(&queue->semVaga);
    if(queue->qntMensagem < 0){
        queue = NULL;
        return -1;
    }

    return 0;
}

int mqueue_destroy (mqueue_t *queue){
    if(queue == NULL){
        return -1;
    }
    
    queue->qntMensagem = -1;
    free(queue->fila);
    sem_destroy(&queue->semFila);
    sem_destroy(&queue->semMensagem);
    sem_destroy(&queue->semVaga);

    return 0;

}

int mqueue_msgs (mqueue_t *queue){
    if(queue == NULL){
        return -1;
    }

    return queue->qntMensagem;
}

