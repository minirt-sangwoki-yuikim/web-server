#ifndef HTTP_REQUEST_HANDLER_HPP
# define HTTP_REQUEST_HANDLER_HPP

# include "HttpRequest.hpp"

class HttpRequestHandler
{
	private:
		static std::map<int, std::string> buffers; // 요청을 읽어오는 소켓, 버퍼
		static std::map<int, HttpRequest*> chunkeds; // chunked 수신 중인 소켓, request 객체

		static int readRequest(int socket_fd);
		static void removeBuffer(int socket_fd);

	public:
		static void handle(int socket_fd);

		static int ChunkedRequestHandling(int socket_fd, HttpRequest* request);
		static void	errorHandling(const char	*erorr_code, int socket_fd);
		static HttpRequest *removeChunkedRequest(int socket_fd);
		static void removeAndDeleteChunkedRequest(int socket_fd);
		static HttpRequest *getChunkedRequest(int socket_fd);
		static const std::string& getBuffer(int socket_fd);
		static void addChunkedRequest(int socket_fd, HttpRequest *request);
		static void removePartOfBuffer(int socket_fd, int start, int count);
};

#endif