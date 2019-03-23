#include "WebsocketFeed.h"
#include <libwebsockets.h>
#include <openssl/err.h>
#include <iostream>

#include "condition_var.h"
#include "Utility.h"
#include "MarketDataMessage.h"

using namespace std;

static bool stop =false;
static char m_order_id[32];
static char m_uuid[32] = "";

static const char* to_string(lws_callback_reasons reason)
{
    switch(reason)
    {
	case LWS_CALLBACK_ESTABLISHED: return "LWS_CALLBACK_ESTABLISHED";
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: return "LWS_CALLBACK_CLIENT_CONNECTION_ERROR";
	case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH: return "LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH";
	case LWS_CALLBACK_CLIENT_ESTABLISHED: return "LWS_CALLBACK_CLIENT_ESTABLISHED";
	case LWS_CALLBACK_CLOSED: return "LWS_CALLBACK_CLOSED";
	case LWS_CALLBACK_CLOSED_HTTP: return "LWS_CALLBACK_CLOSED_HTTP";
	case LWS_CALLBACK_RECEIVE: return "LWS_CALLBACK_RECEIVE";
	case LWS_CALLBACK_RECEIVE_PONG: return "LWS_CALLBACK_RECEIVE_PONG";
	case LWS_CALLBACK_CLIENT_RECEIVE: return "LWS_CALLBACK_CLIENT_RECEIVE";
	case LWS_CALLBACK_CLIENT_RECEIVE_PONG: return "LWS_CALLBACK_CLIENT_RECEIVE_PONG";
	case LWS_CALLBACK_CLIENT_WRITEABLE: return "LWS_CALLBACK_CLIENT_WRITEABLE";
	case LWS_CALLBACK_SERVER_WRITEABLE: return "LWS_CALLBACK_SERVER_WRITEABLE";
	case LWS_CALLBACK_HTTP: return "LWS_CALLBACK_HTTP";
	case LWS_CALLBACK_HTTP_BODY: return "LWS_CALLBACK_HTTP_BODY";
	case LWS_CALLBACK_HTTP_BODY_COMPLETION: return "LWS_CALLBACK_HTTP_BODY_COMPLETION";
	case LWS_CALLBACK_HTTP_FILE_COMPLETION: return "LWS_CALLBACK_HTTP_FILE_COMPLETION";
	case LWS_CALLBACK_HTTP_WRITEABLE: return "LWS_CALLBACK_HTTP_WRITEABLE";
	case LWS_CALLBACK_FILTER_NETWORK_CONNECTION: return "LWS_CALLBACK_FILTER_NETWORK_CONNECTION";
	case LWS_CALLBACK_FILTER_HTTP_CONNECTION: return "LWS_CALLBACK_FILTER_HTTP_CONNECTION";
	case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED: return "LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED";
	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: return "LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION";
	case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS: return "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS";
	case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS: return "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS";
	case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION: return "LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION";
	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: return "LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER";
	case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY: return "LWS_CALLBACK_CONFIRM_EXTENSION_OKAY";
	case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED: return "LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED";
	case LWS_CALLBACK_PROTOCOL_INIT: return "LWS_CALLBACK_PROTOCOL_INIT";
	case LWS_CALLBACK_PROTOCOL_DESTROY: return "LWS_CALLBACK_PROTOCOL_DESTROY";
	case LWS_CALLBACK_WSI_CREATE: return "LWS_CALLBACK_WSI_CREATE";
	case LWS_CALLBACK_WSI_DESTROY: return "LWS_CALLBACK_WSI_DESTROY";
	case LWS_CALLBACK_GET_THREAD_ID: return "LWS_CALLBACK_GET_THREAD_ID";
	case LWS_CALLBACK_ADD_POLL_FD: return "LWS_CALLBACK_ADD_POLL_FD";
	case LWS_CALLBACK_DEL_POLL_FD: return "LWS_CALLBACK_DEL_POLL_FD";
	case LWS_CALLBACK_CHANGE_MODE_POLL_FD: return "LWS_CALLBACK_CHANGE_MODE_POLL_FD";
	case LWS_CALLBACK_LOCK_POLL: return "LWS_CALLBACK_LOCK_POLL";
	case LWS_CALLBACK_UNLOCK_POLL: return "LWS_CALLBACK_UNLOCK_POLL";
	case LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY: return "LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY";
	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: return "LWS_CALLBACK_WS_PEER_INITIATED_CLOSE";
	case LWS_CALLBACK_WS_EXT_DEFAULTS: return "LWS_CALLBACK_WS_EXT_DEFAULTS";
	case LWS_CALLBACK_CGI: return "LWS_CALLBACK_CGI";
	case LWS_CALLBACK_CGI_TERMINATED: return "LWS_CALLBACK_CGI_TERMINATED";
	case LWS_CALLBACK_CGI_STDIN_DATA: return "LWS_CALLBACK_CGI_STDIN_DATA";
	case LWS_CALLBACK_CGI_STDIN_COMPLETED: return "LWS_CALLBACK_CGI_STDIN_COMPLETED";
	case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP: return "LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP";
	case LWS_CALLBACK_CLOSED_CLIENT_HTTP: return "LWS_CALLBACK_CLOSED_CLIENT_HTTP";
	case LWS_CALLBACK_RECEIVE_CLIENT_HTTP: return "LWS_CALLBACK_RECEIVE_CLIENT_HTTP";
	case LWS_CALLBACK_COMPLETED_CLIENT_HTTP: return "LWS_CALLBACK_COMPLETED_CLIENT_HTTP";
	case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ: return "LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ";
	case LWS_CALLBACK_HTTP_BIND_PROTOCOL: return "LWS_CALLBACK_HTTP_BIND_PROTOCOL";
	case LWS_CALLBACK_HTTP_DROP_PROTOCOL: return "LWS_CALLBACK_HTTP_DROP_PROTOCOL";
	case LWS_CALLBACK_CHECK_ACCESS_RIGHTS: return "LWS_CALLBACK_CHECK_ACCESS_RIGHTS";
	case LWS_CALLBACK_PROCESS_HTML: return "LWS_CALLBACK_PROCESS_HTML";
	case LWS_CALLBACK_ADD_HEADERS: return "LWS_CALLBACK_ADD_HEADERS";
	case LWS_CALLBACK_SESSION_INFO: return "LWS_CALLBACK_SESSION_INFO";
	case LWS_CALLBACK_GS_EVENT: return "LWS_CALLBACK_GS_EVENT";
	case LWS_CALLBACK_HTTP_PMO: return "LWS_CALLBACK_HTTP_PMO";
	case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE: return "LWS_CALLBACK_CLIENT_HTTP_WRITEABLE";
	case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION: return "LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION";
	case LWS_CALLBACK_RAW_RX: return "LWS_CALLBACK_RAW_RX";
	case LWS_CALLBACK_RAW_CLOSE: return "LWS_CALLBACK_RAW_CLOSE";
	case LWS_CALLBACK_RAW_WRITEABLE: return "LWS_CALLBACK_RAW_WRITEABLE";
	case LWS_CALLBACK_RAW_ADOPT: return "LWS_CALLBACK_RAW_ADOPT";
	case LWS_CALLBACK_RAW_ADOPT_FILE: return "LWS_CALLBACK_RAW_ADOPT_FILE";
	case LWS_CALLBACK_RAW_RX_FILE: return "LWS_CALLBACK_RAW_RX_FILE";
	case LWS_CALLBACK_RAW_WRITEABLE_FILE: return "LWS_CALLBACK_RAW_WRITEABLE_FILE";
	case LWS_CALLBACK_RAW_CLOSE_FILE: return "LWS_CALLBACK_RAW_CLOSE_FILE";
	case LWS_CALLBACK_SSL_INFO: return "LWS_CALLBACK_SSL_INFO";
	case LWS_CALLBACK_CHILD_WRITE_VIA_PARENT: return "LWS_CALLBACK_CHILD_WRITE_VIA_PARENT";
	case LWS_CALLBACK_CHILD_CLOSING: return "LWS_CALLBACK_CHILD_CLOSING";
	case LWS_CALLBACK_CGI_PROCESS_ATTACH: return "LWS_CALLBACK_CGI_PROCESS_ATTACH";
	case LWS_CALLBACK_EVENT_WAIT_CANCELLED: return "LWS_CALLBACK_EVENT_WAIT_CANCELLED";
	case LWS_CALLBACK_VHOST_CERT_AGING: return "LWS_CALLBACK_VHOST_CERT_AGING";
	case LWS_CALLBACK_VHOST_CERT_UPDATE: return "LWS_CALLBACK_VHOST_CERT_UPDATE";
	case LWS_CALLBACK_USER: return "LWS_CALLBACK_USER";
    default: return "UKNOWN";
    }
}

