#include <stdio.h>
#include <stdbool.h>
#include <proto.h>
#include <stdlib.h>
#include <string.h>

#include "zookeeper.h"

zhandle_t *g_zhdl = NULL;
const char *g_root = "/barrier";

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

void watcher_fn_create_root(int rc, const char* name, const void* data)
{
	fprintf(stderr, "[%s]: rc = %d\n", (char*)(data==0?"null":data), rc);
	if (!rc) {
		fprintf(stderr, "\tname = %s\n", name);
	}
}

int create_root(const char* node_name, const char* data)
{
	int ret = 0;
	struct Stat stat;

	ret = zoo_exists(g_zhdl, node_name, true, &stat);
	if(ret == ZOK)
	{
		printf("create_root %s already create\n", node_name);
	}
	else if(ret == ZNONODE)
	{
		ret = zoo_acreate(g_zhdl, node_name, data, strlen(data), 
			&ZOO_OPEN_ACL_UNSAFE, 0, watcher_fn_create_root, "root node create");
		if(ret)
		{
			printf("create_root %s error", node_name);
		}
	}
	else 
	{
		printf("create_root error\n");
	}

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
	const char* tasksRoot = "/TasksRoot";
	struct Stat stat;

	ret = zoo_exists(g_zhdl, tasksRoot, true, &stat);
	if(ret == ZOK)
	{
		printf("create_taskRootNode %s already create\n", tasksRoot);
	}
	else if(ret == ZNONODE)
	{
		ret = zoo_acreate(g_zhdl, tasksRoot, "0", strlen("0"),
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

char childnode_fullname[256] = {0};
void watcher_fn_create_child(int rc, const char* name, const void* data)
{
	fprintf(stderr, "[%s]: rc = %d\n", (char*)(data==0?"null":data), rc);
	if (!rc) {
		fprintf(stderr, "\tname = %s\n", name);
		strcpy(childnode_fullname, name);
		fprintf(stderr, "\tname = %s\n", childnode_fullname);
	}
}

int g_enterFlag = 0;
void completion_fn_enter(int rc, const struct String_vector *strings, const void* data);
void watcher_fn_znode(zhandle_t *zh, int type, int state, const char* path, void* watcherCtx)
{
	int ret = 0;
	printf("watcher_fn_znode ZOO_CHANGED_EVENT \n");
	if(state == ZOO_CONNECTED_STATE)
	{
		if(type == ZOO_CHANGED_EVENT)
		{
			printf("watcher_fn_znode ZOO_CHANGED_EVENT \n");
		}
		else if(type == ZOO_DELETED_EVENT)
		{
			printf("watcher_fn_znode ZOO_DELETED_EVENT\n");
		}
		else if(type == ZOO_CHILD_EVENT)
		{
			printf("watcher_fn_znode ZOO_CHILD_EVENT\n");
			ret = zoo_aget_children(g_zhdl, g_root, false, completion_fn_enter, 
				"root child node");
		}
		else if(type == ZOO_CREATED_EVENT)
		{
			printf("watcher_fn_znode ZOO_CREATED_EVENT\n");
		}
	}
}
void completion_fn_enter(int rc, const struct String_vector *strings, const void* data)
{
	int ret = 0;
	int i = 0;
	struct String_vector *strNew;
	printf("completion_fn_enter count: %d\n", strings->count);
	if(strings->count < 3)
	{
		//ret = zoo_aget_children(g_zhdl, g_root, false, completion_fn_enter, 
		//	"root child node");
		printf("completion_fn_enter set watcher\n");
		ret = zoo_wget_children(g_zhdl, g_root, watcher_fn_znode, NULL, strNew);
		if(ret != ZOK)
		{
			printf("completion_fn_enter set watcher error\n");
		}
	}
	else
	{
		for(i = 0; i < strings->count; i++)
		{
			printf("completion_fn_enter node: %s\n", strings->data[i]);
		}
		g_enterFlag = 0;
	}
}

int enter()
{
	int ret = 0;
	char childNode[128] = {0};
	struct String_vector *strings;
	sprintf(childNode, "%s/hello", g_root);

	ret = zoo_acreate(g_zhdl, childNode, "child", strlen("child"), 
		&ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL|ZOO_SEQUENCE, watcher_fn_create_child, 
		"child node create");
	if(ret)
	{
		printf("enter create child node %s error", childNode);
		return ret;
	}
	g_enterFlag = 1;
	ret = zoo_aget_children(g_zhdl, g_root, false, completion_fn_enter, "root child node");
	//ret = zoo_wget_children(g_zhdl, g_root, watcher_fn_znode, NULL, strings);
	if(ret != ZOK)
	{
		return -1;
	}

	while(g_enterFlag == 1)
	{
		usleep(10);
	}
	printf("enter finish\n");

	return ret;
}

int g_leaveFlag = 0;
void completion_fn_leave(int rc, const struct String_vector *strings, const void* data);
void watcher_fn_znode_leave(zhandle_t *zh, int type, int state, const char* path, void* watcherCtx)
{
	int ret = 0;
	printf("watcher_fn_znode_leave ZOO_CHANGED_EVENT \n");
	if(state == ZOO_CONNECTED_STATE)
	{
		if(type == ZOO_CHANGED_EVENT)
		{
			printf("watcher_fn_znode_leave ZOO_CHANGED_EVENT \n");
		}
		else if(type == ZOO_DELETED_EVENT)
		{
			printf("watcher_fn_znode_leave ZOO_DELETED_EVENT\n");
		}
		else if(type == ZOO_CHILD_EVENT)
		{
			printf("watcher_fn_znode_leave ZOO_CHILD_EVENT\n");
			ret = zoo_aget_children(g_zhdl, g_root, false, completion_fn_leave, 
				"root child node");
		}
		else if(type == ZOO_CREATED_EVENT)
		{
			printf("watcher_fn_znode_leave ZOO_CREATED_EVENT\n");
		}
	}
}
void completion_fn_leave(int rc, const struct String_vector *strings, const void* data)
{
	int ret = 0;
	struct String_vector *strNew;
	printf("completion_fn_leave count: %d\n", strings->count);
	if(strings->count > 0)
	{
		//ret = zoo_aget_children(g_zhdl, g_root, false, completion_fn_leave, 
		//	"root child node");
		ret = zoo_wget_children(g_zhdl, g_root, watcher_fn_znode_leave, NULL, strNew);
		if(ret != ZOK)
		{
			printf("completion_fn_leave set watcher error\n");
		}
	}
	else
	{
		g_leaveFlag = 0;
	}
}

int leave()
{
	int ret = 0;

	ret = zoo_delete(g_zhdl, childnode_fullname, -1);
	if(ret)
	{
		printf("leave delete child node %s error\n", childnode_fullname);
		return ret;
	}
	g_leaveFlag = 1;
	ret = zoo_aget_children(g_zhdl, g_root, false, completion_fn_leave, "root child node");
	while(g_leaveFlag == 1)
	{
		usleep(10);
	}
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	const char* host = "127.0.0.1:2181";
	int timeout = 30000;
	char c = '0';

	ret = init_zkhandle(host, timeout, watcher_fn_g, "Barrier");
	ret = create_root(g_root, "test");

	//enter();

	while(1)
	{
		c = getchar();
		if(c == 'c')
		{
			break;
		}
		else if(c == 'e')
		{
			enter();
		}
		else if(c == 'l')
		{
			leave();
			break;
		}
		else if(c == 't')
		{
			create_taskRootNode();
		}
	}

	fini_zkhandle();

	return ret;
}
