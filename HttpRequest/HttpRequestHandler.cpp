#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "HttpRequestHandler.hpp"
#include "../ResponseHandler/FrontController.hpp"

#define NO_CHUNKED_REQUEST 1
#define IN_PROGRESS_CHUNKED_REQUEST 2
#define CLOSE_SOCKET 1

std::map<int, std::string> 		HttpRequestHandler::buffers; // 요청을 읽어오는 소켓, 버퍼
std::map<int, HttpRequest *>	HttpRequestHandler::chunkeds; // chunked 수신 중인 소켓, request 객체
std::map<int, bool>				HttpRequestHandler::read_flags;

HttpRequestHandler::HttpRequestHandler(int _socket_fd, ServerConfiguration *_server_config)
	: socket_fd(_socket_fd), server_config(_server_config) {}

int HttpRequestHandler::handle(Event *event)
{
	try
	{
		readRequest(); // 소켓으로부터 요청 읽어오기
		while (true)
		{
			if (RequestAndResponse(event) == FAILURE) // 불완전한 요청인 경우
				return (FAILURE);
			if (buffers.find(socket_fd) == buffers.end() || 
				buffers.at(socket_fd) == "") // 버퍼의 요청을 모두 처리한 경우
			{
				// 버퍼는 다 처리했지만, chunked 요청 중인 경우 -> 요청을 더 받기
				if (chunkeds.find(socket_fd) != chunkeds.end())
					return (FAILURE);
				break;
			} 
		}
	}
	catch (const ClientCloseSocketException &e)
	{
		return (CLOSE_SOCKET);
	}
	catch (const SocketCloseException500 &e)
	{
		return (CLOSE_SOCKET);
	}
	catch (std::exception &e)
	{
		errorHandling(e.what(), server_config, event);
		removeAndDeleteChunkedRequest(socket_fd);
		removeBuffer(socket_fd);
	}
	return (SUCCESS);
}

int HttpRequestHandler::RequestAndResponse(Event *event)
{
	HttpRequest *request = NULL;
	try
	{
		// Request Part
		// std::cout << "Request part\n";
		request = HttpRequestFactory::create(socket_fd, server_config);
		if (request == NULL)
			return (FAILURE);  // 버퍼에 완전한 요청이 없음
		if (ChunkedRequestHandling(request) == IN_PROGRESS_CHUNKED_REQUEST)
		{
			// std::cout << "여기로 와야 하는데!\n";
			return (SUCCESS); // chunk 요청 중인 경우 -> 아직 response 하지 않음
		}

		// Response Part
		// std::cout << "Response part\n";
		int kqueue_fd = 0;
		FrontController front_controller(socket_fd, server_config, event);
		front_controller.run(*request);

		delete request;
	}
	catch (const char *e) // 유효하지 않은 요청 -> 오류 응답
	{
		errorHandling(e, server_config, event);
		delete request;
		request = NULL;
	}
	return (SUCCESS);
}

// 버퍼에 추가적으로 요청을 읽어오는 함수
void HttpRequestHandler::readRequest()
{
	if (buffers.find(socket_fd) == buffers.end()) 	// 버퍼가 존재하지 않는 경우 추가하기
		buffers.insert(std::pair<int, std::string>(socket_fd, ""));

	// 읽어올 크기 read_size 설정하기
	long read_size;
	HttpRequest *request = getChunkedRequest(socket_fd);
	if (request != NULL)	// 청크 요청 : Location body size + 23
		read_size = server_config->getClientBodySize(request->getPath()) + 23;
	else					// 일반 요청 : server header size + server body size
		read_size = server_config->getClientRequestSize("");

	// read_size 만큼 temp_buffer에 읽어오기
	char *temp_buffer = new char[read_size];
	long read_byte = recv(socket_fd, temp_buffer, read_size, 0);
	if (read_byte == -1) { // recv 시스템 콜 오류
		delete[] temp_buffer;
		throw SocketCloseException500();
	} else if (read_byte == 0) { // 클라이언트 연결 끊김
		delete[] temp_buffer;
		throw ClientCloseSocketException();
	}

	// 읽어온 내용 버퍼에 추가하기
	buffers[socket_fd].append(temp_buffer, read_byte);
	delete[] temp_buffer;
}

int HttpRequestHandler::ChunkedRequestHandling(HttpRequest *request)
{
	if (request->getHeader("Transfer-Encoding") != "chunked")
		return (NO_CHUNKED_REQUEST);

	// chunkeds에 포함된 경우 -> 아직 chunked 요청 전체가 끝나지 않았음
	if (chunkeds.find(socket_fd) != chunkeds.end())
		return (IN_PROGRESS_CHUNKED_REQUEST);

	// 마지막 chunked 요청인 경우
	HttpRequestParser::parseRequestParams(request);
	return (LAST_CHUNKED_REQUEST);
}

void HttpRequestHandler::errorHandling(const char *erorr_code, ServerConfiguration *server_config, Event *event)
{
	HttpRequest empty;
	HttpResponse response(socket_fd, server_config, event);

	std::cout << "erorr_code : " << erorr_code << "\n";
	response.setStatusCode(erorr_code);
	response.forward(empty);
}

void HttpRequestHandler::removeBuffer(int socket_fd)
{
	std::map<int, std::string>::iterator it = buffers.find(socket_fd);
	if (it != buffers.end())
		buffers.erase(it);
}

HttpRequest *HttpRequestHandler::removeChunkedRequest(int socket_fd)
{
	std::map<int, HttpRequest *>::iterator it = chunkeds.find(socket_fd);
	if (it == chunkeds.end())
		return (NULL);
	HttpRequest *request = it->second;
	chunkeds.erase(it);
	return (request);
}

void HttpRequestHandler::removeAndDeleteChunkedRequest(int socket_fd)
{
	HttpRequest *request = removeChunkedRequest(socket_fd);
	delete request;
	request = NULL;
}

HttpRequest *HttpRequestHandler::getChunkedRequest(int socket_fd)
{
	std::map<int, HttpRequest *>::iterator it = chunkeds.find(socket_fd);
	if (it == chunkeds.end())
		return (NULL);
	else
		return (it->second);
}

const std::string &HttpRequestHandler::getBuffer(int socket_fd)
{
	return (buffers[socket_fd]);
}

void HttpRequestHandler::addChunkedRequest(int socket_fd, HttpRequest *request)
{
	chunkeds[socket_fd] = request;
}

void HttpRequestHandler::removePartOfBuffer(int socket_fd, int start, int count)
{
	buffers[socket_fd].erase(start, count);
}

bool	HttpRequestHandler::getReadFinish(int socket_fd)
{
	return (read_flags[socket_fd]);
}

void	HttpRequestHandler::setReadFlag(int socket_fd, bool flag)
{
	read_flags[socket_fd] = flag;
}