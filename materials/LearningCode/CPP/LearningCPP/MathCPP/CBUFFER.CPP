#include <stdio.h>
#include <string.h>
#include <fstream>

#include "cbuffer.h"

std::ostream &operator<<(std::ostream & out, std::complex<double> & z)
{
	out<<"("<<std::real(z)<<","<<std::imag(z)<<")";
	return out;
}
std::istream &operator>>(std::istream & in, std::complex<double> & z)
{
	double x,y;
//	in >> "(" >> x >> "," >> y >> ")";
	char a;
	in >> a >> x >> a >> y >> a;
	z = std::complex<double>(x,y);
	return in;
}

std::ostream& operator<<(std::ostream& o, cbuffer& b)
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

// 将第n个值设为d，如果引用数大于1，则克隆出一个新的
// 复数缓存区，再作设置，返回作了设置的缓存区指针
cbuffer* cbuffer::set(size_t n, COMPLEX d)
{
	DOUBLE x,y;
	if(doadjust) {
		x = std::real(d);
		y = std::imag(d);
		d = COMPLEX(adjust(x),adjust(y));
	}
	if(refnum == 1) {
		retrieve(n) = d;
		return this;
	}
	refnum --;
	cbuffer * b;
	b = clone();
	(b->retrieve(n)) = d;
	return b;
}

// 克隆一个完全一样的缓存区
cbuffer* memcbuffer::clone()
{
	cbuffer* b;
	b = new memcbuffer(length);
	if(length > 0)
		for(size_t i=0; i<length; i++)
			(b->retrieve(i)) = (*this)[i];
	return b;
}

// 磁盘缓存区构造函数，打开临时文件
diskcbuffer::diskcbuffer(size_t lth):cbuffer(),length(lth),n(0)
{
//	tempfp = tmpfile();
	tmpfile_s(&tempfp);
}


// 析构函数，关闭临时文件
diskcbuffer::~diskcbuffer()
{
	fclose(tempfp);
}

// 将缓存区尺寸改为num
void diskcbuffer::alloc(size_t num)
{
	length = num;
	if(!tempfp)
//		tempfp = tmpfile();
	tmpfile_s(&tempfp);
	n = 0;
}

// 释放缓存区，关闭临时文件
void diskcbuffer::release()
{
	fclose(tempfp);
	buf = 0.0;
	length = 0;
}

// 返回第i个复数的引用，先将当前buf中的内容写入临时文件
COMPLEX& diskcbuffer::retrieve(size_t i)
{
	long off;
	off = (long)n*sizeof(COMPLEX);
	fseek(tempfp, off, SEEK_SET);
	fwrite(&buf, sizeof(COMPLEX), 1, tempfp);
	off = (long)i*sizeof(COMPLEX);
	fseek(tempfp, off, SEEK_SET);
	fread(&buf, sizeof(COMPLEX), 1, tempfp);
	n = i;
	return buf;
}

// 克隆一个内容完全相同的磁盘缓存区
cbuffer* diskcbuffer::clone()
{
	cbuffer* b;
	b = new diskcbuffer(length);
	if(length > 0)
		for(size_t i=0; i<length; i++)
			(b->retrieve(i)) = (*this)[i];
	return b;
}
