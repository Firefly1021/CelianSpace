#include <math.h>
#include <stdio.h>
#include "func.h"


DOUBLE algo::cal(DOUBLE x) { // 基类的基本算法
	return yfactor*calculate(xfactor*(x-xshift))+addconst;
}

algo * algo::clone() // 克隆自己，必须被继承子类改写
{
	return new algo(*this);
}

algo * algo::mul(DOUBLE a)	// 乘a
{
	algo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->yfactor *= a;
	alg->addconst *= a;
	return alg;
}

algo * algo::add(DOUBLE a)	// 加a
{
	algo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->addconst += a;
	return alg;
}

algo * algo::neg() // 取负
{
	algo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->addconst = -alg->addconst;
	alg->yfactor = -alg->yfactor;
	return alg;
}

algo * algo::setxfactor(DOUBLE x)		// 设置x轴因子
{
	algo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->xfactor = x;
	return alg;
}

algo * algo::xroom(DOUBLE x)	// 将xfactor扩大x倍
{
	algo * alg;
	alg = setxfactor(x*xfactor);
	if(x!=0)alg->xshift/=x;
	return alg;
}

algo * algo::setxshift(DOUBLE x) // 设置xshift的值
{
	algo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->xshift = x;
	return alg;
}

algo * algo::xshiftas(DOUBLE x)	// 从当前开始平移x
{
	return setxshift(xshift+x);
}

algo * algojoin::clone() // 克隆自己
{
	return new algojoin(*this);
}

DOUBLE algojoin::calculate(DOUBLE x)	// 实施结合算法
{
	if(leftalgo==0 || rightalgo==0)
		throw TMESSAGE("empty algo pointor!");
	DOUBLE a,b;
	if(met == method::ccom)		// 复合函数
		return leftalgo->cal(rightalgo->cal(x));
	a = leftalgo->cal(x);
	b = rightalgo->cal(x);
	if(met == method::cadd)	// 返回各结合运算值
		return a+b;
	else if(met == method::csub)
		return a-b;
	else if(met == method::cmul)
		return a*b;
	else if(met == method::cdiv)
		return a/b;
	else if(met == method::cpow)
		return pow(a,b);
	return 0.0;
}

algo * algofun::clone() // 克隆自己
{
	return new algofun(*this);
}

DOUBLE algofun::calculate(DOUBLE x)	// 实施函数算法
{
	if(f)
		return f(x);
	return 0.0;
}

func::func()// 缺省构造函数，产生函数f(x)=x;
{
	alg = new algo();
}

func::func(const func & fn) // 拷贝构造函数
{
	alg = fn.alg;
	alg->refnum++;
}

func::func(DOUBLE (*fun)(DOUBLE))	// 函数指针的构造函数
{
	alg = new algofun(fun);
}

func::func(DOUBLE a) // 常函数构造函数
{
	alg = new algo(a);
}

func::func(algo * a):alg(NULL) // 算法的构造函数
{
	if(a)
		alg = a;
}

func::func(algo& a) // 另一种算法构造函数
{
	alg = a.clone();	// 克隆出一个同样的算法变量并将指针赋予alg.
}

func& func::operator=(const func& fn)	// 赋值运算符
{
	if(this == &fn) return (*this); // 如果等于自己，干脆啥也别做
	if(alg) {
		alg->refnum--;	// 原算法引用数减1
		if(!alg->refnum)	// 如结果为0则删除原算法
			delete alg;
	}
	alg = fn.alg;	// 将fn的算法移过来
	if(alg) alg->refnum++; // 引用数增加
	return (*this);
}

func& func::operator=(DOUBLE (*fn)(DOUBLE)) // 用函数指针的赋值运算符
{
	if(alg) {
		alg->refnum--;	// 原算法引用数减1
		if(!alg->refnum)	// 如结果为0则删除原算法
			delete alg;
	}
	alg = new algofun(fn);
	return (*this);
}

