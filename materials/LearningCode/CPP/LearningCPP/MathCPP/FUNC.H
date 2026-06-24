#ifndef FUNC_H
#define FUNC_H

#include <math.h>

#include "matrix.h"
#include "cmatrix.h"

#ifndef DOUBLE
#define DOUBLE double
#endif

class cmatrix;

DOUBLE gamma(DOUBLE x); // 计算伽马函数
DOUBLE gamma2(DOUBLE a, DOUBLE x); // 计算不完全伽马函数
DOUBLE erf1(DOUBLE x); // 计算误差函数
DOUBLE beta(DOUBLE a,DOUBLE b,DOUBLE x); // 计算不完全贝塔函数
DOUBLE gass(DOUBLE a,DOUBLE d,DOUBLE x); // 给定均值a，标准差d的正态分布函数
DOUBLE student(DOUBLE t, size_t n);	// t-分布函数
DOUBLE chii(DOUBLE x,size_t n); // X-方分布函数
DOUBLE fdisp(DOUBLE f,size_t n1,size_t n2); // F-分布函数
DOUBLE integral(DOUBLE (*f)(DOUBLE),DOUBLE a, DOUBLE b, DOUBLE eps=defaulterr);
 // 函数f,在(a,b)区间积分,采用勒让德高斯求积法
DOUBLE sinn(DOUBLE x);	// 正弦积分函数
DOUBLE coss(DOUBLE x);	// 余弦积分函数
DOUBLE expp(DOUBLE x);	// 指数积分函数
DOUBLE getroot(DOUBLE (*f)(DOUBLE), DOUBLE x0=0.9, DOUBLE eps=defaulterr);
 // 求函数f(x)=0在x0附近的根，返回此根，如找不到根，则丢出例外
class algo;
DOUBLE getroot(algo & alg, DOUBLE x0=0.9, DOUBLE eps=defaulterr);
 // 同上，但函数为algo类变量


class algo	// 算法类
{
 public:
	DOUBLE yfactor;		// 乘因子，初始化为1
	DOUBLE xfactor;		// x轴放大因子，初始化为1
	DOUBLE addconst;	// 加和，初始化为0
	DOUBLE xshift;		// x平移量，初始化为0
	unsigned refnum;		// 引用数,初始化为1
	algo():refnum(1),yfactor(1.0),xfactor(1.0),addconst(0.0),xshift(0.0){};
	 // 构造函数,产生y=x线性函数
	algo(DOUBLE xs, DOUBLE xf,DOUBLE adc=0, DOUBLE yf=1):refnum(1),yfactor(yf),
		addconst(adc),xshift(xs),xfactor(xf){};
	algo(DOUBLE a):refnum(1),yfactor(0.0),xfactor(1.0),addconst(a),xshift(0.0){};
	 // 常函数的构造
	algo(const algo & alg):yfactor(alg.yfactor),xfactor(alg.xfactor),
		addconst(alg.addconst),xshift(alg.xshift),refnum(1){}; // 拷贝构造函数
	virtual ~algo(){}; // 虚析构函数
	DOUBLE cal(DOUBLE x); // 计算算法值
	virtual DOUBLE calculate(DOUBLE x)
		{return x;}; // 本身算法,将被继承子类改写
	virtual algo * clone(); // 克隆自己，必须被继承子类改写
	algo * mul(DOUBLE a);	// 乘a
	algo * add(DOUBLE a);	// 加a
	algo * neg(); // 取负
	algo * setxfactor(DOUBLE x);		// 设置x轴因子
	algo * xroom(DOUBLE x);	// 将xfactor扩大x倍
	algo * setxshift(DOUBLE x); // 设置xshift的值
	algo * xshiftas(DOUBLE x);	// 从当前开始平移x
	DOUBLE integ(DOUBLE a, DOUBLE b, DOUBLE eps = defaulterr); // 在(a,b)区间积分
			// 采用勒让德高斯求积法
};

enum  class	method {cadd,csub,cmul,cdiv,cpow,ccom}; // 枚举加减乘除乘方复合这四种运算

