#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/wait.h>
#include <signal.h>

#include <sys/ipc.h>
#include <sys/shm.h>

 
#define BUFSIZE 200 

int server_use = 0; //editing client num

void error_handling(char *message); 
void read_childproc(int sig);

void server_use_edit(){
    server_use = 1;
};
int main(int argc, char **argv) {   
    int serv_sd; 
    int clnt_sd; 
    int fd; 
    char buf[BUFSIZE];
    char cbuf[BUFSIZE];

     
    FILE* fp;

    struct sockaddr_in serv_addr; 
    struct sockaddr_in clnt_addr; 
    int clnt_addr_size; 
    int len;

    /*adding*/
    pid_t pid;
    struct sigaction act;
    int str_len, state;
    /*end*/ 
    
    int client_status = 0;     //edit possible(true) or impossible(False)
    int shmid;
    int *cal_num;
    void *shared_memory = (void *)0; //make shared memory
    struct shmid_ds shm_info;
    char message[50]={0};

    shmid = shmget((key_t)1234, sizeof(int),0666|IPC_CREAT);
    if(shmid == -1){
        perror("shmget failed : ");
        exit(0);
    }

    shared_memory = shmat(shmid,(void *)0,0);
    if(shared_memory == (void*)-1)
    {
        perror("shmat failed : ");
        exit(0);        
    }
     
    if(argc!=2){ 
        printf("Usage : %s <port>\n", argv[0]); 
        exit(1); 
    } 

    /*adding (zombie)*/
    act.sa_handler=read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    state=sigaction(SIGCHLD,&act,0);
    /*end*/
     
    serv_sd=socket(PF_INET, SOCK_STREAM, 0);    
    if(serv_sd == -1) 
        error_handling("socket() error"); 
     
    memset(&serv_addr, 0, sizeof(serv_addr)); 
    serv_addr.sin_family=AF_INET; 
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY); 
    serv_addr.sin_port=htons(atoi(argv[1])); 
     
    if( bind(serv_sd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1 ) 
        error_handling("bind() error"); 
     
    if( listen(serv_sd, 5)==-1 ) 
        error_handling("listen() error"); 
    //this point adding while
    while(1)
    {
        clnt_addr_size=sizeof(clnt_addr);     
        clnt_sd=accept(serv_sd, (struct sockaddr*)&clnt_addr,&clnt_addr_size); 
        if(clnt_sd==-1)
            continue; 
            //error_handling("accept() error");
        else    //adding
            puts("new Client connected...");
        pid = fork();        //adding fork
        if(pid == -1)
        {
            close(clnt_sd);
            continue;
        }
        if(pid == 0) //child _adding
        {   
            shmid = shmget((key_t)1234,sizeof(int),0);
            shared_memory = shmat(shmid,(void *)0,0);
            shmctl(shmid,IPC_STAT,&shm_info);
            printf("shared memory use %ld\n",shm_info.shm_nattch);
            
            close(serv_sd); //server close
            /* ¹®Œ­ÀÇ ÀÌž§ À» ŒöœÅ ÇÕ */
            len = read(clnt_sd, cbuf, BUFSIZE); 
            write(1, cbuf, len);       
            puts(cbuf);
            /* Á¢ŒÓÇØ¿ÀŽÂ Å¬¶óÀÌŸðÆ®¿¡°Ô ÀüŒÛÇØÁÙ ÆÄÀÏ ¿ÀÇÂ  */ 
            fd = open("calender.db", O_RDONLY | O_CREAT,0644 );  
            if(fd == -1 ) 
                error_handling("File open error"); 
            if(shm_info.shm_nattch == 3){
                sprintf(message,"You CAN Edit this schedule\n");
                write(clnt_sd,message,strlen(message));
            }
            else{               //can edit so kill
                sprintf(message,"You CAN'T EditThisSchedule\n");
                write(clnt_sd,message,strlen(message));

                while( (len=read(fd, buf, BUFSIZE)) != 0 ) 
                { 
                    write(clnt_sd, buf, len); 
                } 
     
                /* µ¥ÀÌÅÍ ÀüŒÛÈÄ ŒÒÄÏÀÇ ÀÏºÎ(ÀüŒÛ¿µ¿ª)žŠ ŽÝÀœ */ 
                if( shutdown(clnt_sd, SHUT_WR) == -1 ) 
                    error_handling("shutdown error"); 
     
                /* ÀÎ»çÀÇ žÞœÃÁöžŠ ŒöœÅ ÇÔ */ 
                len = read(clnt_sd, cbuf, BUFSIZE); 
                write(1, cbuf, len); 
                close(fd); //fd close because i send the file
                close(clnt_sd);
                puts("client disconnected...");
                return 0;
       
            }
            /* Å¬¶óÀÌŸðÆ®¿¡ µ¥ÀÌÅÍ ÀüŒÛ */ 
            while( (len=read(fd, buf, BUFSIZE)) != 0 ) 
            { 
                write(clnt_sd, buf, len); 
            } 
     
            /* µ¥ÀÌÅÍ ÀüŒÛÈÄ ŒÒÄÏÀÇ ÀÏºÎ(ÀüŒÛ¿µ¿ª)žŠ ŽÝÀœ */ 
            if( shutdown(clnt_sd, SHUT_WR) == -1 ) 
                error_handling("shutdown error"); 
     
            /* ÀÎ»çÀÇ žÞœÃÁöžŠ ŒöœÅ ÇÔ */ 
            len = read(clnt_sd, cbuf, BUFSIZE); 
            write(1, cbuf, len); 
            close(fd); //fd close because i send the file
            
            len = read(clnt_sd, cbuf, 2); 
            write(1, cbuf, len);       
            puts(cbuf);

            fd = open("calender.db",O_WRONLY|O_CREAT|O_TRUNC,0644);
            while((len = read(clnt_sd,cbuf,BUFSIZE))!=0)
            {
                write(fd,cbuf,len);
            }
            puts("file synchronized");       
            close(fd); 
            close(clnt_sd);
            puts("client disconnected...");
            return 0;
        }   //if adding
        else    //else adding
            close(clnt_sd);
    }
//while this point  
    return 0; 
}

void read_childproc(int sig){
    pid_t pid;
    int status;
    pid=waitpid(-1, &status,WNOHANG);
    printf("removed proc id: %d \n",pid);
} 
 
void error_handling(char *message) 
{ 
    fputs(message, stderr); 
    fputc('\n', stderr); 
    exit(1); 
}

