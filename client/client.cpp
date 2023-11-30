// Client side program to demonstrate TCP Socket
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include "echoWave.pb.h" // Include the generated header from the proto build of echoWave.proto
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#define PORT 8080

using namespace google::protobuf::io;
using namespace std;

echoWave msg;
echoWaveRequest request;
echoWaveResponse response;


google::protobuf::uint32 readHdr(char *buf)
{
  google::protobuf::uint32 size;
  google::protobuf::io::ArrayInputStream ais(buf,4);
  CodedInputStream coded_input(&ais);
  coded_input.ReadVarint32(&size);//Decode the HDR and get the size
  return size;
}
 
int main(int argc, char const* argv[])
{
    int status, client_fd;
    struct sockaddr_in serv_addr;
    int bytecount;
    char buffer[1024] = { 0 };
    
    request.set_wave("Hello world from client_request");
    msg.mutable_request()->CopyFrom(request);

    int siz = request.ByteSize()+4;
    char *pkt = new char [siz];
    google::protobuf::io::ArrayOutputStream aos(pkt,siz);
    CodedOutputStream *coded_output = new CodedOutputStream(&aos);
    coded_output->WriteVarint32(request.ByteSize());
    request.SerializeToCodedStream(coded_output);

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    // Convert IPv4 and IPv6 addresses from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if ((status
         = connect(client_fd, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    if( (bytecount=send(client_fd, (void *) pkt,siz,0))== -1 ) {
            fprintf(stderr, "Error sending data %d\n", errno);
            goto FINISH;
    }
    //usleep(1);
    delete pkt;
    
    if((bytecount = recv(client_fd,
                        (void *)buffer,
                                4+siz, 0))== -1){
            fprintf(stderr, "Error receiving data %d\n", errno);
    }
    if(bytecount != 0)
    {
        google::protobuf::uint32 len;
        len = readHdr(buffer);
        cout<<"Received response from sever "<<endl;
        google::protobuf::io::ArrayInputStream ais(buffer,len+4);
        CodedInputStream coded_input(&ais);
        //Read an unsigned integer with Varint encoding, truncating to 32 bits.
        coded_input.ReadVarint32(&len);
        google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(len);
        //De-Serialize
        response.ParseFromCodedStream(&coded_input);
        cout<<"Message from server is "<<response.DebugString();
    }
    
    // closing the connected socket
    FINISH:
        close(client_fd);
    return 0;
}