class algojoin : public algo // 结合算法
{
 public:
	algo * leftalgo;	// 左算法，初始化为0
	algo * rightalgo; // 右算法，初始化为0
	method met;	// 指明算法
	algojoin(algo * l, algo * r, method m):leftalgo(l),
		rightalgo(r), met(m)
		{ if(leftalgo)
				leftalgo->refnum++;
		  if(rightalgo)
				rightalgo->refnum++;
		};
	algojoin(algojoin& alg):algo(alg),
		leftalgo(alg.leftalgo),rightalgo(alg.rightalgo),met(alg.met){
			if(leftalgo)
				leftalgo->refnum++;
			if(rightalgo)
				rightalgo->refnum++;};
			// 拷贝构造函数
	virtual ~algojoin() {
		if(leftalgo) {	// 如左或者右算法已经没有被引用，则删除
			leftalgo->refnum--;
			if(!leftalgo->refnum) delete leftalgo;
		}
		if(rightalgo) {
			rightalgo->refnum--;
			if(!rightalgo->refnum) delete rightalgo;
		}
	};
	virtual algo * clone(); // 克隆自己
	virtual DOUBLE calculate(DOUBLE x);	// 实施结合算法
};

class algofun : public algo	// 函数算法
{
 public:
	DOUBLE (*f)(DOUBLE);	// 函数指针
	algofun(DOUBLE (*fun)(DOUBLE)):f(fun){};	// 用函数指针进行初始化
	algofun(algofun& alg):algo(alg),f(alg.f){}; // 拷贝构造函数
	virtual DOUBLE calculate(DOUBLE x);	// 实施函数算法
	virtual algo * clone(); // 克隆自己
};

class algogass : public algo	// 正态分布函数
{
 public:
	algogass(DOUBLE a, DOUBLE d):algo(a,1.0/d){};	// 用均值a和标准差d进行初始化
	algogass(algogass& alg):algo(alg){}; // 拷贝构造函数
	virtual DOUBLE calculate(DOUBLE x);	// 实施函数算法
	virtual algo * clone(); // 克隆自己
};

class algoinverse : public algo	// 根据一个算法求反函数的算法
	// 通过不断求f(x)-y=0在给定的各个y值下的根来进行
{
 public:
	algo * al;
	algoinverse(algo * a):al(a){if(al) al->refnum++;};	// a 是要求反函数的算法
	algoinverse(algoinverse& alg):algo(alg),al(alg.al){
   	al->refnum++;	}; // 拷贝构造函数
	~algoinverse() {
		if(al) {
			al->refnum--;
			if(!al->refnum)
				delete al;
		}
	};		// 析构函数
	virtual DOUBLE calculate(DOUBLE x);	// 实施函数算法
	virtual algo * clone(); // 克隆自己
};

class algoregress : public algo // 线性回归算法产生线性函数
{
 public:
	algoregress(matrix & xy, matrix* dt=0); // xy为n行2列的矩阵为n个样本点
		// dt必须指向一个6列一行的矩阵向量，六个数依次为偏差平方和，平均标准
		// 偏差，回归平方和，最大偏差，最小偏差，偏差平均值
};

class algopoly : public algo	// 多项式
{
 public:
	matrix data;	//	n乘1矩阵，存放n-1次多项式的系数a(0)到a(n-1)
	algopoly(){}; // 缺省构造函数，给子类作调用
	algopoly(matrix& d):data(d){};	// 用矩阵构造多项式
	algopoly(algopoly& alg):algo(alg),data(alg.data){}; // 拷贝构造函数
	virtual DOUBLE calculate(DOUBLE x);	// 实施函数算法
	virtual algo * clone();	// 克隆自己
	cmatrix getroots();	// 求出此多项式的所有根
};

class algopair : public algopoly // 最小二乘拟合类
{
 public:
	algopair(matrix& xy, size_t m, DOUBLE & t0,DOUBLE &t1,DOUBLE &t2);
		// m为拟合多项式的项数，dt0为误差平方和，dt1为误差绝对值和，
		// dt2为最大误差绝对值
	algopair(algopair& alg):algopoly(alg){}; // 拷贝构造函数
};

class algoenter2 : public algo	// 一元全区间不等距插值
{
 public:
	matrix data;	//	n乘2矩阵，n个坐标，先x后y,x必须从小到大
	algoenter2(matrix& d):data(d){};	// 用矩阵构造多项式
	algoenter2(algoenter2& alg):algo(alg),data(alg.data){}; // 拷贝构造函数
	virtual DOUBLE calculate(DOUBLE x);	// 实施函数算法
	virtual algo * clone();	// 克隆自己
};

class algoenter : public algo	// 一元全区间等距插值
{
 public:
	DOUBLE x0;		// 起始点
	DOUBLE h;		// 步长
	matrix data;	//	n乘1矩阵，n个函数值
	algoenter(const matrix& d,DOUBLE xx0, DOUBLE hh):
		data(d),x0(xx0),h(hh){};
	algoenter(algoenter& alg):algo(alg),data(alg.data),
		x0(alg.x0),h(alg.h){}; // 拷贝构造函数
	virtual DOUBLE calculate(DOUBLE x);	// 实施函数算法
	virtual algo * clone();	// 克隆自己
};

