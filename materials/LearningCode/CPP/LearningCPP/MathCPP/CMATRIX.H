#ifndef CMATRIX_H

#define CMATRIX_H

#include <iostream>
//#include <iomanip.h>
#include <stdio.h>

#include "cbuffer.h"
#include "matrix.h"

#ifndef M_PI

#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.785398163397448309616
#define M_1_PI      0.318309886183790671538
#define M_2_PI      0.636619772367581343076

#endif // M_PI

// 定义复矩阵类
class cmatrix {
 public:
	cbuffer * buf;	// 指向复矩阵缓存类的指针
	size_t rownum, colnum;	// 矩阵的行数与列数
	bool istrans;	// 转置标记
	bool isneg;		// 取负标记
	bool isconj;	// 共轭标记
	cmatrix(cbuffer * b=0); // 缺省构造函数，产生0行0列空矩阵
	cmatrix(size_t n, cbuffer * b=0); // 产生n维列向量
	cmatrix(size_t r, size_t c, cbuffer * b=0); // 构造函数，构造一个空的复矩阵
	cmatrix(const cmatrix& m);	// 拷贝构造函数
	cmatrix(const char * filename, cbuffer * b=0); // 从数据文件构造一个复矩阵
	cmatrix(void * data, size_t r, size_t c=1, cbuffer * b=0);
		 // 从内存数据产生复矩阵
	cmatrix(matrix& m, cbuffer * b=0); // 由实矩阵产生一个内容相同的复矩阵
	cmatrix(matrix& mr, matrix& mi, cbuffer * b=0); // 两个实矩阵成为实部和虚部
												// 构成一复矩阵
	matrix real()const;	// 由矩阵的实部生成实矩阵
	matrix image()const; // 由矩阵的虚部生成实矩阵
	COMPLEX operator()(size_t r, size_t c); // 重载函数运算符，返回第r行c列的值
	COMPLEX operator()(size_t r);	// 重载函数运算符，返回第r行0列的值，用于向量
	virtual void set(size_t r, size_t c, COMPLEX v) { // 将第r行c列的值设为v
		size_t l;
		v = isneg ? -v : v;
		v = isconj ? std::conj(v) : v;
		l = istrans ? r+c*rownum : r*colnum+c;
		buf = buf->set(l, v); };
	void set(size_t r, COMPLEX v) {
		set(r,0,v);};	// 设置向量中第r个元素的值
	virtual ~cmatrix(){ // 析构函数
		buf->refnum--;	// 缓存的引用数减1
		if(!buf->refnum) delete buf;}; // 如果缓存的引用数为0，则释放缓存
	cmatrix& operator=(const cmatrix& m);	// 重载赋值运算
	cmatrix& operator=(matrix& m); // 将实矩阵赋给复矩阵
	cmatrix& operator=(COMPLEX a); // 将矩阵所有元素设为常数a
	cmatrix operator*(COMPLEX a);	// 数乘矩阵，产生新的矩阵
	cmatrix& operator*=(COMPLEX a); // 数乘矩阵，只是改动原矩阵
	friend cmatrix operator*(COMPLEX a, cmatrix& m); // 数乘矩阵
	cmatrix operator+(COMPLEX a); // 矩阵加常数，产生新矩阵
	cmatrix& operator+=(COMPLEX a); // 矩阵加常数，更动原矩阵
	friend cmatrix operator+(COMPLEX a, cmatrix& m); // 矩阵加常数，常数在前
	cmatrix operator+(const cmatrix& m)const; // 矩阵加法，产生新的矩阵
	cmatrix& operator+=(const cmatrix &m); // 矩阵加法，结果取代原矩阵

	cmatrix operator*(cmatrix& m);	// 矩阵乘法，产生并返回新的矩阵
	cmatrix& operator*=(cmatrix& m);	// 矩阵乘法，只是更动原矩阵

