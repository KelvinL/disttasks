#include <stdio.h>
#include <stdbool.h>
#include <proto.h>
#include <stdlib.h>
#include <string.h>

#include "zookeeper.h"

zhandle_t *g_zhdl = NULL;
clientid_t g_cliID;
char *g_pBuffer = NULL;

void watcher_fn_g(zhandle_t* zh, int type, int state, const char* path, void *watcherCtx)
{
	clientid_t *pCliID = NULL;
	if(type == ZOO_SESSION_EVENT)
	{
		printf("watcher_fn_g ZOO_SESSION_EVENT \n");
		if (state == ZOO_CONNECTED_STATE) {
			printf("Zookeeper Connected to zookeeper service successfully!\n");
			pCliID = zoo_client_id(zh);
			//memcpy(&g_cliID, pCliID, sizeof(clientid_t));
			printf("init_zkhandle client id: 0x%8x\n", pCliID->client_id);
		} else if (state == ZOO_EXPIRED_SESSION_STATE) {
			printf("Zookeeper session expired!\n");
		} else if (state == ZOO_AUTH_FAILED_STATE) {
			printf("Zookeeper session auth failed\n");
		} else if (state == ZOO_CONNECTING_STATE) {
			printf("Zookeeper session connecting\n");
		} else if (state == ZOO_ASSOCIATING_STATE) {
			printf("Zookeeper session associating\n");
		}

	}
	else if(type == ZOO_CREATED_EVENT)
	{
		printf("watcher_fn_g ZOO_CREATED_EVENT \n");
	}
	else if(type == ZOO_DELETED_EVENT)
	{
		printf("watcher_fn_g ZOO_DELETE_EVENT \n");
	}
	else if(type == ZOO_CHANGED_EVENT)
	{
		printf("watcher_fn_g ZOO_CHANGED_EVENT \n");
	}
	else if(type == ZOO_CHILD_EVENT)
	{
		printf("watcher_fn_g ZOO_CHILD_EVENT \n");
	}
	else if(type == ZOO_NOTWATCHING_EVENT)
	{
		printf("watcher_fn_g ZOO_NOTWATCHING_EVENT \n");
	}
}

int init_zkhandle(const char* host, int timeout, watcher_fn fn, void*cbCtx)
{
	int ret = 0;

	g_zhdl = zookeeper_init(host, fn, timeout, &g_cliID, cbCtx, 0);
	if(g_zhdl == NULL)
	{
		printf("init_zkhandle init error\n");
		ret = -1;
	}

	return ret;
}

int fini_zkhandle()
{
	int ret = 0;

	zookeeper_close(g_zhdl);
	g_zhdl = NULL;

	return ret;
}

void watcher_fn_znode(zhandle_t *zh, int type, int state, const char* path, void* watcherCtx)
{
	int ret = 0;
	struct Stat MyStat;
	int bufLen = 4*1024;
	printf("watcher_fn_znode ZOO_CHANGED_EVENT \n");
	if(state == ZOO_CONNECTED_STATE)
	{
		if(type == ZOO_CHANGED_EVENT)
		{
			printf("watcher_fn_znode ZOO_CHANGED_EVENT \n");
			ret = zoo_wget(g_zhdl, path, watcher_fn_znode, NULL, g_pBuffer, &bufLen, &MyStat);
			if(ret)
			{
				printf("watcher_fn_znode wget error: %d\n", ret);
			}
			printf("watcher_fn_znode data: %s, len: %d\n", g_pBuffer, bufLen);
		}
		else if(type == ZOO_DELETED_EVENT)
		{
			printf("watcher_fn_znode ZOO_DELETED_EVENT\n");
		}
		else if(type == ZOO_CHILD_EVENT)
		{
			printf("watcher_fn_znode ZOO_CHILD_EVENT\n");
		}
		else if(type == ZOO_CREATED_EVENT)
		{
			printf("watcher_fn_znode ZOO_CREATED_EVENT\n");
		}
	}
}

int create_ephemeral_node(const char* pNodePath, const char* pData, int nDataLen)
{
	int ret = 0;
	struct Stat MyStat;
	int bufLen = 4*1024;
	
	ret = zoo_create(g_zhdl, pNodePath, pData, nDataLen, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);
	if(ret)
	{
		printf("create_ephemeral_node error\n");
		return ret;
	}
	ret = zoo_wget(g_zhdl, pNodePath, watcher_fn_znode, NULL, g_pBuffer, &bufLen, &MyStat);
	if(ret)
	{
		printf("wget error: %d\n", ret);
	}
	printf("create_ephemeral_node data: %s\n", g_pBuffer);

	return ret;
}

int main(int argc, char* argv[])
{
	int ret = 0;
	const char* host = "127.0.0.1:2181";
	int timeout = 30000;
	char c = '0';
	g_pBuffer = malloc(1024*4);
	memset(g_pBuffer, 0x0, strlen(g_pBuffer));

	ret = init_zkhandle(host, timeout, watcher_fn_g, "ConfigMgr");
	while(1)
	{
		c = getchar();
		if(c == 'c')
		{
			break;
		}
		else if(c == 'q')
		{
			fini_zkhandle();
			break;
		}
		else if(c == 'r')
		{
			ret = init_zkhandle(host, timeout, watcher_fn_g, "ConfigMgr");
		}
		else if(c == 'n')
		{
			create_ephemeral_node("/ConfigMgr", "CMS:10.82.13.107", sizeof("CMS:10.82.13.107"));
		}
	}

	return ret;
}
