#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include<time.h>
#include "semaphore.h"
/*
Xavier Vaughn
@02928026
*/
sem_t bankAccount;

void  ChildProcess(int SharedMem[]) {
  int account;
  int rNum;
  srand(time(NULL));
  
  for(int i = 0; i < 25;i++) {
    sleep(rand() % 6);
    account = SharedMem[0];
    
    while (SharedMem[1] != 1);

    rNum = rand() % 51;
    printf("Poor Student needs $%d\n", rNum);
    
    if (rNum <= account) {
      account -= rNum;
      printf("Poor Student: Withdraws $%d / Balance = $%d\n", SharedMem[0], account);
      SharedMem[0] = account;
    }else {
      printf("Poor Student: Not Enough Cash ($%d) \n", account);
    }
    SharedMem[1] = 0;
  }
}

void ParentProcess(int SharedMem[]) {
  int account;
  int rNum;
  srand(time(NULL)); 
  for(int i = 0; i < 25;i++){
    sleep(rand() % 6);
    printf("Dear Old Dad: Attempting to check Balance\n");
    account = SharedMem[0];
    
    while(SharedMem[1] != 0);

    if (account < 100) { 
      rNum = rand() % 101; 
      if (rNum % 2 == 0) { 
        account += rNum;
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", SharedMem[0], account);
        }else {
        printf("Dear old Dad: Doesn't have any money to give\n");
      }
      SharedMem[0] = account; 
      SharedMem[1] = 1; 
    } 
    else  {
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
      SharedMem[0] = account; 
      SharedMem[1] = 1; 
    }
  }
}

int  main(int  argc, char *argv[])
{
     int    ShmID;
     int    *ShmPTR;
     pid_t  pid;
     int    status;

     ShmID = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Server has received a shared memory of four integers...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     printf("Server has attached the shared memory...\n");
  
     ShmPTR[0] = 0; // bank accountID
     ShmPTR[1] = 0; // turn
    
     //printf("Main has filled bank accountID = %d turn = %d in shared memory...\n",
     //      ShmPTR[0], ShmPTR[1], ShmPTR[2], ShmPTR[3]);

     printf("Main is about to fork a child process...\n");
     sem_init(&bankAccount, 1, 500);
     pid = fork();
     if (pid < 0) {
          printf("*** fork error (Main) ***\n");
          exit(1);
     }
     else if (pid == 0) { 
          ChildProcess(ShmPTR);
          exit(0);
     }
     else {
          ParentProcess(ShmPTR);
     }
     wait(&status);
     printf("Main has detected the completion of its child...\n");
     shmdt((void *) ShmPTR);
     printf("Main has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Main has removed its shared memory...\n");
     printf("Main exits...\n");
     sem_destroy(&bankAccount);
     exit(0);
}
