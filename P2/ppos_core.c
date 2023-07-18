#include "ppos.h"
#include "ppos_data.h"
#include <stdlib.h>
#include <stdio.h>

//int qnt_task=0;

task_t *atual;
task_t *mainIni;

void ppos_init (){
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0);

    task_t *main = malloc (sizeof(task_t));

    main->next = NULL;
    main->prev = NULL;
    main->id = 0;
    main->status = RODANDO;
    atual=main;
    mainIni=main;



}

int task_init (task_t *task, void (*start_routine)(void *),  void *arg){

    static int qntTaskId = 0;
    char* stack;

    qntTaskId++;

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
    
    
    return task->id;

}

int task_switch (task_t *task){

    task->prev = atual;
    atual=task;
    
    if(swapcontext(&task->prev->context, &task->context)== -1){
        return -1;
    }

    return 0;

}

void task_exit (int exit_code){

    if(atual->id == 1){
        task_switch(mainIni);

    }else{
       task_switch(atual->prev);
    }    
    return;
}   

int task_id (){
    return atual->id;
}
