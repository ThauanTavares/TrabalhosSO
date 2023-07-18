#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include "ppos_disk.h"
#include "queue.h"
#include "ppos_data.h"
#include "disk.h"
#include "ppos_disk.h"





/*int disk_mgr_init (int *numBlocks, int *blockSize){
    
    disk_cmd(DISK_CMD_INIT,0,0);
    *numBlocks = disk_cmd(DISK_CMD_DISKSIZE,0,0);
    *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE,0,0);

    disk.sinal=0;
    disk.filaDisk = NULL;
    disk.filaSuspensaDisk=NULL;
    sem_init(&disk.semDisk,1);

    task_init(&disk.tarefaDisk,diskDispatcher,"diskDispatcher");

    return 0;

}

//void diskDispatcher(){
//
//    while(1){
//
//        sem_down(&disk.semDisk);
//
//        if(disk.sinal == 1){
//            disk.sinal = 0;
//            queue_append((queue_t**)&filaProntos, (queue_t*) disk.filaSuspensaDisk);
//            disk.filaSuspensaDisk=NULL;
//        }
//
//
//        if((disk_cmd(DISK_CMD_STATUS,0,0) == 1) && (disk.filaDisk != NULL)){
//
//            disk.filaSuspensaDisk = queue_remove((queue_t**)&disk.filaDisk,(queue_t*)disk.filaDisk);
//            if(disk.filaSuspensaDisk->diskOp == DISKREAD){
//                disk_cmd(DISK_CMD_READ,disk.filaSuspensaDisk->diskBlock, disk.filaSuspensaDisk->diskBuffer);
//            }else if(disk.filaSuspensaDisk->diskOp == DISKWRITE){
//                disk_cmd(DISK_CMD_WRITE,disk.filaSuspensaDisk->diskBlock, disk.filaSuspensaDisk->diskBuffer);
//
//            }
//        }
//        sem_up(&disk.semDisk);
//        task_yield();
//    }
//}

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer){

    atual->diskOp = DISKREAD;
    atual->diskBlock = block;
    atual->diskBuffer = buffer;

    queue_append((queue_t**)&disk.filaDisk, (queue_t*) atual);

    task_switch(&disk.tarefaDisk);

    return 0;
    
}

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer){
    atual->diskOp = DISKWRITE;
    atual->diskBlock = block;
    atual->diskBuffer = buffer;

    queue_append((queue_t**)&disk.filaDisk, (queue_t*) atual);

    task_switch(&disk.tarefaDisk);

    return 0;
}
*/