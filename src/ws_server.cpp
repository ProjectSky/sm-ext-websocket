#include "extension.h"

WebSocketServer::WebSocketServer(const std::string& host, int port, int addressFamily, int pingInterval) : m_webSocketServer(
	port, 
	host,
	ix::SocketServer::kDefaultTcpBacklog,
	ix::SocketServer::kDefaultMaxConnections,
	ix::WebSocketServer::kDefaultHandShakeTimeoutSecs,
	addressFamily,
	pingInterval)
{
	this->m_webSocketServer.setOnClientMessageCallback([this](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg) {
		switch (msg->type)
		{
			case ix::WebSocketMessageType::Open:
			{
				this->OnOpen(msg->openInfo, connectionState);
				break;
			}
			case ix::WebSocketMessageType::Message:
			{
				this->OnMessage(msg->str, connectionState, &webSocket);
				break;
			}
			case ix::WebSocketMessageType::Close:
			{
				this->OnClose(msg->closeInfo, connectionState);
				break;
			}
			case ix::WebSocketMessageType::Error:
			{
				this->OnError(msg->errorInfo, connectionState);
				break;
			}
		}
	});
}

WebSocketServer::~WebSocketServer() 
{
	this->m_webSocketServer.stop();
}

void WebSocketServer::OnMessage(const std::string& message, std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket* client) 
{
	if (!this->pMessageForward || !this->pMessageForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, message, connectionState, client]()
	{
		HandleError err;
		HandleSecurity sec(NULL, myself->GetIdentity());

		WebSocketClient* pWebSocketClient = new WebSocketClient(client);

		pWebSocketClient->m_websocket_handle = handlesys->CreateHandleEx(g_htWsClient, pWebSocketClient, &sec, NULL, &err);
		if (!pWebSocketClient->m_websocket_handle) return;
		pWebSocketClient->m_keepConnecting = true;

		std::string remoteAddress = connectionState->getRemoteIp() + ":" + std::to_string(connectionState->getRemotePort());
		this->pMessageForward->PushCell(this->m_webSocketServer_handle);
		this->pMessageForward->PushCell(pWebSocketClient->m_websocket_handle);
		this->pMessageForward->PushString(message.c_str());
		this->pMessageForward->PushCell(message.length());
		this->pMessageForward->PushString(remoteAddress.c_str());
		this->pMessageForward->PushString(connectionState->getId().c_str());
		this->pMessageForward->Execute(NULL);
		
		handlesys->FreeHandle(pWebSocketClient->m_websocket_handle, NULL);
	});
}

void WebSocketServer::OnOpen(ix::WebSocketOpenInfo openInfo, std::shared_ptr<ix::ConnectionState> connectionState) 
{
	if (!this->pOpenForward || !this->pOpenForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, openInfo, connectionState]()
	{
		std::string remoteAddress = connectionState->getRemoteIp() + ":" + std::to_string(connectionState->getRemotePort());
		this->pOpenForward->PushCell(this->m_webSocketServer_handle);
		this->pOpenForward->PushString(remoteAddress.c_str());
		this->pOpenForward->PushString(connectionState->getId().c_str());
		this->pOpenForward->Execute(NULL);
	});
}

void WebSocketServer::OnClose(ix::WebSocketCloseInfo closeInfo, std::shared_ptr<ix::ConnectionState> connectionState) 
{
	if (!this->pCloseForward || !this->pCloseForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, closeInfo, connectionState]()
	{
		std::string remoteAddress = connectionState->getRemoteIp() + ":" + std::to_string(connectionState->getRemotePort());
		this->pCloseForward->PushCell(this->m_webSocketServer_handle);
		this->pCloseForward->PushCell(closeInfo.code);
		this->pCloseForward->PushString(closeInfo.reason.c_str());
		this->pCloseForward->PushString(remoteAddress.c_str());
		this->pCloseForward->PushString(connectionState->getId().c_str());
		this->pCloseForward->Execute(NULL);
	});
}

void WebSocketServer::OnError(ix::WebSocketErrorInfo errorInfo, std::shared_ptr<ix::ConnectionState> connectionState) 
{
	if (!this->pErrorForward || !this->pErrorForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, errorInfo, connectionState]()
	{
		std::string remoteAddress = connectionState->getRemoteIp() + ":" + std::to_string(connectionState->getRemotePort());
		this->pErrorForward->PushCell(this->m_webSocketServer_handle);
		this->pErrorForward->PushString(errorInfo.reason.c_str());
		this->pErrorForward->PushString(remoteAddress.c_str());
		this->pErrorForward->PushString(connectionState->getId().c_str());
		this->pErrorForward->Execute(NULL);
	});
}

void WebSocketServer::broadcastMessage(const std::string& message) {
	auto clients = this->m_webSocketServer.getClients();

	for (const auto& client : clients)
	{
		client.first->send(message);
	} 
}

bool WebSocketServer::sendToClient(const std::string& clientId, const std::string& message) {
	auto clients = this->m_webSocketServer.getClients();
	
	for (const auto& client : clients)
	{
		if (client.second == clientId) {
			client.first->send(message);
			return true;
		}
	}
	return false;
}