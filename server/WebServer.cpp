#include "WebServer.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <vector>

#include "StringUtils.hpp"

WebServer& WebServer::getInstance(
    std::map<int, ServerConfiguration*> serverConfig) {
  static WebServer instance(serverConfig);
  return instance;
}

WebServer::WebServer(std::map<int, ServerConfiguration*> serverConfigs) {
  this->serverConfigs = serverConfigs;
}

void WebServer::segSignalHandler(int signo) {
  static_cast<void>(signo);
  std::cout << "[Error] Segmentation Fault is detected" << std::endl;
  std::cout << "[Error] your configuration is wrong" << std::endl;
  exit(1);
}

void WebServer::init() {
  std::map<int, ServerConfiguration*>::iterator it = serverConfigs.begin();
  signal(SIGSEGV, WebServer::segSignalHandler);
  int port = 8080;
  if (eventHandler.initKqueue()) {
    std::cout << "kqueue() error" << std::endl;
    exit(1);
  }
  while (it != serverConfigs.end()) {
    ServerConfiguration* serverConfig = it->second;
    int serversSocket = openPort(serverConfig);
    fcntl(serversSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
    serverSocketPortMap[serversSocket] = it->first;
    eventHandler.registerServerEvent(serversSocket, serverConfig);
    it++;
  }
}

int WebServer::openPort(ServerConfiguration* serverConfig) {
  struct addrinfo* info;
  struct addrinfo hint;
  struct sockaddr_in socketaddr;
  int opt = 1;
  int port = serverConfig->getPort();
  std::string serverName = serverConfig->getServerName();

  std::cout << "[INFO] " << serverName << " "
            << "Port number : " << port << std::endl;

  memset(&hint, 0, sizeof(struct addrinfo));
  memset(&socketaddr, 0, sizeof(struct sockaddr_in));
  socketaddr.sin_family = AF_INET;
  socketaddr.sin_port = htons(port);
  socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  hint.ai_family = AF_INET;
  hint.ai_socktype = SOCK_STREAM;
  std::string portStr = StringUtils::toString(port);
  int res = getaddrinfo(serverName.c_str(), portStr.c_str(), &hint, &info);
  if (res == -1)
    SocketUtils::exitWithPerror("[Error] getaddrinfo() error\n" +
                                std::string(strerror(errno)));
  int serverSocket =
      socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  if (serverSocket == -1)
    SocketUtils::exitWithPerror("[Error] socket() error\n" +
                                std::string(strerror(errno)));
  setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  res = bind(serverSocket, reinterpret_cast<struct sockaddr*>(&socketaddr),
             sizeof(socketaddr));
  if (res == -1) {
    SocketUtils::exitWithPerror("[Error] bind() error\n" +
                                std::string(strerror(errno)));
  }
  res = listen(serverSocket, LISTENCAPACITY);
  if (res == -1) {
    SocketUtils::exitWithPerror("[Error] bind() error\n" +
                                std::string(strerror(errno)));
  }
  return serverSocket;
}

void WebServer::execute() {
  init();
  handleEvent();
}

void WebServer::handleEvent() {
  int newEventCount;
  while (true) {
    newEventCount = eventHandler.create();
    if (newEventCount == -1) {
      SocketUtils::exitWithPerror("[ERROR] kevent() error\n" +
                                  std::string(strerror(errno)));
    }
    eventHandler.clearChangedEventList();
    for (int i = 0; i < newEventCount; i++) {
      processEvent(eventHandler[i]);
    }
    clearClients();
  }
}

void WebServer::processEvent(struct kevent& currEvent) {
  if (currEvent.flags & EV_ERROR) {
    processErrorEvent(currEvent);
    return;
  }
  switch (currEvent.filter) {
    case EVFILT_READ:
      processReadEvent(currEvent);
      break;
    case EVFILT_WRITE:
      processWriteEvent(currEvent);
      break;
    case EVFILT_TIMER:
      processTimerEvent(currEvent);
      break;
  }
}

void WebServer::processErrorEvent(struct kevent& currEvent) {
  if (hasServerFd(currEvent)) {
    disconnectPort(currEvent);
    std::cout << currEvent.ident << "[INFO] server disconnected" << std::endl;
  } else {
    addCandidatesForDisconnection(currEvent.ident);
    std::cout << currEvent.ident << "[INFO] client disconnected" << std::endl;
  }
}

void WebServer::processReadEvent(struct kevent& currEvent) {
  // TODO: 고치기
  std::cout << "currEvent: " << currEvent << std::endl;
  if (hasServerFd(currEvent)) {
    acceptClient(currEvent.ident);
  } else if (isClient(currEvent.ident)) {
    if (currEvent.flags & EV_EOF) {
      addCandidatesForDisconnection(currEvent.ident);
    }
    Handler* handler = reinterpret_cast<Handler*>(currEvent.udata);
    int status = handler->readRequest();
    std::cout << "status " << status << std::endl;
    if (status == -1) {
      std::cout << "statust: " << currEvent << std::endl;
      handler->removeBuffer(currEvent.ident);
      handler->removeAndDeleteChunkedRequest(currEvent.ident);
      handler->errorHandling(
          "400");  // 수정 필요하다. try catch  구문으로 해야할 듯
      addCandidatesForDisconnection(currEvent.ident);
      // handler 관련해서 처리할 부분 처리
      // error response write event 등록. - 하는 게 맞을까?
      // error Handling 분리할 필요가 있음
      // Handler 상태를 설정해주면 write 작업 무리 없이 진행할 수도 있다.
    } else {
      int status = handler->createHttpRequest();
      // CHUNKED 관련 이벤트 처리
    }
    // TODO: 병합 필요 - request 쪽 읽어들일 필요 있음
    // read 작업 분리
    // request http message 파싱 작업 필요
  } else {
    // CGI process 어떻게 돌아가는지 읽을 필요가 있다.
    // dynamic
    // 암튼 두개의 분기점으로 나뉨
  }
}

void WebServer::processWriteEvent(struct kevent& currEvent) {
  if (isClient(currEvent.ident)) {
    Handler* handler = reinterpret_cast<Handler*>(currEvent.udata);
    // TODO: 병합 필요
  } else {
    // CGI
  }
}

void WebServer::processTimerEvent(struct kevent& currEvent) {
  addCandidatesForDisconnection(currEvent.ident);
}

int WebServer::acceptClient(int serverSocket) {
  struct _linger linger;
  linger.l_onoff = 1;
  linger.l_linger = 0;
  const int clientSocket = accept(serverSocket, NULL, NULL);
  const int serverPort = serverSocketPortMap[serverSocket];
  setsockopt(clientSocket, SOL_SOCKET, SO_LINGER, &linger, sizeof(_linger));
  if (clientSocket == -1) {
    std::cout << "[ERROR] accept() error" << std::endl;
    return -1;
  }
  fcntl(clientSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
  addClient(clientSocket, serverConfigs[serverPort], &eventHandler);
  eventHandler.registerEnabledReadEvent(clientSocket, handlerMap[clientSocket]);
  eventHandler.registerDisabledWriteEvent(clientSocket,
                                          handlerMap[clientSocket]);
  return clientSocket;
}

bool WebServer::hasServerFd(struct kevent& currEvent) {
  return serverSocketPortMap.find(currEvent.ident) != serverSocketPortMap.end();
}

void WebServer::disconnectPort(struct kevent& currEvent) {
  close(currEvent.ident);
  serverConfigs.erase(serverSocketPortMap[currEvent.ident]);
}

bool WebServer::isClient(int clientFd) {
  return handlerMap.find(clientFd) != handlerMap.end();
}

void WebServer::disconnectClient(int clientFd) {
  close(clientFd);
  handlerMap.erase(clientFd);
}

void WebServer::clearClients() {
  std::set<int>::iterator it = candidatesForDisconnection.begin();
  for (; it != candidatesForDisconnection.end(); it++) {
    std::map<int, Handler*>::iterator handlerIterator = handlerMap.find(*it);
    // handlerIterator->second
    // TODO: close 로직 세우기 협의 필요
    disconnectClient(*it);
  }
  candidatesForDisconnection.clear();
}

void WebServer::addCandidatesForDisconnection(int clientFd) {
  candidatesForDisconnection.insert(clientFd);
}

int WebServer::addClient(int clientFd, ServerConfiguration* serverConfig,
                         Event* eventHandler) {
  handlerMap[clientFd] = new Handler(
      clientFd, serverConfig, eventHandler);  // TODO: 메모리 delete 확인해주기
  return clientFd;
};