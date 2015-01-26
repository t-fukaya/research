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

// 端末にてtftp ipaddress filename を入力すると
// argv[0] argv[1] argv[2] 内に入力される 
// なお、argv自体はargcに格納されている？
 if (argc != 3) {
    printf("Usage : %s dest\n", argv[0]);
    return 1;
 }

//argv[2]の文字列の長さを図る
 namelen = strlen(argv[2]);
   fp = fopen(argv[2],"r");

//ファイル位置表示子をファイルの終端から０バイト移動
   fseek(fp, 0L, SEEK_END);

//ファイル位置表示子をftellで求める.バイト数を求められる？
   filesize = ftell(fp);

//ファイル位置表示子をファイルの先頭から０バイト移動
   fseek(fp, 0L, SEEK_SET);

//mallocで必要分,配列を作る
   file = malloc(sizeof(char)*filesize);
   for(i=0;i<filesize;i++)
      file[i] = fgetc(fp);
   fclose(fp);
   sprintf(sizec,"%d\0",filesize);
   
 deststr = argv[1];

//ソケットの作成
 sock = socket(AF_INET, SOCK_STREAM, 0);
 if (sock < 0) {
    perror("socket failed");
    return 1;
 }

//接続先指定用構造体の準備
 server.sin_family = AF_INET;
 server.sin_port = htons(12345);

//IPアドレスを表現した文字列をバイナリ値(32bit)に変換する
 server.sin_addr.s_addr = inet_addr(deststr);
 if (server.sin_addr.s_addr == 0xffffffff) {
    struct hostent *host;

//deststrからIPアドレスの変換を行う 
//gethostbynameがエラーの際はh_errnoがエラーの
//値を保持する
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
//アドレス関係
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

   write(sock, argv[2], namelen);
      read(sock,dummy,1);
   write(sock, sizec, strlen(sizec));
      read(sock,dummy,1);
   for(i=0;i<filesize;i+=1024){
      write(sock, &file[i], 1024);
      read(sock,dummy,1);
   }
free(file);
 close(sock);

 return 0;
}

