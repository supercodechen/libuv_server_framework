#pragma once
#include <uv.h>

class CServer
{
public:
	CServer();
	~CServer();

	bool init();
	bool run();

public:
	void on_new_client_connect_callback(uv_stream_t *server_handle, int status);
	void on_new_data_recv_callback(uv_stream_t *server, ssize_t nread, const uv_buf_t *buf);
	void on_send_data_callback(uv_write_t *req,int status);
	void on_heart_beat_callback(uv_timer_t *timer_handle);

private:
	uv_tcp_t *_server_handle;
	uv_timer_t *_heart_beat_timer;
	uv_signal_t *_signal;
};

