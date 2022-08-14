#ifndef FINGERPRINT_SOCKET_H
#define FINGERPRINT_SOCKET_H

class FingerprintSocket
{
private:
    /* data */
public:
    FingerprintSocket(/* args */);
    ~FingerprintSocket();
    int createSocket();
    void sendCmdToSocket();
private:
    void handleMessage(void *data, unsigned int len);
};
#endif