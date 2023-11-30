// Server side program to demonstrate TCP Socket 
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <resolv.h>
#include <arpa/inet.h>
#include "echoWave.pb.h" // Include the generated header from the proto build of echoWave.proto
#include <iostream>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#define PORT 8080

using namespace std;
using namespace google::protobuf::io;

void* SocketHandler(void*);

int main(int argc, char const* argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addr_size = 0;
    int* csock;
    pthread_t thread_id=0;
  
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    // Try attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    // Try attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
     

    addr_size = sizeof(sockaddr_in);

    while(true){
      printf("waiting for a connection\n");
      csock = (int*)malloc(sizeof(int));
      if((*csock = accept( server_fd, (sockaddr*)&address, &addr_size))!= -1){
              printf("---------------------\nReceived connection from %s\n",inet_ntoa(address.sin_addr));
              pthread_create(&thread_id,0,&SocketHandler, (void*)csock );
              pthread_detach(thread_id);
      }
      else{
              fprintf(stderr, "Error accepting %d\n", errno);
      }
    }

FINISH:
;
}

google::protobuf::uint32 readHdr(char *buf)
{
  google::protobuf::uint32 size;
  google::protobuf::io::ArrayInputStream ais(buf,4);
  CodedInputStream coded_input(&ais);
  coded_input.ReadVarint32(&size);//Decode the HDR and get the size
  //cout<<"size of msg is "<<size<<endl;
  return size;
}

void sendresponse(int csock, echoWaveResponse response)
{

  cout<<"send response to client "<<endl;
  int len = response.ByteSize()+4;
  char *pkt = new char [len];
  
  google::protobuf::io::ArrayOutputStream aos(pkt,len);
  CodedOutputStream *coded_output = new CodedOutputStream(&aos);
  coded_output->WriteVarint32(response.ByteSize());
  response.SerializeToCodedStream(coded_output);
  send(csock, (void *) pkt,len,0);  //send response to client

  delete pkt;

}
void readBody(int csock,google::protobuf::uint32 siz)
{
  int bytecount;
  char buffer [siz+4];
    
  echoWave msg;
  echoWaveResponse response;
  echoWaveRequest request;

  //Read the entire buffer including the hdr
  if((bytecount = recv(csock, (void *)buffer, 4+siz, MSG_WAITALL))== -1){
                fprintf(stderr, "Error receiving data %d\n", errno);
        }
  google::protobuf::io::ArrayInputStream ais(buffer,siz+4);
  CodedInputStream coded_input(&ais);
  //Read an unsigned integer with Varint encoding, truncating to 32 bits.
  coded_input.ReadVarint32(&siz);
  google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(siz);
  request.ParseFromCodedStream(&coded_input);
  coded_input.PopLimit(msgLimit);

  cout<<"Message from client is "<<request.DebugString();
  /*create Response for client*/

  
  if (bytecount != 0)
  {
    response.set_status(1);
    response.set_echo(request.wave());
   
  }
  else
  {
    response.set_status(0);
    response.set_echo("Client sent error");
  }

  sendresponse(csock, response);

}

void* SocketHandler(void* lp){
    int *csock = (int*)lp;

        char buffer[4];
        int bytecount=0;
          
        memset(buffer, '\0', 4);

        while (1) {
          if((bytecount = recv(*csock,
                         buffer,
                                 4, MSG_PEEK))== -1){
                fprintf(stderr, "Error receiving data from client %d\n", errno);
          }else if (bytecount == 0)
                break;
          readBody(*csock,readHdr(buffer));
        }
    FINISH:
        free(csock);
    return 0;

}