#include <stdio.h>
#include <string.h>
#include <fstream>

// 功能：实现buffer.h中的各个缓冲类的函数

#include "buffer.h"
#include "cbuffer.h"

DOUBLE defaulterr = 1e-8; 	// 缺省的误差调整值
int doadjust = 1;		// 决定数据是否根据误差值自动向整数靠扰，
							// 其实是向小数点后三位靠扰

/* 下面的六个指针分别初始化指向产生六个缓冲区长度为空的六个
	缓冲区，其中两个实数缓冲区，两个长整数缓冲区，两个复数缓冲区
  其中实数，长整数和复数这三种数据类型的每一种都有磁盘和内存两种
  缓冲区。如果数据较小时可以存放在内存中，而数据量很大时则存放在
  临时文件构成的磁盘缓冲区中。这六个空缓冲区并不真正用来存放数据，
  而是用来产生出新的与自己一样的缓冲区，因为它们都是抽象类buffer的
  子类，所以都将被settomemory函数或者settodisk函数赋给三个buffer类的
  指针，使得getnewbuffer,getnewcbuffer和getnewlbuffer三个函数能够从这些
  缓冲区中用抽象类函数产生同样类型的缓冲区。
  总的想法是，在以后编写的计算各种矩阵或者行列式，或者其它的大量
  数据的运算时，只要调用一次settomemroy或者settodisk函数，那么用户
  不必改写一行代码，就可以照算不误，而存储媒体已经改变。
*/
static membuffer *memorybuffer = new membuffer; // 实数内存缓冲区指针
static diskbuffer *diskfilebuffer = new diskbuffer; // 实数磁盘缓冲区指针
static lmembuffer *memorylbuffer = new lmembuffer; // 长整数内存缓冲区指针
static ldiskbuffer *diskfilelbuffer = new ldiskbuffer; // 长整数磁盘缓冲区指针
static memcbuffer *memorycbuffer = new memcbuffer; // 复数内存缓冲区指针
static diskcbuffer *diskfilecbuffer = new diskcbuffer; // 复数磁盘缓冲区指针


/* 以下三个缺省缓冲区指针为全局变量，用户可以定义自己的缓冲区
	类，并在产生静态实例变量后将其指针赋予它们 */

buffer *defbuffer = memorybuffer; // 缺省实数缓冲区指针
lbuffer *deflbuffer = memorylbuffer; // 缺省长整数缓冲区指针
cbuffer *defcbuffer = memorycbuffer; // 缺省复数缓冲区指针


/* 产生一个新的容量为n个实数的实数缓冲区，返回此缓冲区指针，
	缓冲区的类型与defbuffer指向的变量的子类类型相同 */
buffer * getnewbuffer(size_t n){
	return defbuffer->newbuf(n);
}

/* 产生一个新的容量为n个长整数的长整数缓冲区，返回此缓冲区指针，
	缓冲区的类型与deflbuffer指向的变量的子类类型相同 */
lbuffer * getnewlbuffer(size_t n){
	return deflbuffer->newbuf(n);
}

/* 产生一个新的容量为n个复数的复数缓冲区，返回此缓冲区指针，
	缓冲区的类型与defcbuffer指向的变量的子类类型相同 */
cbuffer * getnewcbuffer(size_t n){
	return defcbuffer->newbuf(n);
}

// 将各实数，长整数和复数的缺省的缓冲区指针指向相应的磁盘缓冲区指针
void settodisk()
{
	defbuffer = diskfilebuffer;
	deflbuffer = diskfilelbuffer;
	defcbuffer = diskfilecbuffer;
}

// 将各实数，长整数和复数的缺省的缓冲区指针指向相应的内存缓冲区指针
void settomemory()
{
	defbuffer = memorybuffer;
	deflbuffer = memorylbuffer;
	defcbuffer = memorycbuffer;
}

