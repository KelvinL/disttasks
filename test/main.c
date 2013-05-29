#include <stdio.h>
#include <stdbool.h>
#include <proto.h>
#include <stdlib.h>
#include <string.h>

#include "zookeeper.h"

//extern ZOOAPI const int ZOO_EXPIRED_SESSION_STATE;
//extern ZOOAPI const int ZOO_AUTH_FAILED_STATE;
//extern ZOOAPI const int ZOO_CONNECTING_STATE;
//extern ZOOAPI const int ZOO_ASSOCIATING_STATE;
//extern ZOOAPI const int ZOO_CONNECTED_STATE;
void QueryServer_watcher_g(zhandle_t* zh, int type, int state,
		const char* path, void* watcherCtx)
{
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			printf("[[[QueryServer]]] Connected to zookeeper service successfully!\n");
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

void QueryServer_string_completion(int rc, const char *name, const void *data)
{
	fprintf(stderr, "[%s]: rc = %d\n", (char*)(data==0?"null":data), rc);
	if (!rc) {
		fprintf(stderr, "\tname = %s\n", name);
	}
}

int main(int argc, char *argv[])
{
	const char* host = "127.0.0.1:2181";
	int timeout = 30000;

	zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
	zoo_deterministic_conn_order(1);
	zhandle_t* zkhandle = zookeeper_init(host, QueryServer_watcher_g, timeout, 0, "hello zookeeper", 0);
	if(zkhandle == NULL)
	{
		fprintf(stderr, "Error when connecting to zookeeper servers...\n");
		exit(EXIT_FAILURE);
	}

	int ret = zoo_acreate(zkhandle, "/QueryServer", "alive", 5, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, 
			QueryServer_string_completion, "zoo_acreate");
	if(ret)
	{
		fprintf(stderr, "Error %d for %s\n", ret, "acreate");
		exit(EXIT_FAILURE);
	}

	do{
		sleep(9);	
	}while(false);

	zookeeper_close(zkhandle);

	return 0;
}
