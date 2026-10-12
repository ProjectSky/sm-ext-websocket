#include "ws_client.h"

static WebSocketClient *GetWsPointer(IPluginContext *pContext, Handle_t Handle)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	WebSocketClient *ws;
	if ((err = handlesys->ReadHandle(Handle, g_htWsClient, &sec, (void **)&ws)) != HandleError_None)
	{
		pContext->ReportError("Invalid WebSocket handle %x (error %d)", Handle, err);
		return nullptr;
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
	pWebSocketClient->m_websocket_handle = handlesys->CreateHandleEx(g_htWsClient, pWebSocketClient, &sec, nullptr, &err);

	if (!pWebSocketClient->m_websocket_handle)
	{
		pContext->ReportError("Could not create WebSocketClient handle (error %d)", err);
		return 0;
	}

	return pWebSocketClient->m_websocket_handle;
}

static cell_t ws_SetMessageCallback(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient *pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	if (pWebSocketClient->pMessageForward) {
		forwards->ReleaseForward(pWebSocketClient->pMessageForward);
	}

    auto forwardMiddleType = Param_Cell;

    switch(pWebSocketClient->m_callback_type) {
        default:
        case WebSocket_BINARY:
        case WebSocket_JSON:
            forwardMiddleType = ParamCell;
            break;
        case WebSocket_STRING:
            forwardMiddleType = Param_String;
            break;
    }

	pWebSocketClient->pMessageForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 3, nullptr, Param_Cell, forwardMiddleType, Param_Cell);

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

	if (!pWebSocketClient) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	if (pWebSocketClient->pOpenForward) {
		forwards->ReleaseForward(pWebSocketClient->pOpenForward);
	}

	pWebSocketClient->pOpenForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 1, nullptr, Param_Cell);
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

	if (!pWebSocketClient) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	if (pWebSocketClient->pCloseForward) {
		forwards->ReleaseForward(pWebSocketClient->pCloseForward);
	}

	pWebSocketClient->pCloseForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 3, nullptr,
		Param_Cell, Param_Cell, Param_String);
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

	if (!pWebSocketClient) return 0;

	IPluginFunction *callback = pContext->GetFunctionById(params[2]);

	if (pWebSocketClient->pErrorForward) {
		forwards->ReleaseForward(pWebSocketClient->pErrorForward);
	}

	pWebSocketClient->pErrorForward = forwards->CreateForwardEx(nullptr, ET_Ignore, 2, nullptr, Param_Cell, Param_String);
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

	if (!pWebSocketClient) return 0;

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

	if (!pWebSocketClient) return 0;

	if (!pWebSocketClient->IsConnected())
	{
		pContext->ReportError("WebSocket is not connected!");
		return 0;
	}

	pWebSocketClient->m_webSocket->stop();

	return 1;
}

static cell_t ws_SetHeader(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient) return 0;

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

	if (!pWebSocketClient) return 0;

	char *key;
	pContext->LocalToString(params[2], &key);

	std::lock_guard<std::mutex> lock(pWebSocketClient->m_headersMutex);
	auto it = pWebSocketClient->m_headers.find(key);

	if (it != pWebSocketClient->m_headers.end()) {
		pContext->StringToLocalUTF8(params[3], params[4], it->second.c_str(), nullptr);
		return 1;
	}

	return 0;
}

static cell_t ws_GetConnected(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient) return 0;

	return pWebSocketClient->IsConnected();
}

static cell_t ws_GetReadyState(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient) return 0;

	return (cell_t)pWebSocketClient->m_webSocket->getReadyState();
}

static cell_t ws_WriteBinary(IPluginContext *pContext, const cell_t *params)
{
    WebSocketClient *pWebSocketClient = GetWsPointer(pContext, params[1]);

    if (!pWebSocketClient) return 0;

    const cell_t length = params[3];

    if (length < 0) 
    {
        return pContext->ThrowNativeError(
            "Binary message length cannot be negative"
        );
    }

    char* data = nullptr;

    int error = pContext->LocalToString(params[2], &data);

    if (error != SP_ERROR_NONE)
    {
        return pContext->ThrowNativeError(
            "Could not access binary message buffer"
        );
    }

    std::string payload(data, static_cast<size_t>(length));

    auto result = pWebSocketClient->m_webSocket->send(
        payload,
        true
    );

    return result.success ? 1 : 0;
}

static cell_t ws_WriteString(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient) return 0;

	char *msg;
	pContext->LocalToString(params[2], &msg);

	auto result = pWebSocketClient->m_webSocket->send(msg);

	return result.success ? 1 : 0;
}

