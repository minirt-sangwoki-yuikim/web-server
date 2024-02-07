#include "Server.hpp"

#include <iostream>
#include <sstream>
#include <vector>

Server::Server()
    : listen(80),
      serverName(""),
      root("/"),
      uploadedPath(""),
      autoIndex(false),
      clientHeaderSize(10000),
      clientBodySize(10000),
      keepAliveTimeout(500) {}

Server::~Server() {}

int Server::getListen() const { return this->listen; }

int Server::getKeepAliveTimeout() const { return this->keepAliveTimeout; }

std::map<std::string, Location> Server::getLocations(void) const {
  return this->locations;
}

std::string Server::getServerName(void) const { return this->serverName; }

std::string Server::getRoot(void) const { return this->root; }

std::map<std::vector<int>, std::string> Server::getErrorPage(void) const {
  return this->errorPage;
}

std::map<std::string, int> Server::getRedirection(void) const {
  return this->redirection;
}

bool Server::getAutoIndex(void) const { return this->autoIndex; }

long Server::getClientBodySize() const {
  return this->clientBodySize;
}

long Server::getClientHeaderSize() const {
  return this->clientHeaderSize;
};

std::string Server::getUploadPath() const { return this->uploadedPath; }

bool Server::fillServer(std::map<std::string, Location>& mapLocations,
                        std::map<std::string, std::string>& mapSentence) {
  std::map<std::string, std::string>::iterator it = mapSentence.begin();

  this->locations = mapLocations;
  while (it != mapSentence.end()) {
    if (it->first == "listen")
      this->setListen(it->second);
    else if (it->first == "server_name")
      this->setServerName(it->second);
    else if (it->first == "root")
      this->setRoot(it->second);
    else if (it->first == "error_page") {
      if (this->setErrorPage(it->second)) return true;
    } else if (it->first == "return") {
      if (this->setRedirection(it->second)) return true;
    } else if (it->first == "autoindex")
      this->setAutoIndex(it->second);
    else if (it->first == "client_max_body_size")
      this->setClientBodySize(it->second);
    else if (it->first == "client_max_header_size")
      this->setClientHeaderSize(it->second);
    else if (it->first == "upload_path")
      this->setUploadPath(it->second);
    else if (it->first == "keepalive_timeout")
      this->setKeepAliveTimeout(it->second);
    else
      return true;
    it++;
  }
  return false;
}

void Server::setListen(std::string& input) {
  long parsedPort = strtod(input.c_str(), NULL);

  if (parsedPort < 0 || parsedPort > 65535)
    std::cout << "[WARN] invalid port number" << std::endl;
  this->listen = parsedPort;
}

void Server::setKeepAliveTimeout(std::string& input) {
  long keepAliveTimeout = strtod(input.c_str(), NULL);
  if (keepAliveTimeout < 0)
    std::cout << "[WARN] invalid keepAliveTimeout" << std::endl;
  this->keepAliveTimeout = keepAliveTimeout;
}

void Server::setServerName(std::string& input) { this->serverName = input; }

void Server::setRoot(std::string& input) { this->root = input; }

std::map<int, std::string> Server::getErrorCodePage() {
  return this->errorCodePage;
}

bool Server::setErrorPage(std::string const& sentence) {
  std::stringstream sstream;
  std::vector<std::string> tokens;
  std::string token;
  std::vector<int> vecErrorCode;
  std::string errorPath;

  sstream << sentence;
  while (sstream >> token) tokens.push_back(token);

  if (tokens.size() < 2) return true;
  errorPath = tokens.back();
  tokens.pop_back();

  for (size_t i = 0; i < tokens.size(); i++) {
    int code = std::strtod(tokens[i].c_str(), NULL);
    vecErrorCode.push_back(code);
    errorCodePage.insert(std::pair<int, std::string>(code, errorPath));
  }
  this->errorPage.insert(
      std::pair<std::vector<int>, std::string>(vecErrorCode, errorPath));
  return false;
}

bool Server::setRedirection(std::string const& sentence) {
  std::stringstream sstream;
  std::vector<std::string> tokens;
  std::string token;
  std::string redirectionPath;
  int redirectionCode;

  sstream << sentence;
  while (sstream >> token) tokens.push_back(token);

  if (tokens.size() != 2) return true;
  redirectionCode = std::strtod(tokens[0].c_str(), NULL);
  redirectionPath = tokens[1];
  this->redirection[redirectionPath] = redirectionCode;
  return false;
}

void Server::setAutoIndex(std::string& input) {
  if (input == "on")
    this->autoIndex = true;
  else
    this->autoIndex = false;
}

void Server::setClientBodySize(std::string& input) {
  this->clientBodySize = std::strtod(input.c_str(), NULL);
}

void Server::setClientHeaderSize(std::string& input) {
  this->clientBodySize = std::strtod(input.c_str(), NULL);
}

void Server::setUploadPath(std::string& input) { this->uploadedPath = input; }