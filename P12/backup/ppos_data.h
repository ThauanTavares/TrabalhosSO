// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__


#define RODANDO 1
#define PRONTA 2
#define TERMINADA 3
#define SUSPENSA 4
#define STACKSIZE 64*1024
#define AGING -1

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include <stdatomic.h>

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next, *esperando;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  // ... (outros campos serão adicionados mais tarde)


  int prioridadeInicial; /*Prioridade INICIAL para salvar a prioridade passada*/
 
  int prioridadeDinamica; /*Prioridade Dinamica para receber o AGING */

  int quantum;

  unsigned int tempoNascimento;

  unsigned int tempoExit;

  unsigned int tempoProcessador;

  unsigned int ativacao;

  int suspendeuTarefa;

  int ec;

  int tempoDormindo;

} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
  int contador;
  struct task_t *fila;
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
struct itemFila{
  struct itemFila *prev,*next;
  void *item;
};

typedef struct
{
  // preencher quando necessário
  struct itemFila *fila;

  int max;
  int tamanhoMensagem;
  int qntMensagem;
  semaphore_t semFila, semVaga, semMensagem;

} mqueue_t ;

#endif