static cell_t ws_WriteJSON(IPluginContext *pContext, const cell_t *params)
{
	IJsonManager* pJsonManager = g_WebsocketExt.GetJsonManager();
	if (!pJsonManager)
	{
		return pContext->ThrowNativeError("JSON extension not loaded");
	}

	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	JsonValue* pJsonValue = pJsonManager->GetValueFromHandle(pContext, params[2]);

	if (!pWebSocketClient || !pJsonValue) return 0;

	uint32_t write_flg = static_cast<uint32_t>(params[3]);
	char* json_str = pJsonManager->WriteToStringPtr(pJsonValue, write_flg);

	if (!json_str)
	{
		return pContext->ThrowNativeError("Failed to serialize JSON to string");
	}

	auto result = pWebSocketClient->m_webSocket->send(json_str);
	free(json_str);

	return result.success ? 1 : 0;
}

static cell_t ws_AutoReconnect(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient) return 0;

	if (params[0] == 2) {
		pWebSocketClient->m_webSocket->setAutomaticReconnection(params[2] != 0);
		return 1;
	}

	return pWebSocketClient->m_webSocket->isAutomaticReconnectionEnabled();
}

static cell_t ws_MinReconnectWait(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient) return 0;

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

	if (!pWebSocketClient) return 0;

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

static cell_t ws_HandshakeTimeout(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient) return 0;

	if (params[0] == 2) {
		pWebSocketClient->m_webSocket->setHandshakeTimeout(params[2]);
		return 1;
	}

	return pWebSocketClient->m_webSocket->getHandshakeTimeout();
}

static cell_t ws_PingInterval(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);

	if (!pWebSocketClient) return 0;

	if (params[0] == 2) {
		pWebSocketClient->m_webSocket->setPingInterval(params[2]);
		return 1;
	}

	return pWebSocketClient->m_webSocket->getPingInterval();
}

static cell_t ws_SetTLSCertAndKey(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	char *certFile, *keyFile;
	pContext->LocalToString(params[2], &certFile);
	pContext->LocalToString(params[3], &keyFile);

	char certAbsolutePath[PLATFORM_MAX_PATH];
	char keyAbsolutePath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, certAbsolutePath, sizeof(certAbsolutePath), "%s", certFile);
	smutils->BuildPath(Path_Game, keyAbsolutePath, sizeof(keyAbsolutePath), "%s", keyFile);

	pWebSocketClient->m_tlsOptions.certFile = certAbsolutePath;
	pWebSocketClient->m_tlsOptions.keyFile = keyAbsolutePath;

	if (!pWebSocketClient->m_tlsOptions.isValid())
	{
		return pContext->ThrowNativeError("TLS configuration error: %s",
			pWebSocketClient->m_tlsOptions.getErrorMsg().c_str());
	}

	pWebSocketClient->m_webSocket->setTLSOptions(pWebSocketClient->m_tlsOptions);

	return 1;
}

static cell_t ws_SetTLSCAFile(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	char *caFile;
	pContext->LocalToString(params[2], &caFile);

	if (strcmp(caFile, "SYSTEM") == 0 || strcmp(caFile, "NONE") == 0)
	{
		pWebSocketClient->m_tlsOptions.caFile = caFile;
	}
	else
	{
		char absolutePath[PLATFORM_MAX_PATH];
		smutils->BuildPath(Path_Game, absolutePath, sizeof(absolutePath), "%s", caFile);
		pWebSocketClient->m_tlsOptions.caFile = absolutePath;
	}

	if (!pWebSocketClient->m_tlsOptions.isValid())
	{
		return pContext->ThrowNativeError("TLS configuration error: %s",
			pWebSocketClient->m_tlsOptions.getErrorMsg().c_str());
	}

	pWebSocketClient->m_webSocket->setTLSOptions(pWebSocketClient->m_tlsOptions);

	return 1;
}

static cell_t ws_SetTLSCiphers(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	char *ciphers;
	pContext->LocalToString(params[2], &ciphers);
	pWebSocketClient->m_tlsOptions.ciphers = ciphers;
	pWebSocketClient->m_webSocket->setTLSOptions(pWebSocketClient->m_tlsOptions);

	return 1;
}

static cell_t ws_TLSEnabled(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	if (params[0] == 2) {
		pWebSocketClient->m_tlsOptions.tls = params[2] != 0;
		pWebSocketClient->m_webSocket->setTLSOptions(pWebSocketClient->m_tlsOptions);
		return 1;
	}

	return pWebSocketClient->m_tlsOptions.tls;
}

