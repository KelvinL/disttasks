#include <stdio.h>
#include <stdbool.h>
#include <proto.h>
#include <stdlib.h>
#include <string.h>

#include "zookeeper.h"

zhandle_t *g_zhdl = NULL;

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
	
	g_zhdl = zookeeper_init(host,fn, timeout, 0, cbCtx, 0);
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

void watcher_fn_create_taskroot(int rc, const char* name, const void* data)
{
	fprintf(stderr, "[%s]: rc = %d\n", (char*)(data==0?"null":data), rc);
	if (!rc) {
		fprintf(stderr, "\tname = %s\n", name);
	}
}

const char* tasksRoot = "/TasksRoot";
int create_taskRootNode()
{
	int ret = 0;
	//const char* tasksRoot = "/TasksRoot";
	struct Stat stat;

	ret = zoo_exists(g_zhdl, tasksRoot, true, &stat);
	if(ret == ZOK)
	{
		printf("create_taskRootNode %s already create\n", tasksRoot);
		ret = zoo_set(g_zhdl, tasksRoot, "2", strlen("2"), -1);
		if(ret)
		{
			printf("create_taskRootNode %s set data error\n", tasksRoot);
		}
	}
	else if(ret == ZNONODE)
	{
		ret = zoo_acreate(g_zhdl, tasksRoot, "2", strlen("2"),
				&ZOO_OPEN_ACL_UNSAFE, 0, watcher_fn_create_taskroot, "TasksRoot data");
		if(ret)
		{
			printf("create_taskRootNode%s error", tasksRoot);
		}
	}
	else
	{
		printf("create_root error\n");
	}

	return ret;
}

int create_tasks()
{
	int ret = 0;
	char tasksNode[128] = {0};
	sprintf(tasksNode, "%s/Task", tasksRoot);

	ret = zoo_create(g_zhdl, tasksNode, "Task1", strlen("Task1"), &ZOO_OPEN_ACL_UNSAFE, 
		ZOO_SEQUENCE, NULL, 0);
	if(ret)
	{
		printf("create_tasks create error\n");
		return ret;
	}
	ret = zoo_create(g_zhdl, tasksNode, "Task2", strlen("Task2"), &ZOO_OPEN_ACL_UNSAFE, 
		ZOO_SEQUENCE, NULL, 0);
	if(ret)
	{
		printf("create_tasks create error\n");
		return ret;
	}

	return ret;
}

int main(int argc, char* argv[])
{
	int ret = 0;
	const char* host = "127.0.0.1:2181";
	int timeout = 30000;
	char c = '0';

	ret = init_zkhandle(host, timeout, watcher_fn_g, "TaskDist");
	while(1)
	{
		c = getchar();
		if(c == 'c')
		{
			break;
		}
		else if(c == 'd')
		{
			printf("main dist task to agent\n");
			create_taskRootNode();
		}
		else if(c == 't')
		{
			printf("main create tasks\n");
			create_tasks();
		}
		else if(c == 'h')
		{
			printf("c\texit\n");
			printf("d\ttask root node create\n");
			printf("t\ttask node create\n");
		}
	}

	return ret;
}

