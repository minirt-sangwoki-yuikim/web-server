#include"HttpHandler.hpp"

HttpHandler::HttpHandler()
{}

HttpHandler::HttpHandler(int fd, ServerConfiguration *serverConfig)
{
    this->fd = fd;
    this->serverConfig = serverConfig;
    this->data = "";
    this->bodySize = -1;
}

HttpHandler::HttpHandler(int fd, std::string data, ServerConfiguration *serverConfig)
{
    this->fd = fd;
    this->data = data;
    this->serverConfig = serverConfig;
    this->bodySize = -1;
}

HttpHandler::HttpHandler(int fd, HttpResponse res, long long bodySize)
{
    this->fd = fd;
    this->serverConfig = res.getServerConfiguration();
    this->response = res;
    this->data = "";
    this->bodySize = bodySize;
}

HttpHandler::HttpHandler(int fd, HttpRequest req, HttpResponse res)
{
    this->fd = fd;
    this->request = req;
    this->response = res;
    this->serverConfig = res.getServerConfiguration();
    this->data = "";
    this->bodySize = -1;
}

HttpHandler::HttpHandler(int fd, HttpRequest req, HttpResponse res, long long bodySize)
{
    this->fd = fd;
    this->request = req;
    this->response = res;
    this->serverConfig = res.getServerConfiguration();
    this->data = "";
    this->bodySize = bodySize;
}

HttpHandler::HttpHandler(int fd, HttpRequest req, HttpResponse res, ServerConfiguration *serverConfig)
{
    this->fd = fd;
    this->request = req;
    this->response = res;
    this->serverConfig = serverConfig;
    this->data = "";
    this->bodySize = -1;
}

HttpHandler::~HttpHandler()
{}

HttpHandler::HttpHandler(const HttpHandler& ref)
{
    fd = ref.fd;
    request = ref.request;
    response = ref.response;
    serverConfig = ref.serverConfig;
    data = ref.data;
    bodySize = ref.bodySize;
}

HttpHandler&	HttpHandler::operator=(const HttpHandler& ref)
{
    if (this == &ref)
        return (*this);
    fd = ref.fd;
    request = ref.request;
    response = ref.response;
    serverConfig = ref.serverConfig;
    data = ref.data;
    bodySize = ref.bodySize;
    return (*this);
}

int	HttpHandler::getFd(void)
{
    return (fd);
}

HttpRequest&    HttpHandler::getHttpRequest(void)
{
    return ((request));
}

HttpResponse&   HttpHandler::getHttpResponse(void)
{
    return ((response));
}

ServerConfiguration*	HttpHandler::getServerConfiguration(void)
{
    return (serverConfig);
}

std::string		HttpHandler::getData(void)
{
    return (this->data);
}

void    HttpHandler::setData(std::string data)
{
    this->data = data;
}

long long   HttpHandler::getBodySize(void)
{
    return (this->bodySize);
}