static cell_t ws_HostnameValidation(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	if (params[0] == 2) {
		pWebSocketClient->m_tlsOptions.disable_hostname_validation = params[2] == 0;
		pWebSocketClient->m_webSocket->setTLSOptions(pWebSocketClient->m_tlsOptions);
		return 1;
	}

	return !pWebSocketClient->m_tlsOptions.disable_hostname_validation;
}

static cell_t ws_PingTimeout(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	if (params[0] == 2) {
		ix::WebSocketTimeouts timeouts = pWebSocketClient->m_webSocket->getTimeouts();
		timeouts.pingTimeoutSecs = params[2];
		pWebSocketClient->m_webSocket->setTimeouts(timeouts);
		return 1;
	}

	return pWebSocketClient->m_webSocket->getTimeouts().pingTimeoutSecs;
}

static cell_t ws_IdleTimeout(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	if (params[0] == 2) {
		ix::WebSocketTimeouts timeouts = pWebSocketClient->m_webSocket->getTimeouts();
		timeouts.idleTimeoutSecs = params[2];
		pWebSocketClient->m_webSocket->setTimeouts(timeouts);
		return 1;
	}

	return pWebSocketClient->m_webSocket->getTimeouts().idleTimeoutSecs;
}

static cell_t ws_SendTimeout(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	if (params[0] == 2) {
		ix::WebSocketTimeouts timeouts = pWebSocketClient->m_webSocket->getTimeouts();
		timeouts.sendTimeoutSecs = params[2];
		pWebSocketClient->m_webSocket->setTimeouts(timeouts);
		return 1;
	}

	return pWebSocketClient->m_webSocket->getTimeouts().sendTimeoutSecs;
}

static cell_t ws_CloseTimeout(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	if (params[0] == 2) {
		ix::WebSocketTimeouts timeouts = pWebSocketClient->m_webSocket->getTimeouts();
		timeouts.closeTimeoutSecs = params[2];
		pWebSocketClient->m_webSocket->setTimeouts(timeouts);
		return 1;
	}

	return pWebSocketClient->m_webSocket->getTimeouts().closeTimeoutSecs;
}

static cell_t ws_MessagesSent(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	return static_cast<cell_t>(pWebSocketClient->m_webSocket->getStats().messagesSent.load());
}

static cell_t ws_MessagesReceived(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	return static_cast<cell_t>(pWebSocketClient->m_webSocket->getStats().messagesReceived.load());
}

static cell_t ws_BytesSent(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	return static_cast<cell_t>(pWebSocketClient->m_webSocket->getStats().bytesSent.load());
}

static cell_t ws_BytesReceived(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	return static_cast<cell_t>(pWebSocketClient->m_webSocket->getStats().bytesReceived.load());
}

static cell_t ws_ConnectionDuration(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	return static_cast<cell_t>(pWebSocketClient->m_webSocket->getStats().connectionDurationSecs());
}

static cell_t ws_PingsSent(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	return static_cast<cell_t>(pWebSocketClient->m_webSocket->getStats().pingsSent.load());
}

static cell_t ws_PongsSent(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	return static_cast<cell_t>(pWebSocketClient->m_webSocket->getStats().pongsSent.load());
}

static cell_t ws_PingsReceived(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	return static_cast<cell_t>(pWebSocketClient->m_webSocket->getStats().pingsReceived.load());
}

static cell_t ws_PongsReceived(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	return static_cast<cell_t>(pWebSocketClient->m_webSocket->getStats().pongsReceived.load());
}

static cell_t ws_BufferedAmount(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	return static_cast<cell_t>(pWebSocketClient->m_webSocket->bufferedAmount());
}

static cell_t ws_ResetStats(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	pWebSocketClient->m_webSocket->resetStats();
	return 1;
}

static cell_t ws_AddSubProtocol(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	char *protocol;
	pContext->LocalToString(params[2], &protocol);
	pWebSocketClient->m_webSocket->addSubProtocol(protocol);

	return 1;
}

static cell_t ws_RemoveSubProtocol(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	char *protocol;
	pContext->LocalToString(params[2], &protocol);
	pWebSocketClient->m_webSocket->removeSubProtocol(protocol);

	return 1;
}

static cell_t ws_ClearSubProtocols(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	pWebSocketClient->m_webSocket->clearSubProtocols();
	return 1;
}

static cell_t ws_SetProxy(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	char *proxyUrl;
	pContext->LocalToString(params[2], &proxyUrl);

	pWebSocketClient->m_proxyConfig = ix::ProxyConfig::fromUrl(proxyUrl);
	pWebSocketClient->m_webSocket->setProxyConfig(pWebSocketClient->m_proxyConfig);
	return 1;
}