//enum class funckind {polyfunc=1, enter2func=2, gammafunc=3}; // 函数种类
const int polyfunc = 1;
const int enter2func = 2;
const int gammafunc = 3;

class func {	// 函数类
 public:
	algo * alg;	// 决定函数的算法

	func();	// 缺省构造函数
	func(DOUBLE a);	// 常函数的构造函数
	func(DOUBLE (*fun)(DOUBLE));	// 函数指针的构造函数
	func(const func & fn);	// 拷贝构造函数
	func(algo * a); // 算法构造函数
	func(algo& a); // 算法构造函数
	func(DOUBLE a, DOUBLE d);	// 产生正态分布函数a为均值，d为标准差
	func(matrix& m, int kind=0);	// 构造数值相关函数，
				// 如kind = polyfunc, 则m为nX1矩阵，是n-1阶多项式系数，
				// 其中m(0,0)为常数项，m(n-1,0)为n-1次项。
				// 如kind = enter2func, 则m为nX2矩阵，代表n个坐标点
				// 其中第1列是由小到大排过序的各点的x坐标
	func(matrix& m, DOUBLE x0, DOUBLE h); // 构造等距插值函数
			// 其中m是nX1阶矩阵，代表n个y值，x0是起始点，h是步长（采样间隔）
	virtual ~func() {		// 析构函数
		if(alg) {
			alg->refnum--;	// 引用数减一，如再无其它引用，则删除算法
			if(!alg->refnum)
				delete alg;
		}
	};

	DOUBLE operator()(DOUBLE x){return alg->cal(x);}; // 计算x的函数值
	func& operator=(const func& fn);	// 赋值运算符
	func& operator=(DOUBLE (*fn)(DOUBLE)); // 用函数指针的赋值运算符
	func& operator=(DOUBLE a); // 常函数的赋值运算符

	func& operator+=(func& fn);	// 自身加一个函数
	func& operator+=(DOUBLE a){alg=alg->add(a);return (*this);};//自身加一个常数
	func& operator+=(DOUBLE (*f)(DOUBLE)); // 自身加一个函数指针
	func operator+(func& fn);	// 相加产生新函数
	func operator+(DOUBLE a);	// 与常数相加产生新函数
	friend func operator+(DOUBLE a, func& f); // 同上但常数在前
	func operator+(DOUBLE (*f)(DOUBLE)); // 加一个函数指针产生新函数
	friend func operator+(DOUBLE (*f)(DOUBLE),func& fn); // 同上但函数指针在前

	func& neg(); // 自身取负
	func operator-(); // 产生负函数

	func& operator-=(func& fn); // 自身减一个函数
	func& operator-=(DOUBLE a){alg=alg->add(-a);return (*this);};//自身减一个常数
	func& operator-=(DOUBLE (*f)(DOUBLE)); // 自身减一个函数指针
	func operator-(func& fn);	// 相减产生新函数
	func operator-(DOUBLE a);	// 与常数相减产生新函数
	friend func operator-(DOUBLE a, func& f); // 同上但常数在前
	func operator-(DOUBLE (*f)(DOUBLE)); // 减一个函数指针产生新函数
	friend func operator-(DOUBLE (*f)(DOUBLE),func& fn); // 函数指针减函数

	func& operator*=(func& fn);	// 自身乘一个函数
	func& operator*=(DOUBLE a){alg=alg->mul(a);return (*this);};//自身乘一个常数
	func& operator*=(DOUBLE (*f)(DOUBLE)); // 自身乘一个函数指针
	func operator*(func& fn);	// 相乘产生新函数
	func operator*(DOUBLE a);	// 与常数相乘产生新函数
	friend func operator*(DOUBLE a, func& f); // 同上但常数在前
	func operator*(DOUBLE (*f)(DOUBLE)); // 乘一个函数指针产生新函数
	friend func operator*(DOUBLE (*f)(DOUBLE),func& fn); // 函数指针乘函数

	func& operator/=(func& fn);	// 自身除以一个函数
	func& operator/=(DOUBLE a){alg=alg->mul(1.0/a);return (*this);
			};//自身除以常数
	func& operator/=(DOUBLE (*f)(DOUBLE)); // 自身除以一个函数指针
	func operator/(func& fn);	// 相除产生新函数
	func operator/(DOUBLE a);	// 与常数相除产生新函数
	friend func operator/(DOUBLE a, func& f); // 常数除以函数
	func operator/(DOUBLE (*f)(DOUBLE)); // 除以一个函数指针产生新函数
	friend func operator/(DOUBLE (*f)(DOUBLE),func& fn); // 函数指针除以函数

