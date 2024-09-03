#include "extension.h"

class HttpRequest
{
public:
	HttpRequest(const std::string &url, const std::string &method);
	~HttpRequest();
	
	bool Perform();

public:
	void onResponse(const ix::HttpResponsePtr response);
	
	ix::HttpClient m_httpclient;
	ix::HttpRequestArgsPtr m_request;

	Handle_t m_httpclient_handle = BAD_HANDLE;

	IChangeableForward *pResponseForward = NULL;
};