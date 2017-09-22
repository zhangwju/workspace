#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uci_opt.h"

int main(int argc, char **argv)
{
	char *str = NULL;

	uci_set_section("test", "people", "people");
	uci_set_option("test", "people", "name", "lucy", UCI_TYPE_STRING);
	uci_set_option("test", "people", "age", "20", UCI_TYPE_STRING);
	uci_set_option("test", "people", "sex", "boy", UCI_TYPE_STRING);
	uci_set_option("test", "people", "friends", "lily", UCI_TYPE_LIST);
	uci_set_option("test", "people", "friends", "jack", UCI_TYPE_LIST);
	uci_set_option("test", "people", "friends", "tom", UCI_TYPE_LIST);
	uci_set_option("test", "people", "friends", "jams", UCI_TYPE_LIST);
	uci_opt_commit("test");
	printf("name: %s\n", uci_get_option("test", "people", "name"));
	printf("age: %s\n", uci_get_option("test", "people", "age"));
	printf("sex: %s\n", uci_get_option("test", "people", "sex"));
	printf("friends: %s\n", uci_get_option("test", "people", "friends"));
	
	uci_del_option("test", "people", "sex", "boy", UCI_TYPE_STRING);
	uci_del_option("test", "people", "friends", "tom", UCI_TYPE_LIST);
	uci_opt_commit("test");
	printf("name: %s\n", uci_get_option("test", "people", "name"));
	printf("age: %s\n", uci_get_option("test", "people", "age"));
	printf("sex: %s\n", uci_get_option("test", "people", "sex"));
	printf("friends: %s\n", uci_get_option("test", "people", "friends"));

	return 0;
}
