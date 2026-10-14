#include "ws_client.h"

WebSocketClient::WebSocketClient(ix::WebSocket* existingSocket) : m_webSocket(existingSocket) {}

WebSocketClient::WebSocketClient(const char* url, uint8_t type)
{
	m_webSocket = new ix::WebSocket();
	m_webSocket->setUrl(url);
	m_webSocket->setAutomaticReconnection(false);
	m_callback_type = type;

	m_webSocket->setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
		switch (msg->type)
		{
			case ix::WebSocketMessageType::Message:
			{
				OnMessage(msg->str);
				break;
			}
			case ix::WebSocketMessageType::Open:
			{
				{
					std::lock_guard<std::mutex> lock(m_headersMutex);
					m_headers = msg->openInfo.headers;
				}
				OnOpen(msg->openInfo);
				break;
			}
			case ix::WebSocketMessageType::Close:
			{
				OnClose(msg->closeInfo);
				break;
			}
			case ix::WebSocketMessageType::Error:
			{
				OnError(msg->errorInfo);
				break;
			}
			case ix::WebSocketMessageType::Ping:
			case ix::WebSocketMessageType::Pong:
			case ix::WebSocketMessageType::Fragment:
				break;
		}
	});
}

WebSocketClient::~WebSocketClient()
{
	if (!m_keepConnecting) {
		m_webSocket->stop();
		delete m_webSocket;
	}

	// Drain the queue for tasks belonging to this client
	ITaskContext* context = nullptr;
	std::vector<ITaskContext*> other_tasks;

	while (g_TaskQueue.TryPop(context)) {
		if (context && context->GetOwner() == this) {
			context->OnCompleted();
			delete context;
		} else if (context) {
			other_tasks.push_back(context);
		}
	}

	for (auto task : other_tasks) {
		g_TaskQueue.Push(task);
	}

	if (pMessageForward) forwards->ReleaseForward(pMessageForward);
	if (pOpenForward) forwards->ReleaseForward(pOpenForward);
	if (pCloseForward) forwards->ReleaseForward(pCloseForward);
	if (pErrorForward) forwards->ReleaseForward(pErrorForward);
}

bool WebSocketClient::IsConnected()
{
	return m_webSocket->getReadyState() == ix::ReadyState::Open;
}

void WebSocketClient::OnMessage(const std::string& message)
{
	if (!pMessageForward || !pMessageForward->GetFunctionCount())
	{
		return;
	}

	WsMessageTaskContext *context = new WsMessageTaskContext(this, message);
	g_TaskQueue.Push(context);
}

void WebSocketClient::OnOpen(ix::WebSocketOpenInfo openInfo)
{
	if (!pOpenForward || !pOpenForward->GetFunctionCount())
	{
		return;
	}

	WsOpenTaskContext *context = new WsOpenTaskContext(this, openInfo);
	g_TaskQueue.Push(context);
}

void WebSocketClient::OnClose(ix::WebSocketCloseInfo closeInfo)
{
	if (!pCloseForward || !pCloseForward->GetFunctionCount())
	{
		return;
	}

	WsCloseTaskContext *context = new WsCloseTaskContext(this, closeInfo);
	g_TaskQueue.Push(context);
}

void WebSocketClient::OnError(ix::WebSocketErrorInfo errorInfo)
{
	if (!pErrorForward || !pErrorForward->GetFunctionCount())
	{
		return;
	}

	WsErrorTaskContext *context = new WsErrorTaskContext(this, errorInfo);
	g_TaskQueue.Push(context);
}

void WsMessageTaskContext::OnCompleted()
{
	const size_t messageLength = m_message.length();

	switch (m_client->m_callback_type)
	{
		case WebSocket_STRING:
		{
			m_client->pMessageForward->PushCell(m_client->m_websocket_handle);
			m_client->pMessageForward->PushString(m_message.c_str());
			m_client->pMessageForward->PushCell(messageLength + 1);
			m_client->pMessageForward->Execute(nullptr);
			break;
		}
		case WebSocket_JSON:
		{
			IJsonManager* pJsonManager = g_WebsocketExt.GetJsonManager();
			if (!pJsonManager)
			{
				smutils->LogError(myself, "JSON extension not loaded, cannot process JSON message");
				return;
			}

			char error[JSON_ERROR_BUFFER_SIZE];
			JsonValue* pJsonValue = pJsonManager->ParseJSON(m_message.c_str(), false, false, 0, error, sizeof(error));

			if (!pJsonValue)
			{
				smutils->LogError(myself, "parse JSON message error: %s", error);
				return;
			}

			HandleError err;
			HandleSecurity pSec(myself->GetIdentity(), myself->GetIdentity());
			Handle_t jsonHandle = handlesys->CreateHandleEx(pJsonManager->GetJsonHandleType(), pJsonValue, &pSec, nullptr, &err);

			if (!jsonHandle)
			{
				pJsonManager->Release(pJsonValue);
				smutils->LogError(myself, "Could not create JSON handle (error %d)", err);
				return;
			}

			m_client->pMessageForward->PushCell(m_client->m_websocket_handle);
			m_client->pMessageForward->PushCell(jsonHandle);
			m_client->pMessageForward->PushCell(messageLength + 1);
			m_client->pMessageForward->Execute(nullptr);

			handlesys->FreeHandle(jsonHandle, &pSec);
			break;
		}
		case WebSocket_BINARY:
		{
			std::vector<cell_t> bytes;
			bytes.reserve(m_message.size());

			for (unsigned char byte : m_message)
			{
				bytes.push_back(static_cast<cell_t>(byte));
			}

			m_client->pMessageForward->PushCell(m_client->m_websocket_handle);

			cell_t empty = 0;
			const unsigned int length = static_cast<unsigned int>(bytes.size());

			m_client->pMessageForward->PushArray(
				bytes.empty() ? &empty : bytes.data(),
				length
			);

			m_client->pMessageForward->PushCell(static_cast<cell_t>(length));

			m_client->pMessageForward->Execute(nullptr);
			break;
		}
	}
}

void WsOpenTaskContext::OnCompleted()
{
	m_client->pOpenForward->PushCell(m_client->m_websocket_handle);
	m_client->pOpenForward->Execute(nullptr);
}

void WsCloseTaskContext::OnCompleted()
{
	m_client->pCloseForward->PushCell(m_client->m_websocket_handle);
	m_client->pCloseForward->PushCell(m_closeInfo.code);
	m_client->pCloseForward->PushString(m_closeInfo.reason.c_str());
	m_client->pCloseForward->Execute(nullptr);
}

void WsErrorTaskContext::OnCompleted()
{
	m_client->pErrorForward->PushCell(m_client->m_websocket_handle);
	m_client->pErrorForward->PushString(m_errorInfo.reason.c_str());
	m_client->pErrorForward->Execute(nullptr);
}