std::ostream& operator<<(std::ostream& o, buffer& b)
{
	size_t maxitem = 5;
	for(size_t i=0; i<b.len(); i++) {
		o << b[i] << '\t';
		if(((i+1) % maxitem)==0)
			o << std::endl;
	}
	o << std::endl;
	return o;
}


// 将实数缓冲区的第n个实数的值设为d，并返回一个实数缓冲区的
// 指针，此指针一般就是指向自己，而在一个缓冲区的引用数多于一个时
// 此函数将先克隆出一个新的缓冲区，此新的缓冲区的引用数为1，并对此
// 新的缓冲区的第n个实数的值设为d，然后返加新缓冲区的指针
buffer* buffer::set(size_t n, DOUBLE d)
{
	// 如调整标志设置，则调整
	if(doadjust) adjust(d);
	if(refnum == 1) {	// 如果引用数为1，则将第n个实数的值设为d，并返回自己
			// 的指针
		retrieve(n) = d;
		return this;
	}
	refnum --;	// 引用数大于1，因此将本缓冲区的引用数减1
	buffer * b;
	b = clone();	// 克隆一个内容同自己一样的新缓冲区
	(b->retrieve(n)) = d; // 将新缓冲区的第n个实数的值设为d
	return b;	// 返回新缓冲区的指针
}

buffer* membuffer::clone() // 克隆一个内容和自己一样的实数内存缓冲区，并返回其指针
{
	buffer* b;
	b = new membuffer(length);	// 建立一个新的实数内存缓冲区，尺寸和自己的一样
	if(length > 0)	// 如果自己有内容，将全部拷贝到新产生的缓冲区中
		for(size_t i=0; i<length; i++)
			(b->retrieve(i)) = (*this)[i];
	return b;
}

// 实数磁盘缓冲区构造函数
diskbuffer::diskbuffer(size_t lth):buffer(),length(lth),n(0),buf(0.0)
{
//	tempfp = tmpfile();	// 打开一新的临时文件，指针赋给tempfp
	tmpfile_s(&tempfp);
}

// 实数磁盘缓冲区的析构函数
diskbuffer::~diskbuffer()
{
	fclose(tempfp);	// 关闭临时文件
}

void diskbuffer::alloc(size_t num) // 将磁盘缓冲区重新定为尺寸为num
{
	length = num;
	if(!tempfp)	 // 如果临时文件尚未打开，则打开临时文件
//		tempfp = tmpfile();
	tmpfile_s(&tempfp);
	n = 0;	// 当前缓冲区指针设在开始，就是0处
}

void diskbuffer::release() // 释放磁盘缓冲区
{
	fclose(tempfp);	// 关闭临时文件
	buf = 0.0;
	length = 0;
}

DOUBLE& diskbuffer::retrieve(size_t i) // 返回实数磁盘缓冲区的第i个实数
{
	long off;
	off = (long)n*sizeof(DOUBLE); // 计算出当前指针n对应的文件的偏移量
	fseek(tempfp, off, SEEK_SET); // 移动文件指针到给定的偏移量处
	fwrite(&buf, sizeof(DOUBLE), 1, tempfp); // 将当前的buf中值写到文件中
	off = (long)i*sizeof(DOUBLE); // 计算第i个实数在文件的什么位置
	fseek(tempfp, off, SEEK_SET); // 移动文件指针到给定的偏移量处
	fread(&buf, sizeof(DOUBLE), 1, tempfp); // 读回所需的实数值到buf中
	n = i;	// 并将n改为i
	return buf; // 返回读出的实数的引用
}

buffer* diskbuffer::clone() // 克隆一个新的内容与自己一样的实数磁盘缓冲区
{
	buffer* b;
	b = new diskbuffer(length); // 定位产生一个与自己长度一样的新缓冲区
	if(length > 0)	// 如果长度大于0，则自己的内容拷贝到新缓冲区
		for(size_t i=0; i<length; i++)
			(b->retrieve(i)) = (*this)[i];
	return b; // 返回新缓冲区的指针
}


