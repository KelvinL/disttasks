#include <stdio.h>
#include <stdbool.h>
#include <proto.h>
#include <stdlib.h>
#include <string.h>

#include "zookeeper.h"

zhandle_t *g_zhdl = NULL;
const char *g_tasksRoot = "/TasksRoot";

void watcher_fn_g(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
        if (type == ZOO_SESSION_EVENT) {
                if (state == ZOO_CONNECTED_STATE) {
                        printf("Zookeeper Connected to zookeeper service successfully!\n");
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
}

int init_zkhandle(const char* host, int timeout, watcher_fn fn, void *cbCtx)
{
	int ret = 0;

	g_zhdl = zookeeper_init(host, fn, timeout, 0, cbCtx, 0);
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

	return ret;
}

void watcher_fn_create_taskroot(int rc, const char* name, const void* data)
{
	fprintf(stderr, "[%s]: rc = %d\n", (char*)(data==0?"null":data), rc);
	if (!rc) {
		fprintf(stderr, "\tname = %s\n", name);
	}
}

int create_taskRootNode()
{
	int ret = 0;
	//const char* tasksRoot = "/TasksRoot";
	struct Stat stat;

	ret = zoo_exists(g_zhdl, g_tasksRoot, true, &stat);
	if(ret == ZOK)
	{
		printf("create_taskRootNode %s already create\n", g_tasksRoot);
	}
	else if(ret == ZNONODE)
	{
		ret = zoo_acreate(g_zhdl, g_tasksRoot, "0", strlen("0"),
				&ZOO_OPEN_ACL_UNSAFE, 0, watcher_fn_create_taskroot, "TasksRoot data");
		if(ret)
		{
			printf("create_taskRootNode%s error", g_tasksRoot);
		}
	}
	else
	{
		printf("create_root error\n");
	}

	return ret;
}

void completion_fn_aget(int rc, const struct String_vector *strings, const void* data)
{
	int ret = 0;
	int i = 0;
	char children[128] = {0};
	if(strings->count > 0)
	{
		//for(i = 0; i < strings->count; i++)
		//{
		//	printf("completion_fn_aget node %d: %s\n", i, strings->data[i]);
		//}
		sprintf(children, "%s/%s", g_tasksRoot, strings->data[0]);
		ret = zoo_delete(g_zhdl, children, -1);
		if(ret)
		{
			if(ret == ZNONODE)
			{
				printf("completion_fn_aget node %s already delete\n", children);
			}
			ret = zoo_aget_children(g_zhdl, g_tasksRoot, false, completion_fn_aget, "root child node");
		}
		else
		{
			//sleep(1);
			printf("completion_fn_aget node %s delete successfully\n", children);
			ret = zoo_aget_children(g_zhdl, g_tasksRoot, false, completion_fn_aget, "root child node");
		}
	}
	else
	{
		//printf("completion_fn_aget no children\n");
		usleep(10000);
		ret = zoo_aget_children(g_zhdl, g_tasksRoot, false, completion_fn_aget, "root child node");
	}
}

int agetChildren()
{
	int ret = 0;

	ret = zoo_aget_children(g_zhdl, g_tasksRoot, false, completion_fn_aget, "root child node");

	return ret;
}

int main(int argc, char* argv[])
{
	int ret = 0;
	const char* host = "127.0.0.1:2181";
	int timeout = 30000;
	int c = '0';

	ret = init_zkhandle(host, timeout, watcher_fn_g, "TaskHandle");
	create_taskRootNode();
	agetChildren();
	while(1)
	{
		c = getchar();
		if(c == 'c')
		{
			break;
		}
	}

	return ret;
}
