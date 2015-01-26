#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int
main()
{
 int sock0;
 struct sockaddr_in addr;
 struct sockaddr_in client;
 int len;
 int sock;
 int yes = 1;
 char buf[32];
 char filename[32],output[32];
   int i,n;
   int size;
   char *file;
   FILE *fp;
   char command[128];


 sock0 = socket(AF_INET, SOCK_STREAM, 0);

 addr.sin_family = AF_INET;
 addr.sin_port = htons(12345);
 addr.sin_addr.s_addr = INADDR_ANY;

 setsockopt(sock0,
   SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

 bind(sock0, (struct sockaddr *)&addr, sizeof(addr));

 listen(sock0, 5);

 while (1) {
   len = sizeof(client);
   sock = accept(sock0, (struct sockaddr *)&client, &len);

   memset(filename, 0, sizeof(filename));
    n = read(sock, filename, sizeof(filename));
   if (n < 0) {
    perror("read");
    return 1;
   }
   printf("%d, %s\n", n, buf);
   write(sock,"",1);


   memset(buf, 0, sizeof(buf));
    n = read(sock, buf, sizeof(buf));
   if (n < 0) {
    perror("read");
    return 1;
   }

   size = atoi(buf);
   printf("%d, %d\n", n, size);
   file = calloc(size+32,sizeof(char));
   write(sock,"",1);

   for(i=0;i<size;i+=1024){
      read(sock, &file[i], 1024);
      write(sock,"",1);
   }

   sprintf(output,"/home/root/%s",filename);
   fp = fopen(output,"w");
   for(i=0;i<size;i++)
      putc(file[i],fp);
   fclose(fp);
   sprintf(command,"chmod 777 %s\n",output);
   system(command);
   printf("complete\n");
 
   close(sock);
 }

 close(sock0);

 return 0;
}

