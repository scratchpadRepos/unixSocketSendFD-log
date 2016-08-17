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
char *socket_path = "/tmp/unixSockSendFe";
int sendfd(int sock, int fd);
int recvfd(int s);
int fd_received;

int main(int argc, char *argv[]) {
  struct sockaddr_un addr;
  char buf[100];
        buf[0]='\n';
  int fd,rc,confd;

int temp,len;
temp=1;

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

if(connect(fd,(struct sockaddr*)&addr,sizeof(addr))==-1)
{
perror("connect error");
exit(-1);
}

fd_received=recvfd(fd);
lseek(fd_received,0,SEEK_SET);
len=read(fd_received,buf,5);
if(len<0)
{
perror("read error");
}
printf("(fd_received=%d,len=%d) first %d characters read from the file whoes fd was received(content within ##) ##%.*s##\n",fd_received,len,5,5,buf);

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

