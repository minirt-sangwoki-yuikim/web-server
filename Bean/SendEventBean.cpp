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
  ret = write(socketfd,
              httpHandler->getBufferStartIdx() + httpHandler->getBufferIdx(),
              buf_size);
  // std::cout << "SEND BEFORE IDX: " << httpHandler->getBufferIdx() << " + "
  //           << ret << "\n";
  httpHandler->setBufferIdx(httpHandler->getBufferIdx() + ret);
  // std::cout << "SEND MOVED IDX: " << httpHandler->getBufferIdx() << "\n";
  if (ret < 0) {
    ;
    // std::cout << "ERROR\n";
    // event->saveEvent(socketfd, EVFILT_WRITE, EV_DISABLE, 0, 0,
    //                  0);  // EVFILT_WRITE
    // close(socketfd);
    delete httpHandler;
    return (ret);
  } else if (httpHandler->getBufferIdx() == httpHandler->getDataLength()) {
    // event->saveEvent(socketfd, EVFILT_READ, EV_ENABLE, 0, 0,
    //                  new HttpHandler(socketfd, serverConfig));
    // close(socketfd);
    delete httpHandler;
    // std::cout << "================== SEND DONE
    // ============================\n";
    return (0);
  }
  return (ret);
}