static cell_t ws_ClearProxy(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	pWebSocketClient->m_proxyConfig = ix::ProxyConfig();
	pWebSocketClient->m_webSocket->setProxyConfig(pWebSocketClient->m_proxyConfig);
	return 1;
}

static cell_t ws_HasProxy(IPluginContext *pContext, const cell_t *params)
{
	WebSocketClient* pWebSocketClient = GetWsPointer(pContext, params[1]);
	if (!pWebSocketClient) return 0;

	return pWebSocketClient->m_proxyConfig.isEnabled();
}

static cell_t ws_SetUserAgent(IPluginContext *pContext, const cell_t *params)
{
	char *userAgent;
	pContext->LocalToString(params[1], &userAgent);
	ix::setUserAgent(userAgent);
	return 1;
}

static cell_t ws_GetUserAgent(IPluginContext *pContext, const cell_t *params)
{
	const std::string& userAgent = ix::getCustomUserAgent();
	pContext->StringToLocalUTF8(params[1], params[2], userAgent.c_str(), nullptr);
	return 1;
}

static cell_t ws_SetServerHeader(IPluginContext *pContext, const cell_t *params)
{
	char *server;
	pContext->LocalToString(params[1], &server);
	ix::setServerHeader(server);
	return 1;
}

static cell_t ws_GetServerHeader(IPluginContext *pContext, const cell_t *params)
{
	const std::string& server = ix::getCustomServerHeader();
	pContext->StringToLocalUTF8(params[1], params[2], server.c_str(), nullptr);
	return 1;
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
	{"WebSocket.WriteBinary",            ws_WriteBinary},
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
	{"WebSocket.HandshakeTimeout.get",   ws_HandshakeTimeout},
	{"WebSocket.HandshakeTimeout.set",   ws_HandshakeTimeout},
	{"WebSocket.PingInterval.get",       ws_PingInterval},
	{"WebSocket.PingInterval.set",       ws_PingInterval},
	{"WebSocket.SetTLSCertAndKey",       ws_SetTLSCertAndKey},
	{"WebSocket.SetTLSCAFile",           ws_SetTLSCAFile},
	{"WebSocket.SetTLSCiphers",          ws_SetTLSCiphers},
	{"WebSocket.TLSEnabled.get",         ws_TLSEnabled},
	{"WebSocket.TLSEnabled.set",         ws_TLSEnabled},
	{"WebSocket.HostnameValidation.get", ws_HostnameValidation},
	{"WebSocket.HostnameValidation.set", ws_HostnameValidation},
	{"WebSocket.PingTimeout.get",        ws_PingTimeout},
	{"WebSocket.PingTimeout.set",        ws_PingTimeout},
	{"WebSocket.IdleTimeout.get",        ws_IdleTimeout},
	{"WebSocket.IdleTimeout.set",        ws_IdleTimeout},
	{"WebSocket.SendTimeout.get",        ws_SendTimeout},
	{"WebSocket.SendTimeout.set",        ws_SendTimeout},
	{"WebSocket.CloseTimeout.get",       ws_CloseTimeout},
	{"WebSocket.CloseTimeout.set",       ws_CloseTimeout},
	{"WebSocket.MessagesSent.get",       ws_MessagesSent},
	{"WebSocket.MessagesReceived.get",   ws_MessagesReceived},
	{"WebSocket.BytesSent.get",          ws_BytesSent},
	{"WebSocket.BytesReceived.get",      ws_BytesReceived},
	{"WebSocket.ConnectionDuration.get", ws_ConnectionDuration},
	{"WebSocket.PingsSent.get",          ws_PingsSent},
	{"WebSocket.PongsSent.get",          ws_PongsSent},
	{"WebSocket.PingsReceived.get",      ws_PingsReceived},
	{"WebSocket.PongsReceived.get",      ws_PongsReceived},
	{"WebSocket.BufferedAmount.get",     ws_BufferedAmount},
	{"WebSocket.ResetStats",             ws_ResetStats},
	{"WebSocket.AddSubProtocol",         ws_AddSubProtocol},
	{"WebSocket.RemoveSubProtocol",      ws_RemoveSubProtocol},
	{"WebSocket.ClearSubProtocols",      ws_ClearSubProtocols},
	{"WebSocket.SetProxy",               ws_SetProxy},
	{"WebSocket.ClearProxy",             ws_ClearProxy},
	{"WebSocket.HasProxy.get",           ws_HasProxy},
	// global
	{"WebSocket_SetUserAgent",           ws_SetUserAgent},
	{"WebSocket_GetUserAgent",           ws_GetUserAgent},
	{"WebSocket_SetServerHeader",        ws_SetServerHeader},
	{"WebSocket_GetServerHeader",        ws_GetServerHeader},
	{nullptr, nullptr}
};
