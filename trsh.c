#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int
main(int argc, char *argv[])
{
 struct sockaddr_in server;
 int sock;
 char buf[32];
 char *deststr;
 unsigned int **addrptr;

 int namelen,filesize;
   char *file,sizec[32],dummy[1];
  FILE *fp; 
   int i;
  char  command_length[8]; 
   int n,size;


 if (argc < 3) {
    printf("Usage : %s dest\n", argv[0]);
    return 1;
 }
   
 deststr = argv[1];

 sock = socket(AF_INET, SOCK_STREAM, 0);
 if (sock < 0) {
    perror("socket");
    return 1;
 }

 server.sin_family = AF_INET;
 server.sin_port = htons(12346);

 server.sin_addr.s_addr = inet_addr(deststr);
 if (server.sin_addr.s_addr == 0xffffffff) {
    struct hostent *host;

    host = gethostbyname(deststr);
    if (host == NULL) {
       if (h_errno == HOST_NOT_FOUND) {
          /* h_errnoはexternで宣言されています */
          printf("host not found : %s\n", deststr);
       } else {
         /*
         HOST_NOT_FOUNDだけ特別扱いする必要はないですが、
         とりあえず例として分けてみました
         */
         printf("%s : %s\n", hstrerror(h_errno), deststr);
       }
       return 1;
    }

    addrptr = (unsigned int **)host->h_addr_list;

    while (*addrptr != NULL) {
       server.sin_addr.s_addr = *(*addrptr);

       /* connect()が成功したらloopを抜けます */
       if (connect(sock,
            (struct sockaddr *)&server,
            sizeof(server)) == 0) {
         break;
       }

       addrptr++;
       /* connectが失敗したら次のアドレスで試します */
    }

    /* connectが全て失敗した場合 */
    if (*addrptr == NULL) {
       perror("connect");
       return 1;
    }
 } else {
    /* inet_addr()が成功したとき */

    /* connectが失敗したらエラーを表示して終了 */
    if (connect(sock,
                     (struct sockaddr *)&server, sizeof(server)) != 0) {
       perror("connect");
       return 1;
    }
 }

   sprintf(command_length,"%d\0",argc);
   write(sock, &command_length, sizeof(command_length));
   read(sock,dummy,1);

   for(i=2;i<argc;i++){
      write(sock, argv[i], 1024);
      read(sock,dummy,1);
   }

   memset(buf, 0, sizeof(buf));
    n = read(sock, buf, sizeof(buf));
   if (n < 0) {
    perror("read");
    return 1;
   }

   size = atoi(buf);
   file = calloc(size+1024,sizeof(char));
   write(sock,"",1);

   for(i=0;i<size;i+=1024){
      read(sock, &file[i], 1024);
      write(sock,"",1);
   }
   
   fp = fopen("/tmp/trshl","w");
   for(i=0;i<size;i++)
      putc(file[i],fp);
   fclose(fp);
   system("cat /tmp/trshl");

   close(sock);

   return 0;
}

