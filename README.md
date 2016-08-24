Test project to understand sendding of file descriptors over unix socket, and sniffing the socket with socat.

run sendfd, socat, and then recvfd.

 - ./senfd
 - socat UNIX-LISTEN:/tmp/unixSockSendFe,mode=775,reuseaddr,fork UNIX-CONNECT:/tmp/unixSockSendFd
 - ./recvfd
