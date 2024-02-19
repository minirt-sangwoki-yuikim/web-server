#include"MyController.hpp"

MyController::MyController() : Controller()
{}

MyController::MyController(int masking) : Controller(masking)
{}

MyController::~MyController()
{}

void    MyController::service(HttpRequest &request, HttpResponse &response)
{
    std::string redirectPath;
    std::string staticPath;
    std::string cgiPath;
    ServerConfiguration *serverConfig = response.getServerConfiguration();

    if (isAcceptableMethod(request.getMethod()) == false)
        throw "405";
    // ============================ 애매한 부분 ============================ //
    // 1. redirectPath는 어떻게 설정하는가?
    // 2. CGI 파열 경로는 어떻게 설정하는가? -> 42번 줄 확인 바람!
    // 3. getResourcePath는 어떻게 설정되고 무엇을 반환하는가?
    // ================================================================== //
    std::cout << "MyController::service" << "\n";
    if (request.getMethod() == "GET" && request.getQueryString() == "")
    {
        redirectPath = serverConfig->getRedirectionPath(request.getPath()).second;
        if (redirectPath != "")
        {
            response.setStatusCode(serverConfig->getRedirectionPath(request.getPath()).first);
            request.setPath(redirectPath);
            response.redirect(request.getPath());
        }
        else
        {
            staticPath = serverConfig->getResourcePath(request.getPath()); // 4. 없으면 무엇을 반환하는가? -> error를 던진다.
            std::cout << staticPath << " = staticPath\n";
            request.setPath(staticPath);
            response.forward(request);
        } 
    }
    else
    {
        // CGI 파일 경로 설정을 따로 해야 할 것으로 보임.. 아마도요?
        // request.setPath("CGI PATH");
        cgiPath = serverConfig->getResourcePath(request.getPath());
        request.setPath(cgiPath);
        if (request.getMethod() == "GET" || request.getHeader("CONTENT-TYPE") == "application/x-www-form-urlencoded") // get, post
            doGet(request, response);
        else if (request.getMethod() == "POST") // file upload
            doPost(request, response);
        else
            doDelete(request, response);
    }
}

// Controller를 Server 개수만큼만 만들자.