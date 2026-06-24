#ifndef VFUNC_H
#define VFUNC_H

#include <math.h>

#include "matrix.h"

class valgo	// 矩阵算法类
{
 private:
	DOUBLE yfactor;		// 乘因子，初始化为1
	DOUBLE xfactor;		// x轴放大因子，初始化为1
	DOUBLE addconst;	// 加和，初始化为0
	DOUBLE xshift;		// x平移量，初始化为0
 public:
	unsigned refnum;		// 引用数,初始化为1
	valgo():refnum(1),yfactor(1.0),xfactor(1.0),addconst(0.0),xshift(0.0){};
	 // 构造函数,产生y=x线性函数
	valgo(DOUBLE xs, DOUBLE xf,DOUBLE adc=0, DOUBLE yf=1):refnum(1),yfactor(yf),
		addconst(adc),xshift(xs),xfactor(xf){}; // 为子类的调用而存在的构造函数
	valgo(DOUBLE a):refnum(1),yfactor(0.0),xfactor(1.0),addconst(a),xshift(0.0){};
	 // 常函数的构造，输出结构与输入矩阵同样阶数的常数矩阵
	valgo(valgo & alg):yfactor(alg.yfactor),xfactor(alg.xfactor),
		addconst(alg.addconst),xshift(alg.xshift),refnum(1){}; // 拷贝构造函数
	virtual ~valgo(){};
	matrix cal(matrix & x); // 计算算法值
	virtual matrix& calculate(matrix & x)
		{return x;}; // 本身算法,将被继承子类改写, 返回的引用须与x同
	virtual valgo * clone(); // 克隆自己，必须被继承子类改写
	valgo * mul(DOUBLE a);	// 乘a
	valgo * add(DOUBLE a);	// 加a
	valgo * neg(); // 取负
	valgo * setxfactor(DOUBLE x);		// 设置x轴因子
	valgo * xroom(DOUBLE x);	// 将xfactor扩大x倍
	valgo * setxshift(DOUBLE x); // 设置xshift的值
	valgo * xshiftas(DOUBLE x);	// 从当前开始平移x
};

#ifndef FUNC_H
//enum	method {cadd,csub,cmul,cdiv,cpow,ccom}; // 枚举加减乘除乘方复合这四种运算
const int cadd = 1;
const int csub = 2;
const int cmul = 3;
const int cdiv = 4;
const int cpow = 5;
const int ccom = 6;
#endif

class valgojoin : public valgo // 结合算法
{
 public:
	valgo * leftalgo;	// 左算法，初始化为0
	valgo * rightalgo; // 右算法，初始化为0
	int met;	// 指明算法
	valgojoin(valgo * l, valgo * r, int m):leftalgo(l),
		rightalgo(r), met(m)
		{ if(leftalgo)
				leftalgo->refnum++;
		  if(rightalgo)
				rightalgo->refnum++;
		};
	valgojoin(valgojoin& alg):valgo(alg),
		leftalgo(alg.leftalgo),rightalgo(alg.rightalgo),met(alg.met){
			if(leftalgo)
				leftalgo->refnum++;
			if(rightalgo)
				rightalgo->refnum++;};
			// 拷贝构造函数
	virtual ~valgojoin() {
		if(leftalgo) {	// 如左或者右算法已经没有被引用，则删除
			leftalgo->refnum--;
			if(!leftalgo->refnum) delete leftalgo;
		}
		if(rightalgo) {
			rightalgo->refnum--;
			if(!rightalgo->refnum) delete rightalgo;
		}
	};
	virtual valgo * clone(); // 克隆自己
	virtual matrix& calculate(matrix& x);	// 实施结合算法
};

class valgofun : public valgo	// 函数算法
{
 public:
	matrix& (*f)(matrix&);	// 函数指针
	valgofun(matrix& (*fun)(matrix&)):f(fun){};	// 用函数指针进行初始化
	valgofun(valgofun& alg):valgo(alg),f(alg.f){}; // 拷贝构造函数
	virtual matrix& calculate(matrix& x);	// 实施函数算法
	virtual valgo * clone(); // 克隆自己
};

class valgofun1 : public valgo	// 标量到向量的函数算法
{
 public:
	matrix (*f)(DOUBLE);	// 标量到向量的函数指针
	valgofun1(matrix (*fun)(DOUBLE)):f(fun){};	// 用函数指针进行初始化
	valgofun1(valgofun1& alg):valgo(alg),f(alg.f){}; // 拷贝构造函数
	virtual matrix& calculate(matrix& x);	// 实施函数算法
	virtual valgo * clone(); // 克隆自己
};

