#ifndef MATRIX_H

#define MATRIX_H

// 定义适合数学运算的实数matrix类，数据将存放在buffer类中

#include <iostream>
#include <stdio.h>
using namespace std;

// buffer.h包含实数缓存类buffer的定义
#include "buffer.h"

// 实数矩阵类，每个矩阵有rownum行，colnum列，总共rownum乘colnum个实数
// 下标一律从0开始起算，即存在第0行第0列
class matrix {
 public:
	buffer * buf;  // 缓存区指针，指向存放实数的地方，可以是任何媒体
		// buffer类是一抽象类，具体的子类可以将数据存放在内存或者
		// 临时文件中，用户还可以设计自己的缓存区子类
	size_t rownum, colnum;  // 矩阵的行数和列数
	bool	istrans;		// 转置标志，0为缺省，1为转置
	bool isneg;			// 为负标志，0为非负，1为取负
	bool issym;			// 对称标志，0为非对称，1为对称，不具有约束力
	// 在下面的所有构造函数中，缓存区指针b通常不设，缺省为0，这样程序将根据
	// 自动申请缺省的缓存区类型。如果用户定义了特殊的缓顾虑区类型，可以
	// 先申请后在构造函数中指明
	matrix(buffer * b=0); // 缺省构造函数，产生0行0列空矩阵
	matrix(size_t n, buffer * b=0); // 产生nX1列向量
	matrix(size_t r, size_t c, buffer * b=0); // 构造函数，产生r行c列矩阵
	matrix(const matrix& m); //拷贝构造函数，产生的矩阵内容同m相同，
		// 新产生的矩阵并不申请新的缓存，而是指向与m同样的缓存，
		// 只是它们共同指向的缓存的引用数增1，这样可以节省存储资源
	matrix(const char * filename, buffer * b=0); /* 文件构造函数，
		使用数据文件对矩阵初使化，数据文件为文本文件，格式：
		以小写的matrix关键词打头，然后是空格，再跟矩阵的行数，再跟
		空格，再跟列数，再跟空格，接着是内容，
		内容按行分，首先是行号，从1开始计数，行号后面紧跟着冒号：，
		然后是这行的数据，各个数据之间都用空格或者换行隔开 */
	matrix(void * data, size_t r, size_t c=1, buffer * b=0); /* 数据构造函数，
		行数为r,列数为c,data必须指向一存放实数的内存，按先列后行的
		顺序进行存放。构造函数进行数据拷贝，因此在构造函数完成后,
		data指向的数据区可以释放 */
	DOUBLE operator()(size_t r, size_t c)const; // 重载函数运算符()返回第r行第c列的实数，
	DOUBLE operator()(size_t r)const{return (*this)(r,0);}; // 取回第r行0列的实数
	virtual void set(size_t r, size_t c, DOUBLE v) {
		size_t l;
		v = isneg ? -v : v;
		l = istrans ? r+c*rownum : r*colnum+c;
		buf = buf->set(l, v); }; // 设置第r行第c列的值为v，
		// 在设置时如果缓存的引用数大于1，产生新的缓存
		// 因此每次设置返回的缓存区指针都要再赋给buf。
	void set(size_t r, DOUBLE v) {
		set(r,0,v);};	// 设置第r行0列的值为v,用作列向量
	virtual ~matrix(){ // 析构函数，如果缓存的引用数大于1，说明还有其它的
			// 矩阵在用此缓存，则不释放缓存，只是将引用数减1
		buf->refnum--;
		if(!buf->refnum) delete buf;};
	matrix& operator=(const matrix& m); // 重载赋值运算符，使矩阵的内容等于另一个矩阵
	matrix& operator=(DOUBLE a); // 通过赋值运算符将矩阵所有元素设为a
	matrix operator*(DOUBLE a)const;	// 重载乘法运算符，实现矩阵与数相乘，矩阵在前
				// 数在后
	matrix& operator*=(DOUBLE a);  // 重载*=运算符，功能同上，但*产生了一个新
			// 的矩阵，原矩阵不变，而本运算符修改了原矩阵
	friend matrix operator*(DOUBLE a, const matrix& m);
			// 数乘矩阵，但数在前矩阵在后
	matrix operator+(const matrix& m)const; // 矩阵相加，原二矩阵不变并产生一个新的和矩阵
	matrix& operator+=(const matrix &m); // 矩阵相加，不产生新矩阵，有一矩阵的内容改变为和

	matrix operator+(DOUBLE a);	// 矩阵加常数，指每一元素加一固定的常数，产生
											// 新矩阵，原矩阵不变
	friend matrix operator+(DOUBLE a, matrix& m); // 常数加矩阵，产生新的和矩阵
	matrix& operator+=(DOUBLE a);	// 矩阵自身加常数，自身内容改变

	matrix operator*(const matrix& m)const; // 矩阵相乘，产生出新的矩阵
	matrix& operator*=(const matrix& m); // 矩阵相乘，结果修改原矩阵

	matrix operator-() const; // 矩阵求负，产生新的负矩阵
	matrix & trans(){
		size_t r = rownum; rownum = colnum; colnum = r;
		istrans = !istrans; return (*this);}; // 矩阵自身转置
	matrix t(); // 矩阵转置，产生新的矩阵
	matrix & neg(){isneg = !isneg; return (*this);}; // 将自己求负

	matrix operator-(const matrix& m); // 矩阵相减，产生新的矩阵
	matrix& operator-=(matrix& m); // 矩阵相减，结果修改原矩阵
	matrix operator-(DOUBLE a) 	// 矩阵减常数，指每一元素减一固定的常数，产生
											// 新矩阵，原矩阵不变
	{ return (*this)+(-a); };
	friend matrix operator-(DOUBLE a, matrix& m); // 常数减矩阵，产生新的矩阵
	matrix& operator-=(DOUBLE a)	// 矩阵自身减常数，自身内容改变
	{ return operator+=(-a);};

