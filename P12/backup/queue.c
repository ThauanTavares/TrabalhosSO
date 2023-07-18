#include "queue.h"
#include <stdio.h>


int queue_size(queue_t *queue) {
    int tam = 0;
  
    if (queue == NULL) {
        return 0;
    } else {
        queue_t *aux = queue;
        do {
            tam++;
            aux = aux->next;
        } while (aux != queue);
        //printf("SIZE: tam %d\n", tam);
        return tam;
    }
}



void queue_print (char *name, queue_t *queue, void print_elem (void*) ){
    
    if (queue == NULL) {
        //printf("A lista %s está vazia\n", name);
        printf("%s:\n", name);

        return;
    }
    
    printf("%s:[", name);
    queue_t *aux = queue;
    
    do {
        print_elem(aux);
        aux = aux->next;
    } while (aux != queue);
    printf("]\n");
    
}




int queue_append(queue_t **queue, queue_t *elem) {
    // verifica se o elemento já está em outra fila
    if (elem->prev != NULL || elem->next != NULL) {
        return -1;
    }

    queue_t *aux = *queue;
    int tem = 0;

    //lista vazia
    if (*queue == NULL) {

        elem->prev = elem;
        elem->next = elem;
        *queue = elem;
    } else {

        //procura o elemento na lista
        do {
            if (aux == elem) {
                tem = 1;
            }
            aux = aux->next;
        } while (aux != *queue);
    // nao tem insere
        if (tem == 0) {
            elem->next = *queue;
            elem->prev = (*queue)->prev;
            (*queue)->prev->next = elem;
            (*queue)->prev = elem;
        }
    }
    return 0;
}


int queue_remove(queue_t **queue, queue_t *elem) {
    queue_t *aux = *queue;

    //lista vazia
    if (*queue == NULL) {
        return -1;
    }

    if (aux == elem) {

        // primeiro da fila
        if (aux->next == *queue) {
            *queue = NULL;
        } else {
            *queue = aux->next;
            aux->prev->next = aux->next;
            aux->next->prev = aux->prev;
        }
        aux->prev = NULL;
        aux->next = NULL;
        return 0;
    }

    // procura na lista
    do {
        if (aux == elem) {
            aux->prev->next = aux->next;
            aux->next->prev = aux->prev;
            aux->prev = NULL;
            aux->next = NULL;
            return 0;
        }
        aux = aux->next;
    } while (aux != *queue);

    return -1;
}
