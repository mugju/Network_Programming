#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include<stdio_ext.h>


#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)

#define BUFF 30
   
#define BUFSIZE 400 

void error_handling(char *message); 

void view_calender(int year, int month) {
	int num[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int i, j, week, th, count, lastday, tyear, tmonth;

	tyear = year;
	tmonth = month;

	lastday = num[tmonth - 1];
	if (tmonth == 2 && (!(tyear % 4) && (tyear % 100) || !(tyear % 400))) {
		lastday = 29;
	}

	if (tmonth == 2 || tmonth == 1) {
		tyear--;
		tmonth += 12;
	}

	th = tyear / 100;
	tyear = tyear % 100;

	week = ((21 * th / 4) + (5 * tyear / 4) + (26 * (tmonth + 1)) / 10) % 7;
	printf("\n 일 월 화 수 목 금 토 \n");
	count = 1;
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 7; j++) {
			if (i == 0 && j < week)
				printf("%3s", " ");
			else {
				printf("%3d", count);
				count++;
			}
			if (count > lastday)
				break;
		}
		printf("\n");

		if (count > lastday)
			break;
	}
}
void set_plan_date(int* year, int* month, int* day) {
	char plan_date[20] = { 0 };
	char temp[20];
	printf("INPUT DATA.  \n");
	__fpurge(stdin);
	printf("YEAR: ");               //보고싶은 년도
	fgets(temp, BUFF, stdin);
	sscanf(temp, "%d", year);


	printf("MONTH: ");               //결정할 달
	fgets(temp, BUFF, stdin);
	sscanf(temp, "%d", month);


	view_calender(*year, *month);         //일정 저장할 때 달력보고 저장할 수 있게..

	printf("DAY: ");               //결정할 달
	fgets(temp, BUFF, stdin);
	sscanf(temp, "%d", day);         //선택 날짜

}
 

