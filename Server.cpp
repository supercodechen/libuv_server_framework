#include "Server.h"
#include "types.h"
#include <string.h>


CServer::CServer():
	_heart_beat_timer(nullptr),
	_server_handle(nullptr),
	_signal(nullptr)
{
}


CServer::~CServer()
{
	if (_heart_beat_timer)
	{
		uv_timer_stop(_heart_beat_timer);
		uv_close((uv_handle_t *)_heart_beat_timer, [](uv_handle_t *handle) {delete handle; handle = nullptr; });
	}

	if (_signal)
	{
		uv_signal_stop(_signal);
		uv_close((uv_handle_t *)_signal, [](uv_handle_t *handle) {delete handle; handle = nullptr; });
	}

	if (_server_handle)
	{
		uv_close((uv_handle_t *)_server_handle, [](uv_handle_t *handle) {delete handle; handle = nullptr; });
	}
}

bool CServer::init()
{
	_heart_beat_timer = new uv_timer_t();
	_heart_beat_timer->data = this;
	int ret = uv_timer_init(uv_default_loop(), _heart_beat_timer);
	if (ret < 0)
	{
		delete _heart_beat_timer;
		_heart_beat_timer = nullptr;
		return false;
	}

	_signal = new uv_signal_t();
	_signal->data = this;
	ret = uv_signal_init(uv_default_loop(), _signal);
	if (ret < 0)
	{
		delete _heart_beat_timer;
		_heart_beat_timer = nullptr;

		delete _signal;
		_signal = nullptr;
		return false;
	}

	_server_handle = new uv_tcp_t();
	_server_handle->data = this;
	ret = uv_tcp_init(uv_default_loop(), _server_handle);
	if (ret < 0)
	{
		delete _heart_beat_timer;
		_heart_beat_timer = nullptr;

		delete _signal;
		_signal = nullptr;

		delete _server_handle;
		_server_handle = nullptr;
		return false;
	}

	return true;
}

bool CServer::run()
{
	int ret = uv_signal_start(_signal, [](uv_signal_t *handle, int signum) {}, SIGPIPE);
	if (ret < 0)
	{
		return false;
	}

	ret = uv_timer_start(_heart_beat_timer, [](uv_timer_t* handle) {static_cast<CServer *>(handle->data)->on_heart_beat_callback(handle); }, 10 * 1000, 10 * 1000);
	if (ret < 0)
	{
		return false;
	}

	struct sockaddr_in server_addr;
	ret = uv_ip4_addr("0.0.0.0",SERVER_PORT, &server_addr);
	if (ret < 0)
	{
		return false;
	}

	ret = uv_tcp_bind(_server_handle, (const struct sockaddr *)&server_addr, 0);
	if (ret < 0)
	{
		return false;
	}

	ret = uv_listen((uv_stream_t *)_server_handle, 1024, [](uv_stream_t *server_handle, int status) {static_cast<CServer *>(server_handle->data)->on_new_client_connect_callback(server_handle, status); });
	if (ret < 0)
	{
		return false;
	}

	return true;
}

void CServer::on_new_client_connect_callback(uv_stream_t *server_handle, int status)
{
	printf("new connect\n");
	if (status < 0)
	{
		return;
	}

	uv_tcp_t *client = new uv_tcp_t();
	client->data = this;
	int ret = uv_tcp_init(uv_default_loop(), client);
	if (ret < 0)
	{
		return;
	}

	if (uv_accept((uv_stream_t *)_server_handle, (uv_stream_t *)client) == 0)
	{
		int ret = uv_read_start((uv_stream_t *)client,
			[](uv_handle_t *client_handle, size_t suggested_size, uv_buf_t *buf) {buf->base = new char[suggested_size]; buf->len = suggested_size; },
			[](uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {static_cast<CServer *>(client->data)->on_new_data_recv_callback(client, nread, buf); }
		);
	}
}

void CServer::on_new_data_recv_callback(uv_stream_t *server, ssize_t nread, const uv_buf_t *buf)
{
	if (nread < 0)
	{
		delete buf->base;
		return;
	}

	printf("%s\n", buf->base);

	delete buf->base;
}

void CServer::on_send_data_callback(uv_write_t *req, int status)
{

}

void CServer::on_heart_beat_callback(uv_timer_t *timer_handle)
{
	printf("heart beat\n");
}
