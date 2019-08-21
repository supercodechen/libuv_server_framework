#include <stdio.h>
#include "Server.h"

int main()
{

	CServer server;
	if (server.init())
	{
		if (server.run())
		{
			uv_run(uv_default_loop(), UV_RUN_DEFAULT);
		}
	}
	return 0;
}