int main(int argc, char **argv) 
  { 
      	int fd; 
      	int sd; 
       
      
      	char buf[BUFSIZE];
      	char cbuf[BUFSIZE]; 
      	int len; 
	struct sockaddr_in serv_addr; 


/////////////////////////////////////////////
	FILE* fp;

	int i, j, year, month, day, pos;
	int mode;

	char calender[40]; // 일정저장

	char plan_date[3][20] = { 0 }; //3일치

	char plan[6][40] = { 0 }; //하루에 저장가능한 일정의 수는 5개입니다. 하나는 일정의 끝인 널문자를 넣기 위함입니다.
	char plan_temp[300] = { 0 };
	char* Plan_data_temp[3] = { NULL };

	//파일 읽어올때 쓰는 변수들
	char* find[3] = { NULL };
	int line_num = 0;
	char buffer[BUFSIZE]; //파일에서 읽어올때 저장할 버퍼
//////////////////////////////////////////////////////////////////////




      if(argc!=3){ 
          printf("Usage : %s <IP> <port>\n", argv[0]); 
          exit(1); 
      } 
        
      // 수신 한 데이터를 저장 할 파일 오픈 
      fd = open("calender.db", O_WRONLY|O_CREAT|O_TRUNC,0644); 
      if(fd == -1) 
          error_handling("File open error"); 
       
      // 서버 접속을 위한 소켓 생성  
      sd=socket(PF_INET, SOCK_STREAM, 0);    
      if(sd == -1) 
           error_handling("socket() error"); 
      
      memset(&serv_addr, 0, sizeof(serv_addr)); 
      serv_addr.sin_family=AF_INET; 
      serv_addr.sin_addr.s_addr=inet_addr(argv[1]); 
      serv_addr.sin_port=htons(atoi(argv[2])); 
 
      if( connect(sd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1 ) 
          error_handling("connect() error!"); 

      // 데이터 수신하기위한 확인 
      printf("if you want recieve data, press any key..\n");
      __fpurge(stdin);
      fgets(cbuf, 15, stdin);
      
      write(sd, cbuf, 15);    
     
      //에딧권한 알려주는 메시지
      char message[20] = {0};
      int nu;
      nu = read(sd, message, 26);
      fputs(message,stdout);
      sleep(1);
      // 데이터를 전송 받아서 파일에 저장한다 
      while( (len=read(sd, buf, BUFSIZE )) != 0 ) 
     { 
         write(fd, buf, len);  
     } 
      
     /* 전송해 준것에 대한 감사의 메시지 전달 */ 
     write(sd, "calender_recieved!\n", 20);    
     close(fd);           
     //close(sd); 		소켓 아직 닫으면 안돼
     


	while (1) {
		fseek(stdin, 0, SEEK_END);
		printf("\n\n==========================<Main Menu>===================================\n\n");
		printf("This program support schedule's save, and lookup data \n Number1 is Input schedule.\n Number 2 is lookup schedule.\n Number 0 is force quit program.\n");
		printf("Number 4 is synchronization. \n Your schedule will be synchronized whit server DB and SHUTDOWN program\n");
		printf("\n\n========================================================================\n\n");
		__fpurge(stdin);
		scanf("%d", &mode);
		fseek(stdin, 0, SEEK_END);
		
		
		switch (mode)      //1은 일정저장    2는 일정조회   3은 일정 수정
		{
		
		case 1:
			/*일정 저장하는 로직*/
			fp = fopen("calender.db", "a");      //일정 저장용  db
									   			//a 옵션을 주어 일정 추가로직을 수행하게 함.
			set_plan_date(&year, &month, &day);   //year, month, day셋팅
			sprintf(plan_date[0], "%d-%d-%2d", year, month, day);

			puts(plan_date[0]);
			//넣기전에 배열 초기화
			for(i=0;i<5;i++){
				for(j=0;j<40;j++){
				plan[i][j] = 0; //수정함
				}
			}
			for (i = 0; i < 5; i++) {
				__fpurge(stdin);
				printf("input your schedule ,You can input 5 data. (if you want stop just press enter!)\n");
				fgets(calender,BUFF,stdin);
				calender[strlen(calender)-1] = '\0';
				if (strlen(calender) == 0||strlen(calender)<3) {
					printf("STOP_input logic.\n");
					break;
				}
				
				strcpy(plan[i], calender);
			}
			printf("5개를 다 썼습니다.\n");
			for (i = 0; i < 5; i++) {
				pos = strlen(plan[i]);
				if (pos != 0)
					printf("Print schedule : %s\n", plan[i]);
				else {
					printf("\n---end of schedule---\n");
					break;
				}
			}
			sprintf(plan_temp, "%s\t%s\t%s\t%s\t%s\t%s\n", plan_date[0], plan[0], plan[1], plan[2], plan[3], plan[4]);      //이부분은 동적할당으로 처리하던가.. 해야됨.
			puts(plan_temp);
			fputs(plan_temp, fp);
			

			fclose(fp);

			/*여기까지 일정 입력로직*/
			break;


		case 2:

			/*일정 조회로직*/
			fp = fopen("calender.db", "r");      //읽기전용으로 파일 열기

			set_plan_date(&year, &month, &day);   //year, month, day셋팅

			sprintf(plan_date[0], "%d-%d-%2d", year, month, day);   //찾을 날짜 여기서 day는 int형이므로 조작가능
			sprintf(plan_date[1], "%d-%d-%2d", year, month, day + 1);   //찾을 날짜 여기서 day는 int형이므로 조작가능
			sprintf(plan_date[2], "%d-%d-%2d", year, month, day + 2);   //찾을 날짜 여기서 day는 int형이므로 조작가능
			printf("%s , %s, %s DATA_finding...\n", plan_date[0], plan_date[1], plan_date[2]);
			for (i = 0; i < 3; i++) {
				line_num = 0;
				fseek(fp, 0, 0);
				while (fgets(buffer, 200, fp)) {
					line_num++;
					find[i] = strstr(buffer, plan_date[i]);
					if (find[i] != NULL) {            //문자열 찾아주는 함수
						printf("IN %d line %s schedule find!\n", line_num, plan_date[i]);

						Plan_data_temp[i] = find[i]; //오류떠서 ㄴ용 복사해줌
						for (j = 0; Plan_data_temp[i][j] != 0; j++) {
							if (Plan_data_temp[i][j] == '\t') {
								Plan_data_temp[i][j] = '\n';
							}
						}
						puts(Plan_data_temp[i]);

					}
					//puts(find[0]); //해당날짜 일정 출

				}
				if (Plan_data_temp[i] == NULL) { printf("%s일정을 찾을 수 없습니다 \n", plan_date[i]); }
				//puts(Plan_data_temp[i]);
				
			}
			fclose(fp);
			break;

		case 3:         //수정로직
			fp = fopen("calender.db", "a+");      //일정 수정용  db
									   //a 옵션을 주어 일정 수정로직을 수행하게 함.
			line_num = 0;

			char* date_temp;   //날짜로 쓰기위한 임시변수 --> 기존 날짜를 그대로 받아온거
			char* plan_t[7];   //일정뒤에 개행문자 들어감 (1개) 마지막 널문자 (1개)
			char* Line_temp = NULL;
			set_plan_date(&year, &month, &day);   //year, month, day셋팅
			sprintf(plan_date[0], "%d-%d-%d", year, month, day);

			//수정은 해당 날짜의 일정을 지우고 새로 추가함.

			//char* find[3] = { NULL };

			while (fgets(buffer, 200, fp)) {
				line_num++;
				find[0] = strstr(buffer, plan_date[0]);
				if (find[0] != NULL) {            //문자열 찾아주는 함수
					printf("In %d Iine Find %s schedule!\n", line_num, plan_date[0]);

					Line_temp = find[0]; //오류떠서 ㄴ용 복사해줌
					strtok(Line_temp, "\t");
					for (j = 0; plan_t[j - 1] != NULL; j++) {
						plan_t[j] = strtok(NULL, "\t");
						if (plan_t[j] != NULL)
							printf("%s", plan_t[j]);
					}
				}
				//puts(find[0]); //해당날짜 일정 출력



			}
			if (Line_temp == NULL) { printf("%s일정을 찾을 수 없습니다 \n", plan_date[0]); }
			fclose(fp);
		

		case 4:			//파일을 다시 서버로 보내서 동기화를 시켜줌
			 printf("This is Synchronization logic\n");
			 printf("if you want synchronize data whit server, press any key..\n if you not, press ctrl + c");
     			 __fpurge(stdin);
      			fgets(cbuf, 2, stdin);
      
     	 		write(sd, cbuf, 2);
			    
			fd = open("calender.db", O_RDONLY);
			if(fd == -1)
				error_handling("File open Error");
			
			while( (len = read(fd,buf,BUFSIZE) ) != 0)
			{
				write(sd,buf,len);
			}

			shutdown(sd, SHUT_WR); //동기화 끝냈음 난 다 보냈음. EOF send
			printf("synchronization complete!\n");
			printf("program shutdown\n");
			close(sd); //드디어 소켓 닫아줌
			return 0;

		case 0:
			return 0;

		default:
			printf("\n mode is wrong_input! \n You must input 1 or 2 or 4 or 0 \n\n");
			continue;
			
		}

		//fclose(fp);

	}

	return 0;
	//일단 수정은 봉인? 

 } 



  
void error_handling(char *message) 
 { 
      fputs(message, stderr); 
      fputc('\n', stderr); 
      exit(1); 
}
