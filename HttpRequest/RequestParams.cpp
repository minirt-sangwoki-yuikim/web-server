#include "RequestParams.hpp"

void RequestParams::addQuearyString(const std::string& query_string)
{
	std::map<std::string, std::string> parsed = HttpRequestUtility::parseQueryString(query_string);
	params.insert(parsed.begin(), parsed.end());
}

std::string RequestParams::getParameter(const std::string& param)
{
	return (params[param]);
}

// 테스트용
std::map<std::string, std::string>::iterator RequestParams::getBegin()
{
	return (params.begin());
}
std::map<std::string, std::string>::iterator RequestParams::getEnd()
{
	return (params.end());
}