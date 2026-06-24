#ifndef CBUFFER_H
#define CBUFFER_H

#include "complex"

#define COMPLEX std::complex<double>

#include "buffer.h" // 包含实数缓冲区说明文件


std::ostream &operator<<(std::ostream & out, std::complex<double> & z);
std::istream &operator>>(std::istream & in, std::complex<double> & z);

// 定义复数缓存区的各类

// 复数缓存区抽象类
class cbuffer {
 public:
	size_t refnum; // 引用数，目的与buffer类一致
	cbuffer():refnum(1){};
	virtual void alloc(size_t num)=0; // 将缓存区的尺寸重新定位为num个复数
	virtual void release()=0; // 释放缓存区
	virtual COMPLEX& retrieve(size_t n)=0; // 获得第n个复数的引用
	virtual cbuffer* clone()=0; // 克隆一个与自己的内容一样的缓存区
	virtual cbuffer* newbuf(size_t n=0)=0; // 产生一个与自己类型一样的缓存区并返回指针
	COMPLEX&  operator[](size_t n){ // 重载[]运算符但只能取值不能赋值
		return retrieve(n);};
	virtual size_t len()=0; // 缓存区长度，以复数个数计
	cbuffer* set(size_t n, COMPLEX d); // 设第n个复数的值为d，如果引用数超过1
					// 则先克隆出一个新的缓存后再行操作
	friend std::ostream& operator<<(std::ostream& o, cbuffer& b);
};

std::ostream& operator<<(std::ostream& o, cbuffer& b);
cbuffer * getnewcbuffer(size_t n=0); // 根据缺省的复数缓存区类型产生一个新的缓存区
extern cbuffer *defcbuffer; // 指向缺省的复数缓存区变量

class memcbuffer : public cbuffer { // 复数内存缓存区类
 public:
	COMPLEX * buf; // 指向复数缓存的指针
	size_t length; // 缓存区的长度，以复数个数计
	memcbuffer():cbuffer(),buf(0),length(0){}; // 缺省构造函数
	memcbuffer(size_t lth):cbuffer(),length(lth){ // 构造长度为lth的复数缓存区
		if(lth > 0)
			buf = new COMPLEX[lth];
		else buf = 0; };
	~memcbuffer(){delete buf;} // 析构函数，释放内存
	void alloc(size_t num){ // 将缓存区容量重新定为长度为num个复数
		delete []buf;
		length = num;
		buf = new COMPLEX[num]; };
	COMPLEX& retrieve(size_t n){ // 获得第n个复数的引用
		return buf[n]; };
	void release() { delete []buf; length = 0; }; // 释放内存
	cbuffer* clone(); // 克隆一个内容一样的新缓存
	cbuffer* newbuf(size_t n=0){ // 产生一个类型同自己一样的具有长度n的新缓存
		return new memcbuffer(n);};
	size_t len(){return length;}; // 返回缓存的长度
};

class diskcbuffer : public cbuffer { // 复数磁盘缓存区子类
 public:
	COMPLEX buf;	// 当前的复数值
	size_t length;	// 缓存区长度
	size_t n;	// 当前复数值的序号
	FILE * tempfp; // 临时文件指针
	diskcbuffer():cbuffer(),buf(0.0),length(0),tempfp(0),n(0){}; // 缺省构造函数
	diskcbuffer(size_t lth); // 构造长度为lth个复数的缓存
	~diskcbuffer(); // 析构函数
	void alloc(size_t num); // 将缓存区尺寸重新定为num
	COMPLEX& retrieve(size_t n); // 取得第n个复数的引用
	void release(); // 关闭临时文件
	cbuffer* clone(); // 克隆一个内容相同的缓存
	cbuffer* newbuf(size_t n=0){return new diskcbuffer(n);}; // 产生一个子类相同的
						// 长度为n的缓存
	size_t len(){return length;}; // 返回缓存长度
};

#endif // CBUFFER_H