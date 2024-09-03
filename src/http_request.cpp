#include "extension.h"

HttpRequest::HttpRequest(const std::string &url, const std::string &method) : m_httpclient(true)
{
	this->m_request = m_httpclient.createRequest(url, method);
}

HttpRequest::~HttpRequest() 
{
}

void HttpRequest::onResponse(const ix::HttpResponsePtr response) 
{
	if (!this->pResponseForward || !this->pResponseForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, response]()
	{
		HandleError err;
		HandleSecurity sec(NULL, myself->GetIdentity());

		this->pResponseForward->PushCell(this->m_httpclient_handle);
		this->pResponseForward->PushString(response->body.c_str());
		this->pResponseForward->PushCell(response->statusCode);
		this->pResponseForward->Execute(NULL);

		handlesys->FreeHandle(this->m_httpclient_handle, &sec);
	});
}

bool HttpRequest::Perform()
{
	return this->m_httpclient.performRequest(m_request, std::bind(&HttpRequest::onResponse, this, std::placeholders::_1));
}