#include "extension.h"

static WebSocketServer *GetWsServerPointer(IPluginContext *pContext, Handle_t Handle)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	WebSocketServer *server;
	if ((err = handlesys->ReadHandle(Handle, g_htWsServer, &sec, (void **)&server)) != HandleError_None)
	{
		pContext->ReportError("Invalid WebSocketServer handle %x (error %d)", Handle, err);
		return NULL;
	}

	return server;
}

static cell_t ws_CreateWebSocketServer(IPluginContext *pContext, const cell_t *params)
{
	char *url;
	pContext->LocalToString(params[1], &url);

	WebSocketServer* pWebsocketServer = new WebSocketServer(url, params[2], params[3] ? AF_INET6 : AF_INET, params[4]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pWebsocketServer->m_webSocketServer_handle = handlesys->CreateHandleEx(g_htWsServer, pWebsocketServer, &sec, NULL, &err);

	if (!pWebsocketServer->m_webSocketServer_handle)
	{
		pContext->ReportError("Could not create WebSocketServer handle (error %d)", err);
		return 0;
	}

	return pWebsocketServer->m_webSocketServer_handle;
}

static cell_t ws_SetMessageCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer *pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);
	
	pWebsocketServer->pMessageForward = forwards->CreateForwardEx(NULL, ET_Ignore, 6, NULL, Param_Cell, Param_Cell, Param_String, Param_Cell, Param_String, Param_String);
	if (!pWebsocketServer->pMessageForward || !pWebsocketServer->pMessageForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create message forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_SetOpenCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer *pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	pWebsocketServer->pOpenForward = forwards->CreateForwardEx(NULL, ET_Ignore, 3, NULL, Param_Cell, Param_String, Param_String);
	if (!pWebsocketServer->pOpenForward || !pWebsocketServer->pOpenForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create open forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_SetCloseCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer *pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	pWebsocketServer->pCloseForward = forwards->CreateForwardEx(NULL, ET_Ignore, 5, NULL, Param_Cell, Param_Cell, Param_String, Param_String, Param_String);
	if (!pWebsocketServer->pCloseForward || !pWebsocketServer->pCloseForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create close forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_SetErrorCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer *pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	pWebsocketServer->pErrorForward = forwards->CreateForwardEx(NULL, ET_Ignore, 4, NULL, Param_Cell, Param_String, Param_String, Param_String);
	if (!pWebsocketServer->pErrorForward || !pWebsocketServer->pErrorForward->AddFunction(callback))
	{
		pContext->ReportError("Could not create error forward.");
		return 0;
	}

	return 1;
}

static cell_t ws_Start(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	auto res = pWebsocketServer->m_webSocketServer.listen();

	if (!res.first)
	{
		pContext->ReportError("Initiating WebSocket Server failed: %s", res.second.c_str());
		return 0;
	}

	pWebsocketServer->m_webSocketServer.start();

	return 1;
}

static cell_t ws_Stop(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	pWebsocketServer->m_webSocketServer.stop();

	return 1;
}

static cell_t ws_SendMessageToClient(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	char *clientId, *msg;
	pContext->LocalToString(params[2], &clientId);
	pContext->LocalToString(params[3], &msg);

	return pWebsocketServer->sendToClient(clientId, msg);
}

static cell_t ws_BroadcastMessage(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	char *msg;
	pContext->LocalToString(params[2], &msg);

	pWebsocketServer->broadcastMessage(msg);

	return 1;
}

static cell_t ws_GetClientsCount(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	return pWebsocketServer->m_webSocketServer.getClients().size();
}

static cell_t ws_SetOrGetPongEnable(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	if (params[0] == 2) {
		params[2] ? pWebsocketServer->m_webSocketServer.enablePong() : pWebsocketServer->m_webSocketServer.disablePong();
		return 1;
	}

	return pWebsocketServer->m_webSocketServer.isPongEnabled();
}

static cell_t ws_DisableDeflate(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	if (!pWebsocketServer->m_webSocketServer.isPerMessageDeflateEnabled())
	{
		pContext->ReportError("Per Message Deflate not enabled!");
		return 0;
	}

	pWebsocketServer->m_webSocketServer.disablePerMessageDeflate();
	return 1;
}

static cell_t ws_IsDeflateEnabled(IPluginContext *pContext, const cell_t *params)
{
	WebSocketServer* pWebsocketServer = GetWsServerPointer(pContext, params[1]);

	if (!pWebsocketServer)
	{
		return 0;
	}

	return pWebsocketServer->m_webSocketServer.isPerMessageDeflateEnabled();
}

static cell_t ws_GetClients(IPluginContext *pContext, const cell_t *params)
{
	// TODO: Implement
	return 1;
}

const sp_nativeinfo_t ws_natives_server[] =
{
	{"WebSocketServer.WebSocketServer",        ws_CreateWebSocketServer},
	{"WebSocketServer.SetMessageCallback",     ws_SetMessageCallback},
	{"WebSocketServer.SetOpenCallback",        ws_SetOpenCallback},
	{"WebSocketServer.SetCloseCallback",       ws_SetCloseCallback},
	{"WebSocketServer.SetErrorCallback",       ws_SetErrorCallback},
	{"WebSocketServer.Start",                  ws_Start},
	{"WebSocketServer.Stop",                   ws_Stop},
	{"WebSocketServer.BroadcastMessage",       ws_BroadcastMessage},
	{"WebSocketServer.SendMessageToClient",    ws_SendMessageToClient},
	{"WebSocketServer.ClientsCount.get",       ws_GetClientsCount},
	{"WebSocketServer.EnablePong.get",         ws_SetOrGetPongEnable},
	{"WebSocketServer.EnablePong.set",         ws_SetOrGetPongEnable},
	{"WebSocketServer.GetClients",             ws_GetClients},
	{"WebSocketServer.DisableDeflate",         ws_DisableDeflate},
	{"WebSocketServer.IsDeflateEnabled",       ws_IsDeflateEnabled},
	{NULL, NULL}
};