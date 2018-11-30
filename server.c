#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_USERS 10

int main(int argc, char* argv[]) {
  int servSock, maxDs, cliSock, listClient[MAX_USERS], sockSize;
  int userCount, iterClient, iRet;
  int arr[100] = {0, }, i, temp;
  fd_set fdSet;
  struct sockaddr_in stAddr, stAccept;
  unsigned char Buf[256];

  char sendPlayer[] = "1";
  qaidx = userCount = 0;
  sockSize = sizeof(struct sockaddr_in);
  bzero(&stAddr, sockSize); 

  if((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    perror("socket() failed");
    close(servSock);
    return -10;
  }
  stAddr.sin_family = AF_INET;  

  if((iRet = inet_pton(AF_INET, "127.0.0.1", &stAddr.sin_addr.s_addr)) <= 0) {
    perror("inet_pton() failded");
    close(servSock);
    return -100;
  }
  stAddr.sin_port = htons(3000);

  if(bind(servSock, (struct sockaddr *)&stAddr, sockSize) == -1) {
    perror("bind() failed");
    close(servSock);
    return -100;
  }
  if(listen(servSock, 5) == -1) {
    perror("listen() failed");
    close(servSock);
    return -100;
  }

  maxDs = servSock + 1;
  while(1) {
    printf("[SYSTEM] 현재 접속자 수는 %d명입니다.\n", userCount);
    for(i = 0; i < userCount;++i) printf("[Player %d] : %d\n", i + 1, arr[i]);
    FD_ZERO(&fdSet); 
    FD_SET(servSock, &fdSet);
    FD_SET(0, &fdSet);
    for(iterClient = 0; userCount > iterClient; ++iterClient) FD_SET(listClient[iterClient], &fdSet);
    if(0 > select(maxDs, &fdSet, NULL, NULL, NULL)) {
      perror("select() error... ");
      close(servSock);
      return -100;
    }
    if(1 == FD_ISSET(0, &fdSet)) {
      iRet = read(0, Buf, sizeof(Buf));
      Buf[iRet] = 0;
      printf("%s\n", Buf);
      for(iterClient = 0; userCount > iterClient; ++iterClient) write(listClient[iterClient], Buf, iRet);
      write(listClient[iterClient], question + qaidx, strlen(*(question + qaidx)));
      continue;
    }
    
    if(1 == FD_ISSET(servSock, &fdSet)) {
      if((cliSock = accept(servSock, (struct sockaddr *)&stAccept, &sockSize)) == -1) {
        perror("accept() error");
        close(servSock); 
        for(iterClient = 0;userCount > iterClient; ++iterClient) close(listClient[iterClient]);
        return -100;
      }

      if(MAX_USERS <= userCount) {
        write(cliSock, "Server is full connection\n", sizeof("Server is full connection\n"));
        close(cliSock);
        continue ;
      }
      
      if(cliSock == maxDs) {
        maxDs = cliSock + 1;
        listClient[userCount++] = cliSock;
      }

      write(listClient[userCount - 1], *(question + qaidx), strlen(*(question + qaidx)));
      printf("Client IP : [%s]\n", inet_ntoa(stAccept.sin_addr));
      continue ;
    }

    for(iterClient = 0; userCount > iterClient; ++iterClient) {
      if(1==FD_ISSET(listClient[iterClient], &fdSet)) {
        iRet = read(listClient[iterClient], Buf, sizeof(Buf));
        Buf[iRet] = 0;
        for(iterClient = 0; userCount > iterClient; ++iterClient) {
          if(iterClient != temp) write(listClient[iterClient], Buf, iRet);
        }
      }
    }
  }
  write(cliSock, "pong", sizeof("pong"));
  close(servSock);
  close(cliSock);
  return 0;
}