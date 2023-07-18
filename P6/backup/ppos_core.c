//Projeto P6 - Sistemas Operacionais
//Professor - Carlos Maziero
//Desenvolvido por Thauan Tavares - GRR20171591 
//Data de criação: 15/05/2023
//Data de atualização: 18/05/2023



#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include "dispatcher.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

static int qntTaskId = 0;
int tempoTotal; // variavel global para contar os ticks

int flag;
task_t *atual;
task_t *mainIni;

task_t *filaProntos=NULL;
task_t dispatcher;
task_t *taskDispacher;


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
    main->status = PRONTA;
    atual=main;
    mainIni=main;
    main->ativacao++;

    
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

    atual->quantum=10;
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

    if (atual->id >= 0) {
       
        atual->status = TERMINADA;
        if(atual->id ==0){
            task_switch(mainIni);
       }
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

void Dispatcher(void *arg){
    task_t *next;
    unsigned int entrada;
    unsigned int saida;
   /*Dispatcher controla as tasks pegando elas  da fila de prontos
   caso pronta nao faz nada por hora, se TERMINADA remove da fila*/

    
    while (qntTaskId > 1){
        next=scheduler();
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