static const struct lws_extension EXTENSIONS[] = 
{
    {
        "permessage-deflate"
    ,   lws_extension_callback_pm_deflate
    ,   "permessage-deflate; client_no_context_message"
    }
,   {
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	}
,   { NULL, NULL, NULL /* terminator */ }
};

static const char* SUBSCRIBE_TEMPLATE = 
"{\n"
"   \"type\" : \"subscribe\"\n"
",  \"product_ids\" : [%s\n]"
",  \"channels\" : [ \"full\" ]\n"
"}";

void WebsocketFeed::SubscribeSymbols(lws* wsi)
{
    char symbols[1024];
    memset(symbols, 0, sizeof(symbols));

    bool first = true;
    vector<string> _clients = {"BTC-USD"};  
    for(const auto& e : _clients)
    {
        if( !first )
        {
            strcat(symbols, ",");
        }
        strcat(symbols, "\"");
        strcat(symbols, e.c_str());
        strcat(symbols, "\"");
        first = false;
    }

    char message[2048];
    snprintf(message, sizeof(message), SUBSCRIBE_TEMPLATE, symbols);

    std::cout << GetNowStr() << ":  Subscribing to symbols " << symbols << endl
              << "\tWith message " << message << endl;

    lws_write(wsi, (unsigned char*)message, strlen(message), LWS_WRITE_TEXT);
}