class valgodiff : public valgo	// 微分方程组函数算法
{
 public:
	matrix (*f)(DOUBLE,matrix&);	// 表示f(t,y)的函数指针,y为向量
	DOUBLE t0;	// 初始变量
	matrix y0;	// 初值或称边界条件
	DOUBLE tnow;	// 最近值
	matrix ynow;	// 最近算得的y值
	valgodiff(matrix (*fun)(DOUBLE,matrix&), DOUBLE tt0, matrix& yy0):
		f(fun),t0(tt0),y0(yy0),tnow(tt0),ynow(yy0){};
	valgodiff(valgodiff& alg):valgo(alg),f(alg.f),t0(alg.t0),y0(alg.y0),
		tnow(alg.tnow),ynow(alg.ynow){};	// 拷贝构造函数
	virtual matrix& calculate(matrix& x);	// 实施函数算法
	virtual valgo * clone(); // 克隆自己
	matrix& calcul(DOUBLE t, DOUBLE eps=defaulterr);	// 标量算法
};

class vfunc {	// 矩阵函数类
 public:
	valgo * alg;	// 决定函数的算法

	vfunc();	// 缺省构造函数
	vfunc(DOUBLE a);	// 常函数的构造函数
	vfunc(matrix& (*fun)(matrix&));	// 函数指针的构造函数
	vfunc(matrix (*fun)(DOUBLE)); // 标量到向量的函数指针构造函数
	vfunc(const vfunc & fn);	// 拷贝构造函数
	vfunc(valgo * a):alg(a){} // 算法构造函数，使用要小心，不能将一个算法产生
									// 两个函数，除非自己控制引用数的增加
	virtual ~vfunc() {		// 析构函数
		if(alg) {
			alg->refnum--;	// 引用数减一，如再无其它引用，则删除算法
			if(!alg->refnum)
				delete alg;
		}
	};

	matrix operator()(matrix& x){return alg->cal(x);}; // 计算x的函数值
	vfunc& operator=(const vfunc& fn);	// 赋值运算符
	vfunc& operator=(matrix& (*fn)(matrix&)); // 用函数指针的赋值运算符
	vfunc& operator=(DOUBLE a); // 常函数的赋值运算符

	vfunc& operator+=(vfunc& fn);	// 自身加一个函数
	vfunc& operator+=(DOUBLE a){alg=alg->add(a);return (*this);};//自身加一个常数
	vfunc& operator+=(matrix& (*f)(matrix&)); // 自身加一个函数指针
	vfunc operator+(vfunc& fn);	// 相加产生新函数
	vfunc operator+(DOUBLE a);	// 与常数相加产生新函数
	friend vfunc operator+(DOUBLE a, vfunc& f); // 同上但常数在前
	vfunc operator+(matrix& (*f)(matrix&)); // 加一个函数指针产生新函数
	friend vfunc operator+(matrix& (*f)(matrix&),vfunc& fn);
		 // 同上但函数指针在前

	vfunc& neg(); // 自身取负
	vfunc operator-(); // 产生负函数

	vfunc& operator-=(vfunc& fn); // 自身减一个函数
	vfunc& operator-=(DOUBLE a){alg=alg->add(-a);return (*this);};
				//自身减一个常数
	vfunc& operator-=(matrix& (*f)(matrix&)); // 自身减一个函数指针
	vfunc operator-(vfunc& fn);	// 相减产生新函数
	vfunc operator-(DOUBLE a);	// 与常数相减产生新函数
	friend vfunc operator-(DOUBLE a, vfunc& f); // 同上但常数在前
	vfunc operator-(matrix& (*f)(matrix&)); // 减一个函数指针产生新函数
	friend vfunc operator-(matrix& (*f)(matrix&),vfunc& fn); // 函数指针减函数

	vfunc& operator*=(vfunc& fn);	// 自身乘一个函数
	vfunc& operator*=(DOUBLE a){alg=alg->mul(a);return (*this);};//自身乘一个常数
	vfunc& operator*=(matrix& (*f)(matrix&)); // 自身乘一个函数指针
	vfunc operator*(vfunc& fn);	// 相乘产生新函数
	vfunc operator*(DOUBLE a);	// 与常数相乘产生新函数
	friend vfunc operator*(DOUBLE a, vfunc& f); // 同上但常数在前
	vfunc operator*(matrix& (*f)(matrix&)); // 乘一个函数指针产生新函数
	friend vfunc operator*(matrix& (*f)(matrix&),vfunc& fn); // 函数指针乘函数

	vfunc& operator/=(vfunc& fn);	// 自身除以一个函数
	vfunc& operator/=(DOUBLE a){alg=alg->mul(1.0/a);return (*this);
			};//自身除以常数
	vfunc& operator/=(matrix& (*f)(matrix&)); // 自身除以一个函数指针
	vfunc operator/(vfunc& fn);	// 相除产生新函数
	vfunc operator/(DOUBLE a);	// 与常数相除产生新函数
	friend vfunc operator/(DOUBLE a, vfunc& f); // 常数除以函数
	vfunc operator/(matrix& (*f)(matrix&)); // 除以一个函数指针产生新函数
	friend vfunc operator/(matrix& (*f)(matrix&),vfunc& fn); // 函数指针除以函数

