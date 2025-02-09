#ifndef REQUEST_LINE_HPP
# define REQUEST_LINE_HPP

# include "RequestUtility.hpp"

class RequestLine
{
	private:
		std::string method;
		std::string path;
		std::string query_string;
		std::string protocol_string;

		void parseMethod(std::string method_string);
		void parseURI(std::string uri_string);
		void parseProtocol(std::string protocol_string);

	public:
		RequestLine(const std::string& input);
		RequestLine(const RequestLine& ref);

		std::string getMethod() const;
		std::string getPath() const;
		std::string getQueryString() const;
		std::string getProtocolString() const;

		void setMethod(const std::string& _method);
		void setPath(const std::string& _path);
		void setQueryString(std::string querystring);
};

#endif