#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

//char *socket_path = "./socket";
char *socket_path = "/tmp/unixSockSendFd";
char *file="/tmp/abcd.txt" ;/*file whose fd is to be sent*/
int sendfd(int sock, int fd);
int recvfd(int s);
char data[]="sahil\0";

int main(int argc, char *argv[]) {
  struct sockaddr_un addr;
  char buf[100];
        buf[0]='\n';
  int fd,rc,confd;

int fd_to_send;
int temp,len;
temp=1;

fd_to_send=open(file,O_TRUNC|O_RDWR|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
if(fd_to_send==-1)
{
perror("file open error");
return -1;
}

  if (argc > 1) socket_path=argv[1];

  if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(-1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  if (*socket_path == '\0') {
    *addr.sun_path = '\0';
    strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
  } else {
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
  }
unlink(socket_path);

if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))==-1){
perror("bind error");
return -1;
}

/*Writing data to file before sending fd*/
len=write(fd_to_send,data,(int)strlen(data));
fsync(fd_to_send);
printf("(len=%d)data written in file(content between ## marks) ##%s##\n",len,data);

listen(fd,1);
for(;;){

confd=accept(fd,NULL,NULL);
if(confd==-1)
{
perror("accept error");
continue;
}
else{
printf("new client connected ... sending fd ... \n");
sendfd(confd,fd_to_send);
close(confd);
}

}
//  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
  //  perror("connect error");
    //exit(-1);
  //}

//len=send(fd,&temp,sizeof(temp),0);
//printf("bytes sent=%d\n",len);
//len=write(fd,buf,1);
//printf("bytes sent=%d\n",len);

//  while( (rc=read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
  //  if (write(fd, buf, rc) != rc) {
    //  if (rc > 0) fprintf(stderr,"partial write");
      //else {
        //perror("write error");
        //exit(-1);
     // }
   // }
  //}

  return 0;
}

int sendfd(int sock, int fd)
{
  struct msghdr hdr;
  struct iovec data;

  char cmsgbuf[CMSG_SPACE(sizeof(int))];

  char dummy = '*';
  data.iov_base = &dummy;
  data.iov_len = sizeof(dummy);

  memset(&hdr, 0, sizeof(hdr));
  hdr.msg_name = NULL;
  hdr.msg_namelen = 0;
  hdr.msg_iov = &data;
  hdr.msg_iovlen = 1;
  hdr.msg_flags = 0;

  hdr.msg_control = cmsgbuf;
  hdr.msg_controllen = CMSG_LEN(sizeof(int));

  struct cmsghdr* cmsg = CMSG_FIRSTHDR(&hdr);
  cmsg->cmsg_len   = CMSG_LEN(sizeof(int));
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type  = SCM_RIGHTS;

  *(int*)CMSG_DATA(cmsg) = fd;
//memcpy((CMSG_DATA(cmsg)), &fd, sizeof(fd)); -- from ivshmem server code - this too works instead of previous line 

  int n = sendmsg(sock, &hdr, 0);

  if(n == -1)
    printf("sendmsg() failed: %s (socket fd = %d)\n", strerror(errno), sock);

  return n;
}


int recvfd(int s)
{
        int n;
        int fd;
        char buf[1];
        struct iovec iov;
        struct msghdr msg;
        struct cmsghdr *cmsg;
        char cms[CMSG_SPACE(sizeof(int))];

        iov.iov_base = buf;
        iov.iov_len = 1;

        memset(&msg, 0, sizeof msg);
        msg.msg_name = 0;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        msg.msg_control = (caddr_t)cms;
        msg.msg_controllen = sizeof cms;

        if((n=recvmsg(s, &msg, 0)) < 0)
                return -1;
        if(n == 0){
                perror("unexpected EOF");
                return -1;
        }
        cmsg = CMSG_FIRSTHDR(&msg);
        memmove(&fd, CMSG_DATA(cmsg), sizeof(int));
        return fd;
}