func& func::operator=(DOUBLE a) // 常函数的赋值运算符
{
	if(alg) {
		alg->refnum--;	// 原算法引用数减1
		if(!alg->refnum)	// 如结果为0则删除原算法
			delete alg;
	}
	alg = new algo(a);
	return (*this);
}

func& func::operator+=(func& fn) // 自身加一个函数
{
	algo * a = new algojoin(alg, fn.alg, method::cadd);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

func& func::operator+=(DOUBLE (*f)(DOUBLE)) // 自身加一个函数指针
{
	func fn(f);	// 将函数指针包装为函数类
	operator+=(fn);	// 实施加操作
	return (*this);
}

func func::operator+(func& fn)	// 相加产生新函数
{
	algo * a = new algojoin(alg, fn.alg, method::cadd);
	func f(a);
	return f;
}

func func::operator+(DOUBLE a)	// 与常数相加产生新函数
{
	func f(*this);
	f += a;
	return f;
}

func func::operator+(DOUBLE (*f)(DOUBLE)) // 加一个函数指针产生新函数
{
	func ff(*this);
	ff += f;
	return ff;
}

func& func::neg() // 自身取负
{
	alg=alg->neg();
	return (*this);
}

func func::operator-() // 产生负函数
{
	func f(*this);
	f.neg();
	return f;
}

func& func::operator-=(func& fn) // 自身减一个函数
{
	algo * a = new algojoin(alg, fn.alg, method::csub);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

func& func::operator-=(DOUBLE (*f)(DOUBLE)) // 自身减一个函数指针
{
	func fn(f);	// 将函数指针包装为函数类
	operator-=(fn);	// 实施减操作
	return (*this);
}

func func::operator-(func& fn)	// 相减产生新函数
{
	algo * a = new algojoin(alg, fn.alg, method::csub);
	func f(a);
	return f;
}

func func::operator-(DOUBLE a)	// 与常数相减产生新函数
{
	func f(*this);
	f -= a;
	return f;
}

func operator-(DOUBLE a, func& f) // 常数减函数
{
	return (-f)+a;
}

func func::operator-(DOUBLE (*f)(DOUBLE)) // 减一个函数指针产生新函数
{
	func ff(*this);
	ff -= f;
	return ff;
}

func operator-(DOUBLE (*f)(DOUBLE),func& fn) // 函数指针减函数
{
	func ff(f);
	ff -= fn;
	return ff;
}

func& func::operator*=(func& fn) // 自身乘一个函数
{
	algo * a = new algojoin(alg, fn.alg, method::cmul);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

func& func::operator*=(DOUBLE (*f)(DOUBLE)) // 自身乘一个函数指针
{
	func fn(f);	// 将函数指针包装为函数类
	operator*=(fn);	// 实施乘操作
	return (*this);
}

func func::operator*(func& fn)	// 相乘产生新函数
{
	algo * a = new algojoin(alg, fn.alg, method::cmul);
	func f(a);
	return f;
}

func func::operator*(DOUBLE a)	// 与常数相乘产生新函数
{
	func f(*this);
	f *= a;
	return f;
}

func func::operator*(DOUBLE (*f)(DOUBLE)) // 乘一个函数指针产生新函数
{
	func ff(*this);
	ff *= f;
	return ff;
}

func& func::operator/=(func& fn) // 自身除以一个函数
{
	algo * a = new algojoin(alg, fn.alg, method::cdiv);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

func& func::operator/=(DOUBLE (*f)(DOUBLE)) // 自身除以一个函数指针
{
	func fn(f);	// 将函数指针包装为函数类
	operator/=(fn);	// 实施除法操作
	return (*this);
}

func func::operator/(func& fn)	// 相除产生新函数
{
	algo * a = new algojoin(alg, fn.alg, method::cdiv);
	func f(a);
	return f;
}

func func::operator/(DOUBLE a)	// 与常数相除产生新函数
{
	func f(*this);
	f /= a;
	return f;
}

func operator/(DOUBLE a, func& f) // 常数除以函数
{
	func ff(a);
	return ff/f;
}

func func::operator/(DOUBLE (*f)(DOUBLE)) // 除以一个函数指针产生新函数
{
	func ff(*this);
	ff /= f;
	return ff;
}

func operator/(DOUBLE (*f)(DOUBLE),func& fn) // 函数指针除以函数
{
	func ff(f);
	ff /= fn;
	return ff;
}

void func::setxfactor(DOUBLE a)	// 设置x因子为a
{
	alg = alg->setxfactor(a);
}

void func::xroom(DOUBLE a)	  // x方向扩大a倍
{
	alg = alg->xroom(a);
}

func& func::power(func& f)	// 函数的f次乘幂，函数自身改变
{
	algo * a = new algojoin(alg, f.alg, method::cpow);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

func func::operator^(func& fn)	// f次乘幂，产生新函数
{
	algo * a = new algojoin(alg, fn.alg, method::cpow);
	func f(a);
	return f;
}

func& func::power(DOUBLE a)	// 函数的a次幂，函数自身改变
{
	func f(a);
	return power(f);
}

func func::operator^(DOUBLE a)  // 函数的a次幂，产生新函数，原函数不变
{
	func f(a);
	func ff(*this);
	return ff.power(f);
}

func func::operator()(const func & fn)const	// 复合函数，产生新的函数
{
	algo * a = new algojoin(alg, fn.alg, method::ccom);
	func f(a);
	return f;
}

DOUBLE algopoly::calculate(DOUBLE x)	// 实施函数算法
{
	size_t i;
	DOUBLE u;
	size_t n=data.rownum-1;
	u=data(n,0);
	for (i=n; i>0; i--)
		u=u*x+data(i-1,0);
	return u;
}

algo * algopoly::clone()	// 克隆自己
{
	return new algopoly(*this);
}

DOUBLE algogass::calculate(DOUBLE x)	// 实施函数算法
{
	return gass(0,1,x);
}

algo * algogass::clone() // 克隆自己
{
	return new algogass(*this);
}

func::func(DOUBLE a, DOUBLE d)	// 产生正态分布函数a为均值，d为标准差
{
	alg = new algogass(a,d);
}

func::func(matrix& m, int kind):alg(NULL)	// 构造数值相关函数，
				// 如kind = polyfunc, 则m为nX1矩阵，是n-1阶多项式系数，
				// 其中m(0,0)为常数项，m(n-1,0)为n-1次项。
{
	if(kind == polyfunc)
		alg = new algopoly(m);	// 产生多项式算法
	else if(kind == enter2func)
		alg = new algoenter2(m);	// 产生一元全区间不等距插值
}

DOUBLE algoenter2::calculate(DOUBLE x)	// 实施函数算法
{
	size_t n = data.rownum;
	size_t i,j,k,m;
	double z,s;
	z=0.0;
	if (n<1) return(z);
	if (n==1) { z=data(0,1); return z;}
	if (n==2) {
		z = data(0,1)*(x-data(1,0))-data(1,1)*(x-data(0,0))/
			(data(0,0)-data(1,0));
		  return(z);
	}
	i=0;
	while ((i<n) && (data(i,0)<x)) i++; // ((x[i]<t)&&(i<n)) i=i+1;
	if (i<4) k=0;
	else k=i-4;
	m=i+3;
	if (m>n-1) m=n-1;
	for (i=k;i<=m;i++)
	{	s=1.0;
		for (j=k;j<=m;j++)
			if (j!=i) s *= (x-data(j,0))/(data(i,0)-data(j,0)); //s=s*(t-x[j])/(x[i]-x[j]);
		z=z+s*data(i,1); // y[i];
	}
	return(z);
}

algo * algoenter2::clone()	// 克隆自己
{
	return new algoenter2(*this);
}

DOUBLE algoenter::calculate(DOUBLE x)	// 实施函数算法
{
// double eelgr(x0,h,n,y,t)
	size_t n=data.rownum;
//  double x0,h,t,y[];
	size_t i,j,k,m;
	DOUBLE z,s,xi,xj,p,q;
	z=0.0;
	if (n<1) return z;
	if (n==1) { z=data(0); return(z);}
	if (n==2)
	{	z=(data(1)*(x-x0)-data(0)*(x-x0-h))/h;
		return z;
	}
	if (x>x0)
	{	p=(x-x0)/h; q = floor(p); i=(size_t)q;
		if (p>q) i++;
	}
	else i=0;
	k=i-4;
	if (i<4) k=0;
	else k=i-4;
	m=i+3;
	if (m>n-1) m=n-1;
	for (i=k;i<=m;i++)
	{	s=1.0; xi=x0+i*h;
		for (j=k; j<=m; j++)
			if (j!=i)
			{	xj=x0+j*h;
				s*=(x-xj)/(xi-xj);
			}
			z=z+s*data(i);
		}
	return z;
}

algo * algoenter::clone()	// 克隆自己
{
	return new algoenter(*this);
}

func::func(matrix& m, DOUBLE x0, DOUBLE h) // 构造等距插值函数
			// 其中m是nX1阶矩阵，代表n个y值，x0是起始点，h是步长（采样间隔）
{
	alg = new algoenter(m,x0,h);
}

void func::setxshift(DOUBLE a)	// 设置函数沿x轴平移a
{
	alg = alg->setxshift(a);
}

void func::shiftxas(DOUBLE a) // 函数沿x轴右移a
{
	alg = alg->xshiftas(a);
}

DOUBLE algo::integ(DOUBLE a, DOUBLE b, DOUBLE eps) // 在(a,b)区间积分
			// 采用勒让德高斯求积法
{
	 size_t m,i,j;
	 DOUBLE s,p,ep,h,aa,bb,w,x,g;
	 g = 0.0;
	 static DOUBLE t[5]={-0.9061798459,-0.5384693101,0.0,
								 0.5384693101,0.9061798459};
	 static DOUBLE c[5]={0.2369268851,0.4786286705,0.5688888889,
								0.4786286705,0.2369268851};
	 m=1;
	 h=b-a; s=fabs(0.001*h);
	 p=1.0e+35; ep=eps+1.0;
	 while ((ep>=eps)&&(fabs(h)>s))
		{ g=0.0;
		  for (i=1;i<=m;i++)
			 { aa=a+(i-1.0)*h; bb=a+i*h;
				w=0.0;
				for (j=0;j<=4;j++)
				  { x=((bb-aa)*t[j]+(bb+aa))/2.0;
					 w=w+cal(x)*c[j];  //flrgsf(x)*c[j];
				  }
				g=g+w;
			 }
		  g *= h/2.0;
		  ep=fabs(g-p)/(1.0+fabs(g));
		  p=g; m=m+1; h=(b-a)/m;
		}
	 return g;
}

DOUBLE func::integ(DOUBLE a, DOUBLE b, DOUBLE eps)
		// 从a到b计算函数的定积分
{
	return alg->integ(a,b,eps);
}

DOUBLE func::singleroot(DOUBLE x, DOUBLE eps)
	// 计算函数在x附近的一个单根
{
	return getroot(*alg,x,eps);
}

DOUBLE algoinverse::calculate(DOUBLE x)	// 实施反函数算法
{
	algo * a = al->clone();	// 克隆一个一样的函数算法变量
	a->add(-x); // 形成函数f(t)-x
	DOUBLE t;
	t = getroot(*a);	// 求f(t)-x=0的根
	delete a;
	return t;
}

algo * algoinverse::clone() // 克隆自己
{
	return new algoinverse(*this);
}

func func::inverse()	// 产生反函数
{
	algo * a = new algoinverse(alg);
	func f;
	f.alg = a;
	return f;
}