// 将第n个长整数的值设为v，如果缓冲区的引用数大于1，
// 则会先克隆出新的缓冲区，然后再进行操作，并返回新缓冲区的指针
lbuffer* lbuffer::set(size_t n, long v)
{
	if(refnum == 1) { //  引用数为1
		retrieve(n) = v; // 将第n个长整数的值设为v
		return this;	// 返回自己
	}
	refnum --;		// 引用数大于1，引用数减1
	lbuffer * b;
	b = clone();	// 克隆出新的内容一样的长整数缓冲区
	(b->retrieve(n)) = v; // 将新缓冲区的第n个长整数值设为v
	return b;	// 返回新缓冲区的指针
}

std::ostream& operator<<(std::ostream& o, lbuffer& b)
{
	size_t maxitem = 5;
	for(size_t i=0; i<b.len(); i++) {
		o << b[i] << '\t';
		if(((i+1) % maxitem)==0)
			o << std::endl;
	}
	o << std::endl;
	return o;
}

lbuffer* lmembuffer::clone() // 克隆一个内容一新的长整数内存缓存区
{
	lbuffer* b;
	b = new lmembuffer(length);	// 产生长度与自己一样的长整数缓存区
	if(length > 0)	// 如果长度大于0，则将自己的内容拷贝过去
		for(size_t i=0; i<length; i++)
			(b->retrieve(i)) = (*this)[i];
	return b;	// 返回新缓存区的指针
}

// 长整数磁盘缓存区的构造函数
ldiskbuffer::ldiskbuffer(size_t lth):lbuffer(),length(lth),n(0),buf(0)
{
//	tempfp = tmpfile();	// 打开临时文件
	tmpfile_s(&tempfp);
}

// 长整数磁盘缓存区的析构函灵敏
ldiskbuffer::~ldiskbuffer()
{
	fclose(tempfp);	// 关闭临时文件
}

// 将长度定改为num
void ldiskbuffer::alloc(size_t num)
{
	length = num;
	if(!tempfp) // 如临时文件尚未打开，则打开
//		tempfp = tmpfile();
	tmpfile_s(&tempfp);
	n = 0;
}

// 长整数磁盘缓存区释放
void ldiskbuffer::release()
{
	fclose(tempfp);	// 关闭临时文件
	buf = 0;
	length = 0;
}

// 检索第i个值的引用
long& ldiskbuffer::retrieve(size_t i)
{
	long off;
	off = (long)n*sizeof(long); // 先将当前的内容保存
	fseek(tempfp, off, SEEK_SET);
	fwrite(&buf, sizeof(long), 1, tempfp);
	off = (long)i*sizeof(long);	// 再取出第i个长整数到buf中
	fseek(tempfp, off, SEEK_SET);
	fread(&buf, sizeof(long), 1, tempfp);
	n = i;
	return buf;
}

// 克隆一个和内容和自己一样的长整数磁盘缓存区
lbuffer* ldiskbuffer::clone()
{
	lbuffer* b;
	b = new ldiskbuffer(length); // 产生长度和自己一样的缓存区
	if(length > 0)	// 如果自己内容不为空，拷贝内容到新的缓存区
		for(size_t i=0; i<length; i++)
			(b->retrieve(i)) = (*this)[i];
	return b;
}

DOUBLE adjust(DOUBLE & a) // 将实数调整为最靠近小数点后二位的实数
// 准则：如果一个实数距某个二位小数小于十的负八次幂，则调整为这个小数
// 否则不变
{
	DOUBLE b = floor(a*100.0+0.5)/100.0; // b是四舍五入到小数点后二位
	if(fabs(b-a)<defaulterr) // 如果舍入误差极小，则抛弃
		a = b;
	return a;
}

char * throwmessage(int l,const char * f, const char * c)
{
	static char a[100];
//	sprintf(a,"file:%s,line:%d\nmessage:%s\n",f,l,c);
	sprintf_s(a,100,"file:%s,line:%d\nmessage:%s\n",f,l,c);
	return a;
}

