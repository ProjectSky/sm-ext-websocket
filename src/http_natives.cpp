#include "extension.h"

static HttpRequest *GetHttpPointer(IPluginContext *pContext, Handle_t Handle)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	HttpRequest *httpClient;
	if ((err = handlesys->ReadHandle(Handle, g_htHttp, &sec, (void **)&httpClient)) != HandleError_None)
	{
		pContext->ReportError("Invalid httpClient handle %x (error %d)", Handle, err);
		return NULL;
	}

	return httpClient;
}

static cell_t http_CreateRequest(IPluginContext *pContext, const cell_t *params)
{
	char *url, *method;
	pContext->LocalToString(params[1], &url);
	pContext->LocalToString(params[2], &method);

	HttpRequest* pHttpRequest = new HttpRequest(url, method);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pHttpRequest->m_httpclient_handle = handlesys->CreateHandleEx(g_htHttp, pHttpRequest, &sec, NULL, &err);

	if (pHttpRequest->m_httpclient_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create HttpRequest handle (error %d)", err);
		return BAD_HANDLE;
	}

	return pHttpRequest->m_httpclient_handle;
}

static cell_t http_SetResponseCallback(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);

	if (!pHttpRequest)
	{
		return 0;
	}

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);
	
	pHttpRequest->pResponseForward = forwards->CreateForwardEx(NULL, ET_Ignore, 3, NULL, Param_Cell, Param_String, Param_Cell);
	if (!pHttpRequest->pResponseForward || !pHttpRequest->pResponseForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create response forward.");
		return 0;
	}

	return 1;
}

static cell_t http_Perform(IPluginContext *pContext, const cell_t *params)
{
	HttpRequest *pHttpRequest = GetHttpPointer(pContext, params[1]);

	if (!pHttpRequest)
	{
		return 0;
	}

	return pHttpRequest->Perform();
}

const sp_nativeinfo_t http_natives[] =
{
	{"HttpRequest.HttpRequest", http_CreateRequest},
	{"HttpRequest.SetResponseCallback", http_SetResponseCallback},
	{"HttpRequest.Perform", http_Perform},
	{NULL, NULL}
};