	void setxfactor(DOUBLE a);	// 设置x因子为a
	void xroom(DOUBLE a);	  // x方向扩大a倍
	void setxshift(DOUBLE a);	// 设置函数沿x轴平移a
	void shiftxas(DOUBLE a); // 函数沿x轴右移a

	func& power(func& f);	// 函数的f次乘幂，函数自身改变
	func& power(DOUBLE a);	// 函数的a次幂，函数自身改变
	func operator^(func & fn);	// 函数的fn次乘幂，产生新函数，原函数不变
	func operator^(DOUBLE a);  // 函数的a次幂，产生新函数，原函数不变

	func operator()(const func & fn)const;	// 复合函数，产生新的函数

	DOUBLE integ(DOUBLE a, DOUBLE b, DOUBLE eps=defaulterr);
		// 从a到b计算函数的定积分
	DOUBLE singleroot(DOUBLE x=0.9, DOUBLE eps = defaulterr);
		// 计算函数f(x)=0的一个单根
	func inverse();	// 产生反函数

	void getab(DOUBLE& a,DOUBLE& b) {	// 主要被线性回归子类用，返回线性因子
   												// 和加常数
		a = alg->yfactor;
		b = alg->addconst;
	};
};

inline func operator+(DOUBLE a, func& f) // 常数加函数
{	return f+a; }

inline func operator+(DOUBLE (*f)(DOUBLE),func& fn) // 函数指针加函数
{	return fn+f;}

func operator-(DOUBLE a, func& f); // 常数减函数
func operator-(DOUBLE (*f)(DOUBLE),func& fn); // 函数指针减函数

inline func operator*(DOUBLE a, func& f) // 常数乘函数
{	return f*a; }
inline func operator*(DOUBLE (*f)(DOUBLE),func& fn) // 函数指针乘函数
{	return fn*f;}

func operator/(DOUBLE a, func& f); // 常数除以函数
func operator/(DOUBLE (*f)(DOUBLE),func& fn); // 函数指针除以函数

class funcgass : public func // 正态分布函数
{
 public:
	funcgass(DOUBLE a=0.0, DOUBLE d=1.0):func(new algogass(a,d)){};
	void setmandd(DOUBLE a, DOUBLE d){
		alg->xshift = a;
		alg->xfactor = 1.0/d; }; // 设置均值和标准差
	void getmandd(DOUBLE &a, DOUBLE &d) {
		a = alg->xshift; d=1.0/alg->xfactor;}; // 获得均值和标准差
};

class funcpoly : public func	// 多项式函数
{
 public:
	funcpoly(matrix& d):func(new algopoly(d)){};
	matrix& getdata(){return ((algopoly*)alg)->data;};
	cmatrix getroots(){return ((algopoly*)alg)->getroots();};
};

class funcenter2 : public func // 一元全区间不等距插值
{
 public:
	funcenter2(matrix& d):func(new algoenter2(d)){};
	matrix& getdata(){return ((algoenter2*)alg)->data;};
};

class funcenter : public func // 一元全区间等距插值
{
 public:
	funcenter(matrix& d, DOUBLE x0, DOUBLE h):
		func(new algoenter(d,x0,h)){};
	matrix& getdata(DOUBLE &xx0, DOUBLE &hh){
		xx0 = ((algoenter*)alg)->x0; hh = ((algoenter*)alg)->h;
		return ((algoenter*)alg)->data; };
};

class funcpair : public func // 最小二乘拟合函数
{
 public:
	funcpair(matrix& xy, size_t m, DOUBLE & t0,DOUBLE &t1,DOUBLE &t2);
		// xy为n行2列数组，存放n个样本点
			// m为拟合多项式的项数，dt0为误差平方和，dt1为误差绝对值和，
		// dt2为最大误差绝对值
	matrix & getdata(){return ((algopair*)alg)->data;};
	 // 返回结果拟合系数一维矩阵
	cmatrix getroots(){return ((algopoly*)alg)->getroots();};
		// 求出所有根
};

class funcregress : public func // 线性回归函数，其实就是线性函数ax+b
	// 但由数据样本点构成a与b
{
 public:
	funcregress(matrix & xy, matrix* dt=0);  // xy为n行2列的矩阵为n个样本点
		// dt必须指向一个6列一行的矩阵向量，六个数依次为偏差平方和，平均标准
		// 偏差，回归平方和，最大偏差，最小偏差，偏差平均值
};

#endif // FUNC_H