void WebsocketFeed::OnMessage(int64_t now, const char* message, int length)
{
    char temp[4096];
    memcpy(temp, message, length);
    temp[length] = 0;

    ValueType value;
    value.Parse(temp);

    OrderMessage m;
    if(!m.FromJson(value)) 
	return;

    //m.receive_time = now; 
    
    if((int)m.type == 68 && (int)m.reason == 70 && m.price > 0){
	//cout << "BTC-USD: " << m.price << " order_id" << m.order_id << endl;
	if(m.price > Conditions::price && !stop){ 
		cout << "price breached at: " << m.exchange_time << endl;        
		lock_guard<std::mutex> lk(RegUtil::mu);
		RegUtil::breached = true;
		RegUtil::cv.notify_one();
		stop = true;
		copy_order_id(m_uuid, OrderInfo::uuid);			
	}

	if(stop && m_uuid[0] == ' '){
		cout << "my order id is: " << m_order_id << endl;	
		cout << m.order_id << endl;
		cout << strcmp(m.order_id, m_order_id) << endl;
		if(strcmp(m.order_id, m_order_id) == 0)
			cout << "order executed at: " << m.exchange_time << endl;
	}	 		
	
    }
    
    	
    if(stop && (int)m.type == 82 && m.client_oid[0] != ' ' && m_uuid[0] != ' '){
	cout << "my uuid: " << m_uuid << endl;	
	cout << m.client_oid << endl;
	if(strcmp(m_uuid, m.client_oid) == -82){
		copy_order_id(m_order_id, m.order_id);	
		m_uuid[0] = ' ';
	}
	
    }

    

    
}

int WebsocketFeed::WebsocketCallback(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t length)
{
    WebsocketFeed* pthis = (WebsocketFeed*)user;
    int64_t now = GetNow();
    switch(reason)
    {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        cout << GetNowStr() << ": Connected" << endl;
        lws_callback_on_writable(wsi);
        pthis->SubscribeSymbols(wsi);
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
        pthis->OnMessage(now, (const char*)in, length);
        break;
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        cerr << GetNowStr() << ": Error Connecting ";
        if( in )
            cerr.write( (const char*)in, length );
        cerr << endl;
        pthis->_running = false;
    case LWS_CALLBACK_WSI_DESTROY:
    case LWS_CALLBACK_CLOSED:
        cerr << GetNowStr() << ": Disconnected" << endl;
        pthis->_connected = false;
        break;
    default:
        //cerr << GetNowStr() << ": Received unknown reason " << (int)reason << " - " << to_string(reason) << endl;
        break;
    }

    return 0;
}

/*
void WebsocketFeed::AddClient(const std::string& symbol, WebsocketFeedClient* client)
{
    _clients[symbol].push_back(client);
}
*/
void WebsocketFeed::Run(const std::string& host, int port)
{
    static lws_protocols PROTOCOLS[] = 
    {
        {    "client-protocol"
        ,   &WebsocketFeed::WebsocketCallback
        ,   0
        ,   4096
        }
    ,   { NULL, NULL, 0, 0 }
    };

    _connected = false;
    _running = true;

    lws_context_creation_info info = {0};
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = PROTOCOLS;
    info.gid = -1;
    info.uid = -1;
    info.extensions = EXTENSIONS;
    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

    lws_context* context = lws_create_context(&info);

    while(_running)
    {
        if( !_connected )
        {
            cout << GetNowStr() << ": Connecting to " << host << ":" << port << endl;

            lws_client_connect_info cinfo = {0};
            cinfo.context = context;
            cinfo.host = strdup(host.c_str());
            cinfo.address = strdup(host.c_str());
            cinfo.origin = strdup(host.c_str());
            cinfo.port = port;
            cinfo.path = "/";
            cinfo.ssl_connection = LCCSCF_USE_SSL;

            cinfo.protocol = PROTOCOLS[0].name;
            cinfo.ietf_version_or_minus_one = -1;
            cinfo.userdata = this;

            lws* socket = lws_client_connect_via_info(&cinfo);
            if( !socket )
            {
                return;
            }

            _connected = true;
        }

        lws_service(context, 500);
    }
}

