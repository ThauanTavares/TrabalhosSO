#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include "dispatcher.h"
#include <stdlib.h>
#include <stdio.h>

static int qntTaskId = 0;
task_t *atual;
task_t *mainIni;

task_t *filaProntos=NULL;
task_t dispatcher;
task_t *taskDispacher;
void ppos_init (){
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0);

    task_t *main = malloc (sizeof(task_t));
/*Set do main*/
    main->next = NULL;
    main->prev = NULL;
    main->id = 0;
    main->status = PRONTA;
    atual=main;
    mainIni=main;

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
    
    
    
    
    if(swapcontext(&aux->context, &task->context)== -1){
        return -1;
    }

    return 0;

}

void task_exit (int exit_code){

    /*Termina as tarefas*/

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
   /*Dispatcher controla as tasks pegando elas  da fila de prontos
   caso pronta nao faz nada por hora, se TERMINADA remove da fila*/


    while (qntTaskId > 1){
        next=scheduler();
        if(next != NULL){
            task_switch(next);
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
