哈希算法将任意长度的二进制值映射为固定长度的较小二进制值，这个小的二进制值称为哈希值。哈希值是一段数据唯一且极其紧凑的数值表示形式。如果散列一段明文而且哪怕只更改该段落的一个字母，随后的哈希都将产生不同的值。要找到散列为同一个值的两个不同的输入，在计算上是不可能的，所以数据的哈希值可以检验数据的完整性。

链表查找的时间效率为O(N)，二分法为log2N，B+ Tree为log2N，但Hash链表查找的时间效率为O(1)。

设计高效算法往往需要使用Hash链表，常数级的查找速度是任何别的算法无法比拟的，Hash链表的构造和冲突的不同实现方法对效率当然有一定的影响，然 而Hash函数是Hash链表最核心的部分，下面是几款经典软件中使用到的字符串Hash函数实现，通过阅读这些代码，我们可以在Hash算法的执行效率、离散性、空间利用率等方面有比较深刻的了解。

下面分别介绍几个经典软件中出现的字符串Hash函数。

●PHP中出现的字符串Hash函数

static unsigned long hashpjw(char *arKey, unsigned int nKeyLength)
{
unsigned long h = 0, g;
char *arEnd=arKey+nKeyLength; 

while (arKey < arEnd) {
h = (h << 4) + *arKey++;
if ((g = (h & 0xF0000000))) {
h = h ^ (g >> 24);
h = h ^ g;
}
}
return h;
}

●OpenSSL中出现的字符串Hash函数

unsigned long lh_strhash(char *str)
{
int i,l;
unsigned long ret=0;
unsigned short *s; 

if (str == NULL) return(0);
l=(strlen(str)+1)/2;
s=(unsigned short *)str; 

for (i=0; i
ret^=(s[i]<<(i&0x0f));
return(ret);
} 

/* The following hash seems to work very well on normal text strings 
* no collisions on /usr/dict/words and it distributes on %2^n quite 
* well, not as good as MD5, but still good. 
*/
unsigned long lh_strhash(const char *c)
{
unsigned long ret=0;
long n;
unsigned long v;
int r; 

if ((c == NULL) || (*c == '\0'))
return(ret);
/*
unsigned char b[16]; 
MD5(c,strlen(c),b); 
return(b[0]|(b[1]<<8)|(b[2]<<16)|(b[3]<<24)); 
*/ 

n=0x100;
while (*c)
{
v=n|(*c);
n+=0x100;
r= (int)((v>>2)^v)&0x0f;
ret=(ret(32-r));
ret&=0xFFFFFFFFL;
ret^=v*v;
c++;
} 

return((ret>>16)^ret);
}

●MySql中出现的字符串Hash函数

#ifndef NEW_HASH_FUNCTION 

/* Calc hashvalue for a key */
static uint calc_hashnr(const byte *key,uint length)
{
register uint nr=1, nr2=4; 

while (length--)
{
nr^= (((nr & 63)+nr2)*((uint) (uchar) *key++))+ (nr << 8);
nr2+=3;
} 

return((uint) nr);
} 

/* Calc hashvalue for a key, case indepenently */
static uint calc_hashnr_caseup(const byte *key,uint length)
{
register uint nr=1, nr2=4; 

while (length--)
{
nr^= (((nr & 63)+nr2)*((uint) (uchar) toupper(*key++)))+ (nr << 8);
nr2+=3;
} 

return((uint) nr);
}
#else
/* 
* Fowler/Noll/Vo hash 
* 
* The basis of the hash algorithm was taken from an idea sent by email to the 
* IEEE Posix P1003.2 mailing list from Phong Vo (kpv@research.att.com) and 
* Glenn Fowler (gsf@research.att.com). Landon Curt Noll (chongo@toad.com) 
* later improved on their algorithm. 
* 
* The magic is in the interesting relationship between the special prime 
* 16777619 (2^24 + 403) and 2^32 and 2^8. 
* 
* This hash produces the fewest collisions of any function that we've seen so 
* far, and works well on both numbers and strings. 
*/
uint calc_hashnr(const byte *key, uint len)
{
const byte *end=key+len;
uint hash; 

for (hash = 0; key < end; key++)
{
hash *= 16777619;
hash ^= (uint) *(uchar*) key;
} 

return (hash);
} 

uint calc_hashnr_caseup(const byte *key, uint len)
{
const byte *end=key+len;
uint hash; 

for (hash = 0; key < end; key++)
{
hash *= 16777619;
hash ^= (uint) (uchar) toupper(*key);
} 

return (hash);
}
#endif

Mysql中对字符串Hash函数还区分了大小写

●另一个经典字符串Hash函数

unsigned int hash(char *str)
{
register unsigned int h;
register unsigned char *p; 

for(h=0, p = (unsigned char *)str; *p ; p++)
h = 31 * h + *p; 

return h;
}