	int isnear(const matrix& m, double e = defaulterr)const; // 检查两个矩阵是否近似相等,
								// 如近似相等则返回1，否则返回0，e为允许误差
	int isnearunit(double e = defaulterr) const; // 检查矩阵是否近似为单位矩阵
								// 如是则返回1，否则返回0，e为允许误差

	matrix row(size_t r); // 提取第r行行向量
	matrix col(size_t c); // 提取第c列列向量

	void checksym();	// 检查和尝试调整矩阵到对称矩阵

	void swapr(size_t r1, size_t r2, size_t k=0); // 交换两行,只交换k列以上
	void swapc(size_t c1, size_t c2, size_t k=0); // 交换两列,只交换k行以上
	void swap(size_t r1, size_t c1, size_t r2, size_t c2){ // 交换两个元素
		DOUBLE a; a=(*this)(r1,c1);set(r1,c1,(*this)(r2,c2));
		set(r2,c2,a);};
	DOUBLE maxabs(size_t &r, size_t &c, size_t k=0);
		 // 求主元值和位置，即k行k列之后的最大元素，
		// 元素所在的行列将放在r,c中，返回此元素值
	size_t zgsxy(matrix & m, int fn=0);	// 主高斯消元运算
	matrix& operator/=(matrix m); // 用主高斯消元法求解线性方程的解
		// 矩阵本身为系数矩阵，m为常数向量，返回解向量
	matrix operator/(matrix m); // 左乘m的逆矩阵

	matrix& inv();		// 用全选主元高斯-约当法求逆矩阵
	matrix operator~();	// 求逆矩阵，但产生新矩阵
	friend matrix operator/(DOUBLE a, matrix& m); // 求逆矩阵再乘常数
	matrix& operator/=(DOUBLE a); // 所有元素乘a的倒数，自身改变
	matrix operator/(DOUBLE a); // 所有元素乘a的倒数，产生新的矩阵
	matrix cholesky(matrix& d);	// 用乔里斯基分解法求对称正定阵的线性
		// 方程组ax=d返回方程组的解，本身为a，改变为分解阵u,d不变

	DOUBLE det(DOUBLE err=defaulterr);		// 求行列式的值
	size_t rank(DOUBLE err=defaulterr);		// 求矩阵的秩
	friend istream& operator<<(istream& o, matrix& m);
	friend ostream& operator>>(ostream& in, matrix& m);

	void house(buffer & b, buffer & c); // 用豪斯荷尔德变换将对称阵变为对称三对
											// 角阵，b返回主对角线元素，c返回次对角线元素
	void trieigen(buffer& b, buffer& c, size_t l=600, DOUBLE eps=defaulterr);
						// 计算三对角阵的全部特征值与特征向量
	matrix eigen(matrix & eigenvalue, DOUBLE eps=defaulterr, size_t l=600);
		 // 计算对称阵的全部特征向量和特征值
		// 返回按列排放的特征向量，而eigenvalue将返回一维矩阵，为所有特征值
		// 组成的列向量
	int ispositive();		// 判定矩阵是否对称非负定阵，如是返回1，否则返回0

	void hessenberg();	// 将一般实矩阵约化为赫申伯格矩阵
	void qreigen(matrix & a, matrix & b, size_t l=600, DOUBLE eps=defaulterr);
		// 求一般实矩阵的所有特征根
		// a和b均返回rownum行一列的列向量矩阵，返回所有特征根的实部和虚部

	friend std::ostream& operator<<(std::ostream& o,const matrix& m);
	friend std::istream& operator>>(std::istream& in, matrix& m);
	virtual DOUBLE value(size_t r, size_t c) const{ // 返回第r行c列的数值，不做行列检查
		DOUBLE a;
		a = istrans ? (*buf)[r+c*rownum] : (*buf)[r*colnum + c];
		return isneg ? -a : a;
	};
 protected:
	DOUBLE detandrank(size_t & r, DOUBLE err);	// 求方阵的行列式和秩
};

inline matrix operator*(DOUBLE a,const matrix& m) {
	return m*a;
};

inline matrix operator+(DOUBLE a,matrix& m) { // 常数加矩阵，产生新的和矩阵
	return m+a;
};

matrix operator-(DOUBLE a, matrix& m); // 常数减矩阵，产生新的矩阵
matrix operator/(DOUBLE a, matrix& m); // 求逆矩阵再乘常数


//friend std::ostream& operator<<(std::ostream& o, matrix& m);
//std::istream& operator>>(std::istream& in, matrix& m);

matrix unit(size_t n);	// 产生n阶单位矩阵

DOUBLE gassrand(int rr=0);	// 返回一零均值单位方差的正态分布随机数
			// rr为种子

class gassvector //返回零均值给定协方差阵的正态随机向量的类
{
 public:
	matrix a;	// a是增益矩阵，由协方差矩阵算出
	gassvector(matrix & r);	//r必须是正定对称阵，为正态随机向量的协方差
	matrix operator()(matrix & r); // 返回给定协方差矩阵的正态随机向量
	matrix operator()();	// 返回已设定的协方差矩阵的正态随机向量
	void setdata(matrix & r); // 根据协方差矩阵设置增益矩阵
};

DOUBLE lineopt(matrix& a,matrix& b, matrix& c, matrix & x);
	// 线性规划最优点寻找程序，a为mXn不等式约束条件左端系数矩阵，b为不等式约束
	// 条件的右端项，为m维向量，c为目标函数系数，n维向量，x返回极小点，为n维向量

#endif // MATRIX_H