	cmatrix operator-()const; // 矩阵求负，产生新的矩阵
	cmatrix& neg();	// 将自己求负，不产生新的矩阵

	cmatrix	t();		// 矩阵转置，产生新的矩阵
	cmatrix& trans()	// 矩阵自身转置
		{  size_t r = rownum; rownum = colnum; colnum = r; // 交换行数和列数
		istrans = !istrans; return (*this);};
	cmatrix&	conj()	// 矩阵共轭，原矩阵更动
		{isconj = !isconj; return (*this);};
	cmatrix	operator!();		// 矩阵共轭，产生新的矩阵
	cmatrix& transconj()		// 矩阵自身共轭转置
		{ isconj = !isconj; trans(); return (*this);};
	cmatrix	tc();		// 矩阵共轭转置，产生新的矩阵

	cmatrix operator-(cmatrix& a);	// 矩阵相减
	cmatrix& operator-=(cmatrix& a); // 矩阵自身减矩阵
	cmatrix& operator-=(COMPLEX a); // 矩阵自身减常数
	cmatrix operator-(COMPLEX a);	// 矩阵减常数，产生新的矩阵
	friend cmatrix operator-(COMPLEX a, cmatrix m); // 常数减矩阵，产生新的矩阵

	void swapr(size_t r1, size_t r2, size_t k=0); // 交换两行,只交换k列以上
	void swapc(size_t c1, size_t c2, size_t k=0); // 交换两列,只交换k行以上
	void swap(size_t r1, size_t c1, size_t r2, size_t c2){ // 交换两个元素
		COMPLEX a; a=(*this)(r1,c1);set(r1,c1,(*this)(r2,c2));
		set(r2,c2,a);};
	DOUBLE maxabs(size_t &r, size_t &c, size_t k=0);
		 // 求主元值和位置，即k行k列之后的最大元素，
		// 元素所在的行列将放在r,c中，返回此元素值
	size_t zgsxy(cmatrix & m, int fn=0);	// 主高斯消元运算
	cmatrix& operator/=(cmatrix m); // 用主高斯消元法求解线性方程的解
		// 矩阵本身为系数矩阵，m为常数向量，返回解向量

	cmatrix& inv();		// 用全选主元高斯-约当法求逆矩阵
	cmatrix operator~();	// 求逆矩阵，但产生新矩阵
	friend cmatrix operator/(COMPLEX a, cmatrix& m); // 求逆矩阵再乘常数
	cmatrix& operator/=(COMPLEX a); // 所有元素乘a的倒数，自身改变
	cmatrix operator/(COMPLEX a); // 所有元素乘a的倒数，产生新的矩阵

	cmatrix& fft(int l=0);


	friend std::ostream& operator<<(std::ostream& o, cmatrix& m); // 矩阵的流输出
	friend std::istream& operator>>(std::istream& in, cmatrix& m); // 矩阵的流输入

	virtual COMPLEX value(size_t r, size_t c)const { // 返回第r行c列的复数值
		COMPLEX v;
		v = istrans ? (*buf)[r+c*rownum] : (*buf)[r*colnum+c];
		v = isneg ? -v : v;
		v = isconj ? std::conj(v) : v;
		return v; };
};

inline cmatrix operator*(COMPLEX a, cmatrix& m) {
	return m*a;
};

inline cmatrix operator+(COMPLEX a, cmatrix& m){ // 矩阵加常数，常数在前
	return m+a;
};

cmatrix operator/(COMPLEX a, cmatrix& m); // 求逆矩阵再乘常数


std::ostream& operator<<(std::ostream& o, cmatrix& m);
std::istream& operator>>(std::istream& in, cmatrix& m);

cmatrix cunit(size_t n);	// 产生n阶单位复矩阵

cmatrix fft(matrix& m);	// 对实向量作fft变换
matrix convolute(matrix&a, matrix& b);	// 求矩阵a与b的离散卷积

#endif // CMATRIX_H