# btjsontree

A JSON API library with 1000 lines of code，simple, small, and easy to use.

概要：

1. node特指object和array，其他的bool,int,int64,double,string为基础类型。
2. object和array使用连续的内存块存储子节点。
3. 当一个node加入为另一个node的子节点后，不需要手工释放这个node，只需要释放最终的根节点==btjson_free_root(rootnode)==。
4. 使用索引方式遍历和访问子节点。

```
rootnode->|bool|int|string|subnode(object/array)|...|double|
                                   |
                                   V
                       |bool|...|string|sub-subnode(object/array)| 
                                          |
                                          V
                                    |string|...|other|

just free the root node by btjson_free_root(rootnode);
```

Summary:

1. "Node" specifically refers to "object" and "array", while other types such as "bool", "int", "int64", "double", and "string" are considered basic types.
2. Both object and array use contiguous memory blocks to store child nodes.
3. After a node is added as a child node of another node, there is no need to manually free this node. It is only necessary to free the final root node == btjson_free_root(rootnode) \==.
4. Traverse and access child nodes using indexing methods.



用法(Method)：

1. 代码生成(New node)

```
struct btjson_node * obj = btjson_new_object();
//or
struct btjson_node * array = btjson_new_array();	
```

2. json字符串生成(json string parse)

```
const char * jsonstr = "{  }";
//some json string, must be "{...}", or "[...]", single value int, string like "10", "hello" is not a valid json string, will return NULL.
struct btjson_node * root = btjson_parse(jsonstr);
//int or int64 will convert to be int64
```

3. 释放(Free)

```
struct btjson_node * root = btjson_new_object();
struct btjson_node * sub  = btjson_new_object();
int index = btjson_add(root, "sub");
btjson_set_node(root, index, sub);

btjson_free_root(root);  //just free the root node
//btjson_free_root(sub); invalid, you don't have to free sub node.

```

4. 添加节点(Add node)

```
//1. if parent is object
struct btjson_node * root = btjson_new_object();
int index = btjson_add(root, "bool_node");
btjson_set_bool(root, index, true);

index = btjson_add(root, "int_node");
btjson_set_int(root, index, 123);

index = btjson_add(root, "int64_node");
btjson_set_int64(root, index, 12345678);

index = btjson_add(root, "double_node");
btjson_set_double(root, index, 123.456);

index = btjson_add(root, "string_node");
btjson_set_string(root, index, "hello world!");

struct btjson_node * sub = btjson_new_object();
index = btjson_add(root, "sub_node");
btjson_set_node(root, index, sub);


//1. if parent is array
struct btjson_node * root = btjson_new_array();
int index = btjson_add(root, NULL);
btjson_set_bool(root, index, true);
.....
```

5. 删除节点(Delete node)

```
//if parent is object
btjson_del(root, "bool_node");
//or
btjson_del_index(root, bool_node_index);

//if parent is array
btjson_del_index(root, bool_node_index);
```

6. 查找节点(Find node)

```c
//if parent is object
int index = btjson_find(root, "bool_node");
if(index < 0)
    return false;
bool b = btjson_get_bool(root, index);

//if parent is array, just get value by index
bool b = btjson_get_bool(root, index);

```

7. 遍历节点(Traverse nodes)

```c
int count = btjson_count(root);
for(int i = 0; i < count; i++)
{
    //if root is object
    const char * key = btjson_get_key(root, i);

    int type = btjson_get_type(root, i);
    switch(type)
    {
    case btjson_node_bool:
        {
           bool b = btjson_get_bool(root, i);
        }
        break;
    case btjson_node_int:
        {
           int i = btjson_get_int(root, i);
        }
        break;
    case btjson_node_int64:
        {
           int64 i64 = btjson_get_int64(root, i);
        }
        break;
    case btjson_node_int64:
        {
           int64 i64 = btjson_get_int64(root, i);
        }
        break;
    case btjson_node_double:
        {
           double d = btjson_get_double(root, i);
        }
        break;
    case btjson_node_string:
        {
           const char * str = btjson_get_string(root, i);
        }
        break;
    case btjson_node_object:
        {
           const struct btjson_node * subobj = btjson_get_node(root, i);//don't free the sub child node
        }
        break;
    case btjson_node_array:
        {
           const struct btjson_node * subarray = btjson_get_node(root, i);//don't free the sub child node
        }
        break;
    }
}
```

8. tojsonstring

```
char * jsonstr = btjson_tojsonstring(root);
//if return NULL, may be failed.
free(jsonstr); //you must free the return value by yourself.
```
