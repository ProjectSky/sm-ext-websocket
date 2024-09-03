#include "extension.h"

static WebSocketClient *GetWsPointer(IPluginContext *pContext, Handle_t Handle)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	WebSocketClient *ws;
	if ((err = handlesys->ReadHandle(Handle, g_htWsClient, &sec, (void **)&ws)) != HandleError_None)
	{
		pContext->ReportError("Invalid WebSocket handle %x (error %d)", Handle, err);
		return NULL;
	}

	return ws;
}

static cell_t ws_CreateWebSocketClient(IPluginContext *pContext, const cell_t *params)
{
	char *url;
	pContext->LocalToString(params[1], &url);

	WebSocketClient* pWebSocketClient = new WebSocketClient(url, params[2]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pWebSocketClient->m_websocket_handle = handlesys->CreateHandleEx(g_htWsClient, pWebSocketClient, &sec, NULL, &err);

	if (pWebSocketClient->m_websocket_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create WebSocketClient handle (error %d)", err);
		return BAD_HANDLE;
	}

	return pWebSocketClient->m_websocket_handle;
}

static cell_t ws_SetMessageCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient *pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);
	
	pWebSocketClient->pMessageForward = forwards->CreateForwardEx(NULL, ET_Ignore, 3, NULL, Param_Cell, pWebSocketClient->m_callback_type == WebSocket_JSON ? Param_Cell : Param_String, Param_Cell);
	if (!pWebSocketClient->pMessageForward || !pWebSocketClient->pMessageForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create message forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_SetOpenCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient *pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	pWebSocketClient->pOpenForward = forwards->CreateForwardEx(NULL, ET_Ignore, 1, NULL, Param_Cell);
	if (!pWebSocketClient->pOpenForward || !pWebSocketClient->pOpenForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create open forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_SetCloseCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient *pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	pWebSocketClient->pCloseForward = forwards->CreateForwardEx(NULL, ET_Ignore, 3, NULL, Param_Cell, Param_Cell, Param_String);
	if (!pWebSocketClient->pCloseForward || !pWebSocketClient->pCloseForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create close forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_SetErrorCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient *pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	pWebSocketClient->pErrorForward = forwards->CreateForwardEx(NULL, ET_Ignore, 2, NULL, Param_Cell, Param_String);
	if (!pWebSocketClient->pErrorForward || !pWebSocketClient->pErrorForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create error forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_Connect(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	if (pWebSocketClient->IsConnected())
	{
		pContext->ReportError("WebSocket is already connected!");
		return 0;
	}

	pWebSocketClient->m_webSocket->start();

	return 1;
}

static cell_t ws_Disconnect(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	pWebSocketClient->m_webSocket->stop();

	return 1;
}

static cell_t ws_SetHeader(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	char *key, *value;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &value);

	pWebSocketClient->m_extraHeaders[key] = value;
	pWebSocketClient->m_webSocket->setExtraHeaders(pWebSocketClient->m_extraHeaders);

	return 1;
}

static cell_t ws_GetHeader(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	char *key;
	pContext->LocalToString(params[2], &key);

	auto it = pWebSocketClient->m_headers.find(key);

	if (it != pWebSocketClient->m_headers.end()) {
		pContext->StringToLocalUTF8(params[3], params[4], it->second.c_str(), NULL);
		return 1;
	}

	return 0;
}

static cell_t ws_GetConnected(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	return pWebSocketClient->IsConnected();
}

static cell_t ws_GetReadyState(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	return (cell_t)pWebSocketClient->m_webSocket->getReadyState();
}

static cell_t ws_WriteString(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	char *msg;
	pContext->LocalToString(params[2], &msg);

	pWebSocketClient->m_webSocket->send(msg);

	return 1;
}

static cell_t ws_WriteJSON(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	YYJsonWrapper* pYYJsonWrapper = g_WebsocketExt.GetJSONPointer(pContext, params[2]);

	if (!pYYJsonWrapper)
	{
		return 0;
	}

	char *json_str = yyjson_mut_val_write(pYYJsonWrapper->m_pVal_mut, 0, NULL);
	
	if (!json_str)
	{
		return 0;
	}

	pWebSocketClient->m_webSocket->send(json_str);
	free(json_str);

	return 1;
}

static cell_t ws_AutoReconnect(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	if (params[0] == 2) {
		params[2] ? pWebSocketClient->m_webSocket->enableAutomaticReconnection() : pWebSocketClient->m_webSocket->disableAutomaticReconnection();
		return 1;
	}

	return pWebSocketClient->m_webSocket->isAutomaticReconnectionEnabled();
}

static cell_t ws_MinReconnectWait(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	if (!pWebSocketClient->m_webSocket->isAutomaticReconnectionEnabled())
	{
		pContext->ReportError("auto reconnection not enabled!");
		return 0;
	}

	if (params[0] == 2) {
		pWebSocketClient->m_webSocket->setMinWaitBetweenReconnectionRetries(params[2]);
		return 1;
	}

	return pWebSocketClient->m_webSocket->getMinWaitBetweenReconnectionRetries();
}

static cell_t ws_MaxReconnectWait(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	if (!pWebSocketClient->m_webSocket->isAutomaticReconnectionEnabled())
	{
		pContext->ReportError("auto reconnection not enabled!");
		return 0;
	}

	if (params[0] == 2) {
		pWebSocketClient->m_webSocket->setMaxWaitBetweenReconnectionRetries(params[2]);
		return 1;
	}

	return pWebSocketClient->m_webSocket->getMaxWaitBetweenReconnectionRetries();
}

static cell_t ws_SetHandshakeTimeout(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	pWebSocketClient->m_webSocket->setHandshakeTimeout(params[2]);
	return 1;
}

static cell_t ws_PingInterval(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient)
	{
		return 0;
	}

	if (params[0] == 2) {
		pWebSocketClient->m_webSocket->setPingInterval(params[2]);
		return 1;
	}

	return pWebSocketClient->m_webSocket->getPingInterval();
}

const sp_nativeinfo_t ws_natives[] =
{
	// client
	{"WebSocket.WebSocket",              ws_CreateWebSocketClient},
	{"WebSocket.SetMessageCallback",     ws_SetMessageCallback},
	{"WebSocket.SetOpenCallback",        ws_SetOpenCallback},
	{"WebSocket.SetCloseCallback",       ws_SetCloseCallback},
	{"WebSocket.SetErrorCallback",       ws_SetErrorCallback},
	{"WebSocket.Connect",                ws_Connect},
	{"WebSocket.SetHeader",              ws_SetHeader},
	{"WebSocket.GetHeader",              ws_GetHeader},
	{"WebSocket.WriteString",            ws_WriteString},
	{"WebSocket.WriteJSON",              ws_WriteJSON},
	{"WebSocket.Disconnect",             ws_Disconnect},
	{"WebSocket.Connected.get",          ws_GetConnected},
	{"WebSocket.ReadyState.get",         ws_GetReadyState},
	{"WebSocket.AutoReconnect.get",      ws_AutoReconnect},
	{"WebSocket.AutoReconnect.set",      ws_AutoReconnect},
	{"WebSocket.MinReconnectWait.get",   ws_MinReconnectWait},
	{"WebSocket.MinReconnectWait.set",   ws_MinReconnectWait},
	{"WebSocket.MaxReconnectWait.get",   ws_MaxReconnectWait},
	{"WebSocket.MaxReconnectWait.set",   ws_MaxReconnectWait},
	{"WebSocket.HandshakeTimeout.set",   ws_SetHandshakeTimeout},
	{"WebSocket.PingInterval.get",       ws_PingInterval},
	{"WebSocket.PingInterval.set",       ws_PingInterval},
	{NULL, NULL}
};