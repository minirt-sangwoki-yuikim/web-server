#include "SendEventBean.hpp"

#include <sys/socket.h>

SendEventBean::SendEventBean() {}
SendEventBean::~SendEventBean() {}

#define BUF_SIZE 1024 * 1024
int SendEventBean::runBeanEvent(HttpHandler *httpHandler, Event *event) {
  ServerConfiguration *serverConfig;
  int socketfd;
  int ret;
  int buf_size;

  serverConfig = httpHandler->getServerConfiguration();
  socketfd = httpHandler->getFd();
  buf_size = BUF_SIZE;
  if (httpHandler->getDataLength() - httpHandler->getBufferIdx() < BUF_SIZE)
    buf_size = httpHandler->getDataLength() - httpHandler->getBufferIdx();
  ret = write(socketfd, httpHandler->getBufferStartIdx() + httpHandler->getBufferIdx(), buf_size);
  std::cout << "SEND BEFORE IDX: " << httpHandler->getBufferIdx() << " + " << ret << "\n";
  httpHandler->setBufferIdx(httpHandler->getBufferIdx() + ret);
  std::cout << "SEND MOVED IDX: " << httpHandler->getBufferIdx() << "\n";
  // std::cout << "SEND [" << std::string(httpHandler->getBufferStartIdx(), httpHandler->getBufferIdx()) << "]\n";
  if (ret < 0)
  {
    std::cout << "ERROR\n";
    event->saveEvent(socketfd, EVFILT_WRITE, EV_DISABLE, 0, 0, 0);  // EVFILT_WRITE
    delete httpHandler;
    httpHandler = 0;
    return (ret);
  }
  else if (httpHandler->getBufferIdx() == httpHandler->getDataLength() || ret == 0) {
    std::cout << "SEND FD: " << socketfd << ", RET: " << ret << "\n";
    int waitSec;
    
    HttpHandler *recvHandler = new HttpHandler(socketfd, serverConfig);
    waitSec = httpHandler->getDataLength() / 10000000 + 3; // default 5sec + 100MB 기준 1초
    if (waitSec >= 15)
      waitSec = 15;
    std::cout << "waitSec: " << waitSec << "\n";
    event->saveEvent(socketfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, recvHandler);
    event->saveEvent(socketfd, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, waitSec, recvHandler);
    event->saveEvent(socketfd, EVFILT_WRITE, EV_DISABLE, 0, 0, 0);  // EVFILT_WRITE 
    delete httpHandler;
    httpHandler = 0;
    // close(socketfd);
    std::cout << "================== SEND DONE ============================\n";
    return (0);
  }
  return (ret);
}