	vfunc operator()(vfunc & fn);	// 复合函数，产生新的函数

	void setxfactor(DOUBLE a);	// 设置x因子为a
	void xroom(DOUBLE a);	  // x方向扩大a倍
	void setxshift(DOUBLE a);	// 设置函数沿x轴平移a
	void shiftxas(DOUBLE a); // 函数沿x轴右移a
};

class vfuncdiff : public vfunc	// 微分方程函数
{
 public:
	vfuncdiff(matrix (*fun)(DOUBLE,matrix&), DOUBLE t0, matrix& y0):
		vfunc(new valgodiff(fun,t0,y0)){};	// 构造函数，fun为微分方程右端函数
				// 的函数指针，t0起始时间，y0为起始值
	matrix& operator()(DOUBLE t) {
		return ((valgodiff*)alg)->calcul(t); };
};

class linemodel  // 线性动态观测系统模型
					// 用作测试卡尔曼滤波器而产生仿真观测数据
{
 public:
	matrix	x;		// 当前状态变量
	matrix	a;		// 当前状态转移矩阵
	matrix	h;		// 当前观测矩阵
	gassvector w;	// 模型噪声源
	gassvector v;	// 观测噪声源
	matrix	y;		// 当前观测向量
	linemodel(matrix& va, matrix & vh, matrix & q, matrix & r, matrix & vx);
			// 构造函数，va初始状态转移矩阵，vh初始观测矩阵，q当前模型噪声协
			// 方差阵，r当前观测噪声协方差阵，vx初始状态变量
	void setdata(matrix& va, matrix & vh, matrix & q, matrix & r);
	void seta(matrix& va);
	void seth(matrix& vh);
	void setq(matrix& q);
	void setr(matrix& r);
	matrix & next();	// 计算下一级x值并返回新的x对应的y值
};

class kalman 	// 卡尔曼滤波类
{
 public:
	matrix	x;		// 当前状态变量的估值
	matrix	p;		// 当前估值的误差协方差阵
	matrix	a;		// 当前状态转移矩阵
	matrix	h;		// 当前观测矩阵
	matrix	y;		// 当前观测向量
	matrix	q;		// 当前模型噪声协方差阵
	matrix	r;		// 当前观测噪声协方差阵
	kalman(matrix &va,matrix& vq,matrix& vr,matrix& vh,matrix& vx,matrix& vp);
		// 构造函数，va为状态转移矩阵，vh观测矩阵，vq模型噪声协方差阵，
		// vr当前观测噪声协方差阵，vx初始状态变量估值，vp初始估值协方差阵
	void setdata(matrix &va,matrix& vq,matrix& vr,matrix& vh);
		// 为时变系统随时设置系统参数，va为状态转移矩阵，vh观测矩阵，vq模型噪
		// 声协方差阵，vr当前观测噪声协方差阵
	void seta(matrix& va);
	void seth(matrix& vh);
	void setq(matrix& vq);
	void setr(matrix& vr);
	matrix& next(matrix& y);	// 根据下一个观测值获得新的状态变量的估值
};

inline vfunc operator+(DOUBLE a, vfunc& f) // 常数加函数
{	return f+a; }

inline vfunc operator+(matrix& (*f)(matrix&),vfunc& fn) // 函数指针加函数
{	return fn+f;}

vfunc operator-(DOUBLE a, vfunc& f); // 常数减函数
vfunc operator-(matrix& (*f)(matrix&),vfunc& fn); // 函数指针减函数

inline vfunc operator*(DOUBLE a, vfunc& f) // 常数乘函数
{	return f*a; }
inline vfunc operator*(matrix& (*f)(matrix&),vfunc& fn) // 函数指针乘函数
{	return fn*f;}

vfunc operator/(DOUBLE a, vfunc& f); // 常数除以函数
vfunc operator/(matrix& (*f)(matrix&),vfunc& fn); // 函数指针除以函数

class regress	// 多元线性回归分析类
{
 public:
	matrix a;	// 算出的回归系数m+1维向量
	matrix v;	// 算出的偏相关系数m维向量
	DOUBLE q;	// 偏差平方和
	DOUBLE s;	// 平均标准偏差
	DOUBLE r;	// 复相关系数
	DOUBLE u;	// 回归平方和
	regress(matrix& x, matrix& y);	// x为mXn维矩阵，y为n个观测值
	DOUBLE operator()(matrix& x); // 回归后的线性函数，x是自变量
};

#endif // VFUNC_H