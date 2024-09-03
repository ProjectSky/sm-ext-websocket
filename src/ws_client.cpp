#include "extension.h"

WebSocketClient::WebSocketClient(ix::WebSocket* existingSocket) : m_webSocket(existingSocket) {}

WebSocketClient::WebSocketClient(const char* url, uint8_t type) 
{
	this->m_webSocket = new ix::WebSocket();
	this->m_webSocket->setUrl(url);
	this->m_webSocket->disableAutomaticReconnection();
	this->m_callback_type = type;

	this->m_webSocket->setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
		switch (msg->type)
		{
			case ix::WebSocketMessageType::Message:
			{
				this->OnMessage(msg->str);
				break;
			}
			case ix::WebSocketMessageType::Open:
			{
				this->m_headers = msg->openInfo.headers;
				this->OnOpen(msg->openInfo);
				break;
			}
			case ix::WebSocketMessageType::Close:
			{
				this->OnClose(msg->closeInfo);
				break;
			}
			case ix::WebSocketMessageType::Error:
			{
				this->OnError(msg->errorInfo);
				break;
			}
		}
	});
}

WebSocketClient::~WebSocketClient() 
{
	if (!this->m_keepConnecting)
		this->m_webSocket->stop();
}

bool WebSocketClient::IsConnected()
{
	return this->m_webSocket->getReadyState() == ix::ReadyState::Open;
}

void WebSocketClient::OnMessage(const std::string& message) 
{
	if (!this->pMessageForward || !this->pMessageForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, message]()
	{
		const size_t messageLength = message.length() + 1;

		switch (this->m_callback_type)
		{
			case Websocket_STRING:
			{
				this->pMessageForward->PushCell(this->m_websocket_handle);
				this->pMessageForward->PushString(message.c_str());
				this->pMessageForward->PushCell(messageLength);
				this->pMessageForward->Execute(NULL);
				break;
			}
			case WebSocket_JSON:
			{
				YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();

				yyjson_read_err readError;
				yyjson_doc *idoc = yyjson_read_opts(const_cast<char*>(message.c_str()), message.length(), 0, NULL, &readError);

				if (readError.code)
				{
					smutils->LogError(myself, "parse JSON message error (%u): %s at position: %d", readError.code, readError.msg, readError.pos);
					delete pYYJsonWrapper;
					return;
				}

				pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, NULL);
				pYYJsonWrapper->m_pVal_mut = yyjson_mut_doc_get_root(pYYJsonWrapper->m_pDocument_mut);
				yyjson_doc_free(idoc);

				HandleError err;
				HandleSecurity pSec(NULL, myself->GetIdentity());
				this->m_json_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &pSec, NULL, &err);

				if (!this->m_json_handle)
				{
					smutils->LogError(myself, "Could not create JSON handle (error %d)", err);
					delete pYYJsonWrapper;
					return;
				}

				this->pMessageForward->PushCell(this->m_websocket_handle);
				this->pMessageForward->PushCell(this->m_json_handle);
				this->pMessageForward->PushCell(messageLength);
				this->pMessageForward->Execute(NULL);

				handlesys->FreeHandle(this->m_json_handle, &pSec);
				break;
			}
		}
	});
}

void WebSocketClient::OnOpen(ix::WebSocketOpenInfo openInfo) 
{
	if (!this->pOpenForward || !this->pOpenForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, openInfo]()
	{
		this->pOpenForward->PushCell(this->m_websocket_handle);
		this->pOpenForward->Execute(NULL);
	});
}

void WebSocketClient::OnClose(ix::WebSocketCloseInfo closeInfo) 
{
	if (!this->pCloseForward || !this->pCloseForward->GetFunctionCount())
	{
		return;
	}
	
	// TODO: Fixed crash when unload extension after connecting
	// 2024/06/30 - 23:09 - Fixed
	g_TaskQueue.Push([this, closeInfo]()
	{
		this->pCloseForward->PushCell(this->m_websocket_handle);
		this->pCloseForward->PushCell(closeInfo.code);
		this->pCloseForward->PushString(closeInfo.reason.c_str());
		this->pCloseForward->Execute(NULL);
	});
}

void WebSocketClient::OnError(ix::WebSocketErrorInfo errorInfo) 
{
	if (!this->pErrorForward || !this->pErrorForward->GetFunctionCount())
	{
		return;
	}

	g_TaskQueue.Push([this, errorInfo]()
	{
		this->pErrorForward->PushCell(this->m_websocket_handle);
		this->pErrorForward->PushString(errorInfo.reason.c_str());
		this->pErrorForward->Execute(NULL);
	});
}