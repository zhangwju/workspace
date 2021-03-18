
/* This is the reply object returned by redisCommand() */
typedef struct redisReply {
    int type; /* REDIS_REPLY_* */
    long long integer; /* The integer when type is REDIS_REPLY_INTEGER */
    size_t len; /* Length of string */
    char *str; /* Used for both REDIS_REPLY_ERROR and REDIS_REPLY_STRING */
    size_t elements; /* number of elements, for REDIS_REPLY_ARRAY */
    struct redisReply **element; /* elements vector for REDIS_REPLY_ARRAY */
} redisReply;

#define REDIS_ERR -1
#define REDIS_OK 0

/* When an error occurs, the err flag in a context is set to hold the type of
 * error that occurred. REDIS_ERR_IO means there was an I/O error and you
 * should use the "errno" variable to find out what is wrong.
 * For other values, the "errstr" field will hold a description. */
 * /
#define REDIS_READER_MAX_BUF (1024*16)  /* Default max unused reader buffer.

#define REDIS_ERR -1
#define REDIS_OK 0
#define REDIS_ERR_IO 1 /* Error in read or write */
#define REDIS_ERR_EOF 3 /* End of file */
#define REDIS_ERR_PROTOCOL 4 /* Protocol error */
#define REDIS_ERR_OOM 5 /* Out of memory */
#define REDIS_ERR_OTHER 2 /* Everything else... */
    
#define REDIS_REPLY_STRING 1   //返回字符串，查看str,len字段
#define REDIS_REPLY_ARRAY 2    //返回一个数组，查看elements的值（数组个数），通过element[index]的方式访问数组元素，每个数组元素是一个redisReply对象的指针
#define REDIS_REPLY_INTEGER 3  //返回整数，从integer字段获取值
#define REDIS_REPLY_NIL 4      //没有数据返回
#define REDIS_REPLY_STATUS 5   //表示状态，内容通过str字段查看，字符串长度是len字段
#define REDIS_REPLY_ERROR 6    //表示出错，查看出错信息，如上的str,len字段
    
REDIS_REPLY_STATUS      表示状态，内容通过str字段查看，字符串长度是len字段
REDIS_REPLY_ERROR       表示出错，查看出错信息，如上的str,len字段
REDIS_REPLY_INTEGER    返回整数，从integer字段获取值
REDIS_REPLY_NIL         没有数据返回
REDIS_REPLY_STRING      返回字符串，查看str,len字段
REDIS_REPLY_ARRAY       返回一个数组，查看elements的值（数组个数），通过element[index]的方式访问数组元素，每个数组元素是一个redisReply对象的指针

//连接：最简单的是第一个方法。  
redisContext *redisConnect(const char *ip, int port);  
redisContext *redisConnectWithTimeout(const char *ip, int port, const struct timeval tv);  
redisContext *redisConnectNonBlock(const char *ip, int port);  
redisContext *redisConnectBindNonBlock(const char *ip, int port, const char *source_addr);  
redisContext *redisConnectUnix(const char *path);  
redisContext *redisConnectUnixWithTimeout(const char *path, const struct timeval tv);  
redisContext *redisConnectUnixNonBlock(const char *path);  
redisContext *redisConnectFd(int fd);  
  
//查询：查询的返回类型为（redisReply *），该类型在hiredis.h中定义。  
void *redisvCommand(redisContext *c, const char *format, va_list ap);  
void *redisCommand(redisContext *c, const char *format, ...);  
void *redisCommandArgv(redisContext *c, int argc, const char **argv, const size_t *argvlen); 

int func_reply_cnt(redisReply *r)  
{  
        int res = 0;  
        size_t i = 0;  
        switch(r->type)  
        {     
        case REDIS_REPLY_ERROR:  
        case REDIS_REPLY_NIL:  
                res = -1;   
                break;  
  
        case REDIS_REPLY_STATUS:  
        case REDIS_REPLY_STRING:  
                printf("%s\n", r->str);  
                break;  
  
        case REDIS_REPLY_INTEGER:  
                printf("%lld\n", r->integer);  
                break;  
  
        case REDIS_REPLY_ARRAY:  
                for(i = 0; i < r->elements; i ++)   
                {     
                        reply_cnt(r->element[i]);  
                }     
                break;  
  
        default:  
                break;  
        }  
  
        return res;  
}
