#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include "../server/Event.hpp"
#include "HttpRequestFactory.hpp"

class Event;

class HttpRequestHandler
{
	private:
		int socket_fd;
		ServerConfiguration *server_config;

		static std::map<int, std::string> buffers;  // 요청을 읽어오는 소켓, 버퍼
		static std::map<int, HttpRequest *> chunkeds;  // chunked 수신 중인 소켓, request 객체
		static std::map<int, bool>  read_flags;
		void readRequest();

	public:
		HttpRequestHandler(int socket_fd, ServerConfiguration *server_config);
		static void removeBuffer(int socket_fd);

		int handle(Event *event);
		int RequestAndResponse(Event *event);
		int ChunkedRequestHandling(HttpRequest *request);
		void errorHandling(const char *erorr_code, ServerConfiguration *serverConfig, Event *event);

		static HttpRequest *removeChunkedRequest(int socket_fd);
		static void removeAndDeleteChunkedRequest(int socket_fd);
		static HttpRequest *getChunkedRequest(int socket_fd);
		static const std::string &getBuffer(int socket_fd);
		static void addChunkedRequest(int socket_fd, HttpRequest *request);
		static void removePartOfBuffer(int socket_fd, int start, int count);
		static bool getReadFinish(int socket_fd);
		static void setReadFlag(int socket_fd, bool flag);
};

#endif