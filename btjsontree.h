/*
    btjsontree.h
    a mini json library include parse, tostring.
    write by zouqinglei, all rights reserved.
    zouqinglei@163.com 20260825
    
*/
#ifndef _btjsontree_h_
#define _btjsontree_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>

#pragma pack(push, 1)

#define btjson_node_null    0
#define btjson_node_bool    1
#define btjson_node_int     2
#define btjson_node_int64   3
#define btjson_node_double  4
#define btjson_node_string  5
#define btjson_node_blob    6 //reserved
#define btjson_node_object  7
#define btjson_node_array   8

struct btjson_entry;
struct btjson_vector
{
	struct btjson_entry * entrys;
	int num;
	int total;
};

struct btjson_node
{
    uint8_t type;
    union{
        bool     c_bool;
        int32_t  c_int;
        int64_t  c_int64;
        double   c_double;
        char *   c_str;
        struct btjson_vector * c_vec;
    }data;
};

struct btjson_entry
{
	uint32_t hash;
	char * key;
	struct btjson_node node;
};
#pragma pack(pop)

struct btjson_node * btjson_new_object();
struct btjson_node * btjson_new_array();
void   btjson_free_root(struct btjson_node * node);

struct btjson_node * btjson_parse(const char * jsonstr);
char * btjson_tojsonstring(const struct btjson_node * node);

int  btjson_add(struct btjson_node * node, const char * key);
bool btjson_set_bool(struct btjson_node * node, int index, bool b);
bool btjson_set_int(struct btjson_node * node, int index, int32_t i);
bool btjson_set_int64(struct btjson_node * node, int index, int64_t i64);
bool btjson_set_double(struct btjson_node * node, int index, double d);
bool btjson_set_string(struct btjson_node * node, int index, const char * str);
bool btjson_set_node(struct btjson_node * node, int index, struct btjson_node * subnode);

bool btjson_del(struct btjson_node * node, const char * key);
bool btjson_del_index(struct btjson_node * node, int index);

int  btjson_find(const struct btjson_node * node, const char * key);

int  btjson_count(const struct btjson_node * node);
int  btjson_get_type(const struct btjson_node * node, int index);
const char * btjson_get_key(const struct btjson_node * node, int index);

bool    btjson_get_bool(const struct btjson_node * node, int index);
int32_t btjson_get_int(const struct btjson_node * node, int index);
int64_t btjson_get_int64(const struct btjson_node * node, int index);
double  btjson_get_double(const struct btjson_node * node, int index);
const char * btjson_get_string(const struct btjson_node * node, int index);
const struct btjson_node * btjson_get_node(const struct btjson_node * node, int index);

#ifdef __cplusplus
}
#endif

#endif
