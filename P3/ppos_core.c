#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include "dispatcher.h"
#include <stdlib.h>
#include <stdio.h>

//int qnt_task=0;
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
    //&dispatcher = malloc(sizeof(task_t));

    main->next = NULL;
    main->prev = NULL;
    main->id = 0;
    main->status = PRONTA;
    atual=main;
    mainIni=main;
    //=qntTaskId++;
    //dispatcher=taskDispacher;
    //queue_append((queue_t**)&filaProntos,(queue_t*)main);
    task_init(&dispatcher,Dispatcher,"dispacher");
    //filaProntos=NULL;

    
}

int task_init (task_t *task, void (*start_routine)(void *),  void *arg){

    //printf ("%s = arg task_init\n", (char *) arg) ;
    char* stack;

    qntTaskId++;
    //printf("qntTaskiID %d\n",qntTaskId);
    //printf ("%s = arg task_init task = %d\n", (char *) arg, qntTaskId) ;

    task->next = NULL;
    task->prev = NULL;
    task->id = qntTaskId;
    task->status = PRONTA;

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
    task_t* aux;

    //task->prev = atual;
    aux=atual;
    atual=task;
    
    
    /*task->prev = aux;
    aux=task;*/
    
    if(swapcontext(&aux->context, &task->context)== -1){
        return -1;
    }

    return 0;

}

void task_exit (int exit_code){

    if (atual->id >= 0) {
       // printf("Terminou a main \n");
        atual->status = TERMINADA;
        task_switch(&dispatcher);
    } else {
        /*task_t* temp = atual;
        atual = atual->prev;
        queue_remove((queue_t**)&filaProntos, (queue_t*)temp);
        qntTaskId--;
        atual->status = TERMINADA;*/
        task_switch(mainIni);
    }
}   

int task_id (){
    return atual->id;
}

void task_yield(){
    task_switch(&dispatcher);
}



void Dispatcher(void *arg){
    task_t *next;
    //printf ("%s: ENTROU\n", (char *) arg) ;

    while (qntTaskId > 0){
       // printf("dentrou whiloe\n");
        next=scheduler();
      //  printf("task = %d \n",next->id);
        if(next != NULL){
           // printf("passou IF\n");
            //atual = next;
            //printf("id next = %d \n", next->id);
            task_switch(next);
           // printf("valor task switch %d\n",task_switch(next));
           // printf("passou taskswitch\n");
            //printf("Status =  %d\n", next->status);
            switch (next->status){
                case PRONTA:
                    /* code *///queue_append((queue_t**)&filaProntos,(queue_t*)next);
                   // printf("fez o append de novo\n");
                    break;
                case TERMINADA:
                    queue_remove((queue_t**)&filaProntos, (queue_t*)atual);
             //       printf("Entrei em terminada\n");
                    break;
            }
           // printf("passou switch\n");
        }
    }
    task_exit(0);
    
}

task_t *scheduler(){
    task_t* aux;
    //printf("tamanho fila %d \n",queue_size((queue_t*)filaProntos));
    //printf("ptarefa dentro da fila atual = %d\n", filaProntos->id);
    //printf("ultimo da fila atual = %d\n", filaProntos->prev->id);
    //printf("prox da fila atual = %d\n", filaProntos->next->id);


    //printf("cabeca fila %d \n ", filaProntos->id );
    if (filaProntos == NULL) {
        return NULL;
    }

    aux = filaProntos;
    filaProntos=filaProntos->next;
    //atual=aux->next;
   // printf("proxima tarefa dentro da fila = %d\n", aux->next->id);
    //queue_remove((queue_t**)&filaProntos, (queue_t*)aux);
    //queue_append((queue_t**)&filaProntos,(queue_t*)aux);
    
   // printf("id aux = %d \n", aux->id);
    return aux;
    
}