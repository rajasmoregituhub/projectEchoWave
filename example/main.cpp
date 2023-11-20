#include <iostream>

#include "echoWave.pb.h" // Include the generated header from the proto build of echoWave.proto

int main (void)
{
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    echoWave msg;
    echoWaveRequest request;
    request.set_wave("Example");
    msg.mutable_request()->CopyFrom(request);

    std::cout << msg.DebugString() << std::endl;

    return 0;
}