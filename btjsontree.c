/*
    btjsontree.h
    a mini json library include parse, tostring.
    write by zouqinglei, all rights reserved.
    zouqinglei@163.com 20260825
    
*/
#include "btjsontree.h"
// BKDR Hash Function
static uint32_t btjson_hashstring(const char *str)
{
    unsigned int seed = 131;
    unsigned int hash = 0;
 
    while (*str)
    {
        hash = hash * seed + (*str++);
    }
 
    return (hash & 0x7FFFFFFF);
}

static int btjson_vector_binary_search_first(struct btjson_vector * vec, uint32_t hash) {
    int left = 0;
    int right = vec->num - 1;
    int result = -1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        uint32_t tmp = vec->entrys[mid].hash;
        if (tmp >= hash) {
            if (tmp == hash) {
                result = mid;
            }
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    
    return result;
}

static int btjson_vector_binary_search_less(struct btjson_vector * vec, uint32_t hash) {
    int left = 0;
    int right = vec->num - 1;
    int result = -1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        uint32_t tmp = vec->entrys[mid].hash;
        if (tmp < hash) {
            result = mid;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return result;
}

//btjson_vector
static struct btjson_vector * btjson_vector_new()
{
    struct btjson_vector * vec = malloc(sizeof(struct btjson_vector));
    vec->num = 0;
    vec->total = 5;
    int size = sizeof(struct btjson_entry) * vec->total;
    vec->entrys = (struct btjson_entry *)malloc(size);
    memset((void *)vec->entrys, 0, size);
    return vec;
}

static void btjson_vector_free(struct btjson_vector * vec)
{
    if(vec->entrys)
    {
        for(int i = 0; i < vec->num; i++)
        {
            struct btjson_entry * entry = &vec->entrys[i];
            if(entry->key)
                free(entry->key);
            if(entry->node.type == btjson_node_object || entry->node.type == btjson_node_array)
                btjson_vector_free(entry->node.data.c_vec);
            else if(entry->node.type == btjson_node_string)
                free(entry->node.data.c_str);
        }
        
        free(vec->entrys);
    }
    free(vec);
}

static bool btjson_vector_extend(struct btjson_vector * vec)
{
    if(vec->num < vec->total)
        return true;
    int newtotal = vec->total * 2;
    int newsize = sizeof(struct btjson_entry) * newtotal;
    struct btjson_entry * newentry = realloc(vec->entrys, newsize);
    if(newentry == NULL)
        return false;
    
    int oldsize = sizeof(struct btjson_entry) * vec->total;
    memset((void *)newentry + oldsize, 0, newsize - oldsize);
    vec->entrys = newentry;
    vec->total = newtotal;
      
    return true;
}

static int btjson_vector_find(struct btjson_vector * vec, const char * key)
{
    if(key == NULL || strlen(key) == 0)
        return -1;
    
    uint32_t hash = btjson_hashstring(key);
    int index = btjson_vector_binary_search_first(vec, hash);
    if(index < 0)
        return -1;
    
    for(int i = index; i < vec->num; i++)
    {
        if(vec->entrys[index].hash == hash)
        {
            if(strcmp(vec->entrys[index].key, key) == 0)
                return index;
        }
        else
        {
            break;
        }
    }
    return -1;
}

static int btjson_vector_append(struct btjson_vector * vec)
{
    if(btjson_vector_extend(vec) == false)
        return -1;
    
    int index = vec->num;
    vec->num++;
    int size = sizeof(struct btjson_entry);
    memset((void *)vec->entrys + index * size, 0, size);
    return index;
}

static int btjson_vector_add(struct btjson_vector * vec, const char * key)
{
    if(key == NULL || strlen(key) == 0)
        return -1;
    
    int index = btjson_vector_find(vec, key);
    if(index >= 0)
        return index;
    
    if(btjson_vector_extend(vec) == false)
        return -1;
    
    //insert by sort hash
    uint32_t hash = btjson_hashstring(key);
    int size = sizeof(struct btjson_entry);
    index = btjson_vector_binary_search_less(vec, hash);
    index++;
    int movesize = (vec->num - index) * size;
    if(movesize > 0)
    {
        int offset = index *  size;
        int dst_offset = (index + 1) * size;
        memmove((void *)vec->entrys + dst_offset, (void *)vec->entrys + offset, movesize);
    }
    
    memset((void *)vec->entrys + index * size, 0, size);
    vec->entrys[index].hash = hash;
    vec->entrys[index].key = strcpy(malloc(strlen(key) + 1), key);
    vec->num++;
    
    return index;
}

static bool btjson_vector_clear(struct btjson_vector * vec, int index)
{
    if(index < 0 ||index >= vec->num)
        return false;
    
    struct btjson_entry * entry = &vec->entrys[index];
    if(entry->node.type == btjson_node_object || entry->node.type == btjson_node_array)
    {
        btjson_vector_free(entry->node.data.c_vec);
        entry->node.data.c_vec = NULL;
    }
    else if(entry->node.type == btjson_node_string)
    {
        free(entry->node.data.c_str);
        entry->node.data.c_str = NULL;
    }
    entry->node.type = btjson_node_null;
    
    return true;
}

static bool btjson_vector_del(struct btjson_vector * vec, int index)
{
    if(btjson_vector_clear(vec, index) == false)
        return false;
    
    int size = sizeof(struct btjson_entry);
    int movesize = (vec->num - index - 1) * size;
    if(movesize > 0)
    {
        int offset = (index + 1)  *  size;
        int dst_offset = index * size;
        memmove((void *)vec->entrys + dst_offset, (void *)vec->entrys + offset, movesize);
    }
    
    vec->num--;
    memset((void *)vec->entrys + vec->num * size, 0, size);
    
    return true;
}
//end btjson_vector

struct btjson_node * btjson_new_object()
{
    struct btjson_node * node = (struct btjson_node *)malloc(sizeof(struct btjson_node));
    node->type = btjson_node_object;
    node->data.c_vec = btjson_vector_new();
    return node;
}

struct btjson_node * btjson_new_array()
{
    struct btjson_node * node = (struct btjson_node *)malloc(sizeof(struct btjson_node));
    node->type = btjson_node_array;
    node->data.c_vec = btjson_vector_new();
    return node;
}

void  btjson_free_root(struct btjson_node * node)
{
    if(node == NULL)
        return;
    
    if(node->type == btjson_node_object || node->type == btjson_node_array)
    {
        btjson_vector_free(node->data.c_vec);
    }
    else if(node->type == btjson_node_string)
    {
        free(node->data.c_str);
    }
    
    free(node);
}

int  btjson_add(struct btjson_node * node, const char * key)
{
    if(node == NULL)
        return -1;
    if(node->type == btjson_node_object)
    {
        if(key == NULL || strlen(key) == 0)
            return -1;
        
        return btjson_vector_add(node->data.c_vec, key);
    }
    else if(node->type == btjson_node_array)
    {
        return btjson_vector_append(node->data.c_vec);
    }
    
    return -1;
}

static bool btjson_node_check(const struct btjson_node * node, int index)
{
    if(node == NULL)
        return false;
    if(node->type != btjson_node_object && node->type != btjson_node_array)
        return false;
    if(index < 0 ||index >= node->data.c_vec->num)
        return false;
    
    return true;
}

static bool btjson_node_check_clear(struct btjson_node * node, int index)
{
    if(btjson_node_check(node, index) == false)
        return false;
    
    return btjson_vector_clear(node->data.c_vec, index);
}

bool btjson_set_bool(struct btjson_node * node, int index, bool b)
{
    if(btjson_node_check_clear(node, index) == false)
        return false;
    
    struct btjson_entry * entry = &node->data.c_vec->entrys[index];
    entry->node.type = btjson_node_bool;
    entry->node.data.c_bool = b;
    return true;
}

bool btjson_set_int(struct btjson_node * node, int index, int32_t i)
{
    if(btjson_node_check_clear(node, index) == false)
        return false;
    
    struct btjson_entry * entry = &node->data.c_vec->entrys[index];
    entry->node.type = btjson_node_int;
    entry->node.data.c_int = i;
    return true;
}

bool btjson_set_int64(struct btjson_node * node, int index, int64_t i64)
{
    if(btjson_node_check_clear(node, index) == false)
        return false;
    
    struct btjson_entry * entry = &node->data.c_vec->entrys[index];
    entry->node.type = btjson_node_int64;
    entry->node.data.c_int64 = i64;
    return true;
}

bool btjson_set_double(struct btjson_node * node, int index, double d)
{
    if(btjson_node_check_clear(node, index) == false)
        return false;
    
    struct btjson_entry * entry = &node->data.c_vec->entrys[index];
    entry->node.type = btjson_node_double;
    entry->node.data.c_double = d;
    return true;
}

bool btjson_set_string(struct btjson_node * node, int index, const char * str)
{
    if(btjson_node_check_clear(node, index) == false)
        return false;
    
    struct btjson_entry * entry = &node->data.c_vec->entrys[index];
    entry->node.type = btjson_node_string;
    if(str)
        entry->node.data.c_str = strcpy(malloc(strlen(str) + 1), str);
    
    return true;
}

bool btjson_set_node(struct btjson_node * node, int index, struct btjson_node * subnode)
{
    if(btjson_node_check_clear(node, index) == false)
        return false;
    
    struct btjson_entry * entry = &node->data.c_vec->entrys[index];
    entry->node.type = subnode->type;
    entry->node.data.c_vec = subnode->data.c_vec;
    free(subnode);
    return true;
}

bool btjson_del(struct btjson_node * node, const char * key)
{
    int index = btjson_find(node, key);
    if(index < 0)
        return false;
    
    return btjson_vector_del(node->data.c_vec, index);
}

bool btjson_del_index(struct btjson_node * node, int index)
{
    if(node == NULL)
        return false;
    if(node->type != btjson_node_array)
        return false;
    
    return btjson_vector_del(node->data.c_vec, index);
}

int btjson_find(const struct btjson_node * node, const char * key)
{
    if(node == NULL)
        return -1;
    if(node->type != btjson_node_object)
        return -1;
    
    return btjson_vector_find(node->data.c_vec, key);
}

int  btjson_count(const struct btjson_node * node)
{
    if(node == NULL)
        return 0;
    
    if(node->type == btjson_node_object || node->type == btjson_node_array)
    {
        return node->data.c_vec->num;
    }
    
    return 0;
}

int  btjson_get_type(const struct btjson_node * node, int index)
{
    if(btjson_node_check(node, index ) == false)
        return 0;
    return node->data.c_vec->entrys[index].node.type;
}

const char * btjson_get_key(const struct btjson_node * node, int index)
{
    if(btjson_node_check(node, index ) == false)
        return NULL;
    return node->data.c_vec->entrys[index].key;
}

bool btjson_get_bool(const struct btjson_node * node, int index)
{
    if(btjson_node_check(node, index ) == false)
        return false;
    
    struct btjson_node * subnode = &node->data.c_vec->entrys[index].node;
    switch(subnode->type)
    {
    case btjson_node_bool:
        return subnode->data.c_bool;
    case btjson_node_int:
        return subnode->data.c_int != 0;
    case btjson_node_int64:
        return subnode->data.c_int64 != 0;
    case btjson_node_double:
        return subnode->data.c_double != 0;
    case btjson_node_string:
        return (subnode->data.c_str != NULL && strlen(subnode->data.c_str) > 0);
    default:
        return 0;
    }
}

int32_t btjson_get_int(const struct btjson_node * node, int index)
{
    if(btjson_node_check(node, index ) == false)
        return 0;
    struct btjson_node * subnode = &node->data.c_vec->entrys[index].node;
    switch(subnode->type)
    {
    case btjson_node_bool:
        return subnode->data.c_bool;
    case btjson_node_int:
        return subnode->data.c_int;
    case btjson_node_int64:
            return (int32_t)subnode->data.c_int64;
    case btjson_node_double:
            return (int32_t)subnode->data.c_double;
    case btjson_node_string:
            return atoi(subnode->data.c_str);
    default:
        return 0;
    }
}

int64_t btjson_get_int64(const struct btjson_node * node, int index)
{
    if(btjson_node_check(node, index ) == false)
        return 0;
    struct btjson_node * subnode = &node->data.c_vec->entrys[index].node;
    switch(subnode->type)
    {
    case btjson_node_bool:
        return subnode->data.c_bool;
    case btjson_node_int:
        return subnode->data.c_int;
    case btjson_node_int64:
            return subnode->data.c_int64;
    case btjson_node_double:
            return (int64_t)subnode->data.c_double;
    case btjson_node_string:
            return atoll(subnode->data.c_str);
    default:
        return 0;
    }
}

double  btjson_get_double(const struct btjson_node * node, int index)
{
    if(btjson_node_check(node, index ) == false)
        return 0.0;
    struct btjson_node * subnode = &node->data.c_vec->entrys[index].node;
    switch(subnode->type)
    {
    case btjson_node_bool:
        return subnode->data.c_bool;
    case btjson_node_int:
        return subnode->data.c_int;
    case btjson_node_int64:
        return subnode->data.c_int64;
    case btjson_node_double:
        return subnode->data.c_double;
    case btjson_node_string:
        return atof(subnode->data.c_str);
    default:
        return 0;
    }
}

const char * btjson_get_string(const struct btjson_node * node, int index)
{
    if(btjson_node_check(node, index ) == false)
        return NULL;
    struct btjson_node * subnode = &node->data.c_vec->entrys[index].node;
    switch(subnode->type)
    {
    case btjson_node_bool:
        return "bool";
    case btjson_node_int:
        return "int";
    case btjson_node_int64:
        return "int64";
    case btjson_node_double:
         return "double";
    case btjson_node_string:
         return (subnode->data.c_str);
    default:
        return 0;
    }
}

const struct btjson_node * btjson_get_node(const struct btjson_node * node, int index)
{
    if(btjson_node_check(node, index ) == false)
        return NULL;
    return &node->data.c_vec->entrys[index].node;
}

//parese
typedef struct stbtparse_input
{
    const char * str;
    int pos;
}btparse_input;

void btparse_input_init(btparse_input * input, const char * parse_str)
{
    input->str = parse_str;
    input->pos = 0;
}

typedef struct stbtparse_output
{
    char * str;
    int pos;
    int length;
}btparse_output;

void btparse_output_init(btparse_output * output)
{
    output->pos = 0;
    output->str = (char *)malloc(32);
    memset(output->str, 0, 32);
    output->length = 32;
}

void btparse_output_destroy(btparse_output * output)
{
    if(output == NULL)
        return;
    if(output->str)
        free(output->str);
    
    memset(output, 0, sizeof(btparse_output));
}

bool btparse_output_extend(btparse_output * output, int minsize)
{
    int newsize = 0;
    if(output->length < minsize)
        newsize = minsize * 2;
    else
        newsize = output->length * 2;
    
    char * newstr = realloc(output->str, newsize);
    if(newstr == NULL)
        return false;
    
    memset(newstr + output->length, 0, newsize - output->length);
    output->str = newstr;
    output->length = newsize;
    
    return true;
}

bool btparse_output_append_char(btparse_output * output, char c)
{
    if(output->pos == output->length)
    {
        if(btparse_output_extend(output, 1) == false)
            return false;
    }
    
    output->str[output->pos++] = c;
    return true;
}

bool btparse_output_append_string(btparse_output * output, const char * str)
{
    int len = strlen(str);
    if((output->pos + len + 1) >= output->length)
    {
        if(btparse_output_extend(output, len) == false)
            return false;
    }
    
    memcpy(output->str + output->pos, str, len);
    output->pos += len;
    
    return true;
}

static void btparse_skip_blank(btparse_input * input)
{
    const char * blankstr = " \f\n\r\t\v";
    int index = 0;
    while(1)
    {
        int c = input->str[input->pos];
        if(strchr(blankstr, c))
        {
            input->pos++;
            continue;
        }
        break;
    }
}

static bool btparse_is_byte(btparse_input * input, char c)
{
    btparse_skip_blank(input);
    int current = input->str[input->pos];
    if(current == c)
    {
        return true;
    }
    
    return false;
}

static bool btparse_byte(btparse_input * input, char c)
{
    btparse_skip_blank(input);
    int current = input->str[input->pos];
    if(current == c)
    {
        input->pos++;
        return true;
    }
    
    return false;
}

static bool btparse_byteseq(btparse_input * input, const char * seq)
{
    btparse_skip_blank(input);
    const char * ptr = &input->str[input->pos];
    if(strstr(ptr, seq) == ptr)
    {
        input->pos += strlen(seq);
        return true;
    }
    
    return false;
}

static bool btparse_int64(btparse_input * input, int64_t * result)
{
    btparse_skip_blank(input);
    const char * beginptr = &input->str[input->pos];
    char * endptr = NULL;
    
    int64_t i64 = strtoll(beginptr, &endptr, 0);
    
    if(endptr == beginptr)
        return false;
    
    if(*endptr == '\0')
        return false;
    
    if(isalnum(*endptr))
        return false;
    
    input->pos += (endptr - beginptr);
    *result = i64;
    return true;
}

static bool btparse_double(btparse_input * input, double * result)
{
    btparse_skip_blank(input);
    const char * beginptr = &input->str[input->pos];
    char * endptr = NULL;
    
    double d = strtod(beginptr, &endptr);
    
    if(endptr == beginptr)
        return false;
    
    if(*endptr == '\0')
        return false;
    
    if(isalnum(*endptr))
        return false;
    
    input->pos += (endptr - beginptr);
    *result = d;
    return true;
}

static bool btparse_string(btparse_input * input, btparse_output * output)
{
    btparse_skip_blank(input);
    int pos = input->pos;
    if(input->str[pos] != '\"')
        return false;
    pos++;
    while(1)
    {
        char s = input->str[pos++];
        if(s == '\0')
            return false;
        
        if(s == '\"')
        {
            input->pos = pos;
            return true;
        }
        
        if(s != '\\')
        {
            btparse_output_append_char(output, s);
            continue;
        }
        
        s = input->str[pos++];
        switch(s)
        {
        case 'a':
            btparse_output_append_char(output, '\a');
            break;
        case 'b':
            btparse_output_append_char(output, '\b');
            break;
        case 'f':
            btparse_output_append_char(output, '\f');
            break;
        case 'n':
            btparse_output_append_char(output, '\n');
            break;
        case 'r':
            btparse_output_append_char(output, '\r');
            break;
        case 't':
            btparse_output_append_char(output, '\t');
            break;
        case 'v':
            btparse_output_append_char(output, '\v');
            break;
        case '\\':
            btparse_output_append_char(output, '\\');
            break;
        case '\'':
            btparse_output_append_char(output, '\'');
            break;
        case '"':
            btparse_output_append_char(output, '\"');
            break;
        case '0':
            btparse_output_append_char(output, '\0');
            break;
        default:
            btparse_output_append_char(output, '\\');
            btparse_output_append_char(output, s);
        }
    }
    
    return false;
}

struct btjson_node * btjson_parse_input(btparse_input * input);
static bool btjson_parse_value(btparse_input * input, struct btjson_node * node, int index)
{
    bool bok = true;
    if(btparse_is_byte(input, '\"'))
    {
        btparse_output output;
        btparse_output_init(&output);
        if(btparse_string(input, &output) == false)
        {
            btparse_output_destroy(&output);
            return false;
        }
        btjson_set_string(node, index, output.str);
        btparse_output_destroy(&output);
    }
    else if(btparse_is_byte(input, '{'))
    {
        struct btjson_node * subnode = btjson_parse_input(input);
        if(subnode == NULL)
        {
            return false;
        }
        btjson_set_node(node, index, subnode);
    }
    else if(btparse_is_byte(input, '['))
    {
        struct btjson_node * subnode = btjson_parse_input(input);
        if(subnode == NULL)
        {
            return false;
        }
        btjson_set_node(node, index, subnode);
    }
    else if(btparse_byteseq(input, "null"))
    {
    }
    else if(btparse_byteseq(input, "true"))
    {
        btjson_set_bool(node, index, true);
    }
    else if(btparse_byteseq(input, "false"))
    {
        btjson_set_bool(node, index, false);
    }
    else
    {
        int64_t i64;
        double d;
        if(btparse_int64(input, &i64))
        {
            btjson_set_int64(node, index, i64);
        }
        else if(btparse_double(input, &d))
        {
            btjson_set_double(node, index, d);
        }
        else
        {
            return false;
        }
    }
    
    return true;
}

struct btjson_node * btjson_parse_input(btparse_input * input)
{
    btparse_skip_blank(input);
    bool bok = true;
    struct btjson_node * node = NULL;
    if(btparse_byte(input, '{'))
    {
        node = btjson_new_object();
        while(1)
        {
            btparse_output key;
            btparse_output_init(&key);
            if(btparse_string(input, &key) == false)
            {
                btparse_output_destroy(&key);
                bok = false;
                break;
            }
            
            int index = btjson_add(node, key.str);
            btparse_output_destroy(&key);
            
            if(btparse_byte(input, ':') == false)
            {
                bok = false;
                break;
            }
            
            bok = btjson_parse_value(input, node, index);
            if(!bok)
                break;
            
            if(btparse_byte(input, '}'))
                break;
            else if(btparse_byte(input, ',') == false)
            {
                bok = false;
                break;
            }
        }
    }
    else if(btparse_byte(input, '['))
    {
        node = btjson_new_array();
        while(1)
        {
            int index = btjson_add(node, NULL);
            
            bok = btjson_parse_value(input, node, index);
            if(!bok)
                break;
            
            if(btparse_byte(input, ']'))
                break;
            else if(btparse_byte(input, ',') == false)
            {
                bok = false;
                break;
            }
        }
    }
    
    if(!bok)
    {
        btjson_free_root(node);
        node = NULL;
    }
    
    return node;
}

struct btjson_node * btjson_parse(const char * jsonstr)
{
    btparse_input input;
    btparse_input_init(&input, jsonstr);
    struct btjson_node * node = btjson_parse_input(&input);
    
    return node;
}
/////////////////////////////////////////////////
//build output
static bool btbuild_string(btparse_output * output, const char * str)
{
    btparse_output_append_char(output, '"');
    const char * s = str;
    while(*s)
    {
        switch(*s)
        {
        case '\a':
            btparse_output_append_string(output, "\\a");
            break;
        case '\b':
            btparse_output_append_string(output, "\\b");
            break;
        case '\f':
            btparse_output_append_string(output, "\\f");
            break;
        case '\n':
            btparse_output_append_string(output, "\\n");
            break;
        case '\r':
            btparse_output_append_string(output, "\\r");
            break;
        case '\t':
            btparse_output_append_string(output, "\\t");
            break;
        case '\v':
            btparse_output_append_string(output, "\\v");
            break;
        case '\\':
            btparse_output_append_string(output, "\\\\");
            break;
        case '\"':
            btparse_output_append_string(output, "\\\"");
            break;
        default:
            btparse_output_append_char(output, *s);
        }
        s++;
    }
    
    return btparse_output_append_char(output, '"');
}

static bool btbuild_int(btparse_output * output, int32_t i)
{
    char buf[128];
    int size;
    size = snprintf(buf, sizeof(buf), "%d", i);
    
    return btparse_output_append_string(output, buf);
}

static bool btbuild_int64(btparse_output * output, int64_t i64)
{
    char buf[128];
    int size;
    size = snprintf(buf, sizeof(buf), "%lld", i64);
    
    return btparse_output_append_string(output, buf);
}

static bool btbuild_double(btparse_output * output, double d)
{
    char buf[256], *p, *q;
    int size;
    if(isnan(d))
    {
        size = snprintf(buf, sizeof(buf), "NaN");
    }
    else if(isinf(d))
    {
        if(d > 0)
            size = snprintf(buf, sizeof(buf), "Infinity");
        else
            size = snprintf(buf, sizeof(buf), "-Infinity");
    }
    else
    {
        const char * format = "%.17g";
        int format_drops_decimals = 0;
        int looks_numeric = 0;

        size = snprintf(buf, sizeof(buf), format, d);

        if (size < 0)
            return false;

        p = strchr(buf, ',');
        if (p)
            *p = '.';
    }

    if (size < 0)
        return false;

    if (size >= (int)sizeof(buf))
        size = sizeof(buf) - 1;
    
    return btparse_output_append_string(output, buf);
}

static bool btjson_build_output(const struct btjson_node * node, btparse_output * output);
static bool btjson_build_value(const struct btjson_node * node, btparse_output * output, int index)
{
    bool bok = false;
    int type = btjson_get_type(node, index);
    switch(type)
    {
    case btjson_node_null:
        bok = btparse_output_append_string(output, "null");
        break;
    case btjson_node_bool:
        {
            bool b = btjson_get_bool(node, index);
            bok = btparse_output_append_string(output, b?"true":"false");
        }
        break;
    case btjson_node_int:
        {
            int i32 = btjson_get_int(node, index);
            bok = btbuild_int(output, i32);
        }
        break;
    case btjson_node_int64:
        {
            int i64 = btjson_get_int64(node, index);
            bok = btbuild_int64(output, i64);
        }
        break;
    case btjson_node_double:
        {
            double d = btjson_get_double(node, index);
            bok = btbuild_double(output, d);
        }
        break;
    case btjson_node_string:
        {
            const char * str = btjson_get_string(node, index);
            bok = btbuild_string(output, str);
        }
        break;
    case btjson_node_object:
    case btjson_node_array:
        {
            const struct btjson_node * subnode = btjson_get_node(node, index);
            bok = btjson_build_output(subnode, output);
        }
        break;
    default:
        return false;
    }
    
    return bok;
}

static bool btjson_build_childs(const struct btjson_node * node, btparse_output * output)
{
    int count = btjson_count(node);
    for(int index = 0; index < count; index++)
    {
        if(index != 0)
            btparse_output_append_string(output, ", ");
        
        if(node->type == btjson_node_object)
        {
            const char * key = btjson_get_key(node, index);
            btbuild_string(output, key);
            btparse_output_append_string(output, " : ");
        }
        
        if(btjson_build_value(node, output, index) == false)
            return false;
    }
    
    return true;
}

static bool btjson_build_output(const struct btjson_node * node, btparse_output * output)
{
    if(node->type == btjson_node_object)
    {
        btparse_output_append_string(output, "{  ");
        btjson_build_childs(node, output);
        btparse_output_append_string(output, "  }");
    }
    else if(node->type == btjson_node_array)
    {
        btparse_output_append_string(output, "[  ");
        btjson_build_childs(node, output);
        btparse_output_append_string(output, "  ]");
    }
        
    return true;
}

char * btjson_tojsonstring(const struct btjson_node * node)
{
    if(node == NULL)
        return NULL;
    
    btparse_output output;
    btparse_output_init(&output);
    
    if(btjson_build_output(node, &output) == false)
    {
        btparse_output_destroy(&output);
        return NULL;
    }

    return output.str;
}
