#include "extension.h"

#define MAX_PROCESS 10

WebsocketExtension g_WebsocketExt;
SMEXT_LINK(&g_WebsocketExt);

HandleType_t g_htWsClient, g_htWsServer, g_htJSON, g_htHttp;
WsClientHandler g_WsClientHandler;
WsServerHandler g_WsServerHandler;
JSONHandler g_JSONHandler;
HttpHandler g_HttpHandler;

ThreadSafeQueue<std::function<void()>> g_TaskQueue;

static void OnGameFrame(bool simulating) {
	std::function<void()> task;
	int count = 0;
	while (g_TaskQueue.TryPop(task) && count < MAX_PROCESS) {
		task();
		count++;
	}
}

bool WebsocketExtension::SDK_OnLoad(char* error, size_t maxlen, bool late)
{
	sharesys->AddNatives(myself, ws_natives);
	sharesys->AddNatives(myself, ws_natives_server);
	sharesys->AddNatives(myself, json_natives);
	sharesys->AddNatives(myself, http_natives);
	sharesys->RegisterLibrary(myself, "websocket");
	
	HandleAccess haDefaults;
	handlesys->InitAccessDefaults(NULL, &haDefaults);
	haDefaults.access[HandleAccess_Delete] = 0;
	g_htWsClient = handlesys->CreateType("WebSocket", &g_WsClientHandler, 0, NULL, &haDefaults, myself->GetIdentity(), NULL);
	g_htWsServer = handlesys->CreateType("WebSocketServer", &g_WsServerHandler, 0, NULL, &haDefaults, myself->GetIdentity(), NULL);
	g_htHttp = handlesys->CreateType("HttpRequest", &g_HttpHandler, 0, NULL, &haDefaults, myself->GetIdentity(), NULL);
	g_htJSON = handlesys->CreateType("YYJSON", &g_JSONHandler, 0, NULL, &haDefaults, myself->GetIdentity(), NULL);

	smutils->AddGameFrameHook(&OnGameFrame);
	return true;
}

void WebsocketExtension::SDK_OnUnload()
{
	handlesys->RemoveType(g_htWsClient, myself->GetIdentity());
	handlesys->RemoveType(g_htWsServer, myself->GetIdentity());
	handlesys->RemoveType(g_htJSON, myself->GetIdentity());
	handlesys->RemoveType(g_htHttp, myself->GetIdentity());

	smutils->RemoveGameFrameHook(&OnGameFrame);
}

void WsClientHandler::OnHandleDestroy(HandleType_t type, void *object)
{
	delete (WebSocketClient *)object;
}

void WsServerHandler::OnHandleDestroy(HandleType_t type, void *object)
{
	delete (WebSocketServer *)object;
}

void JSONHandler::OnHandleDestroy(HandleType_t type, void *object)
{
	delete (YYJsonWrapper *)object;
}

void HttpHandler::OnHandleDestroy(HandleType_t type, void *object)
{
	delete (HttpRequest *)object;
}

YYJsonWrapper *WebsocketExtension::GetJSONPointer(IPluginContext *pContext, Handle_t handle)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	YYJsonWrapper *pYYJsonWrapper;
	if ((err = handlesys->ReadHandle(handle, g_htJSON, &sec, (void **)&pYYJsonWrapper)) != HandleError_None)
	{
		pContext->ReportError("Invalid JSON handle %x (error %d)", handle, err);
		return NULL;
	}

	return pYYJsonWrapper;
}