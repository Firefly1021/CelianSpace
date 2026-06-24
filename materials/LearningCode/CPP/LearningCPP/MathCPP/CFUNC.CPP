// #include <values.h>
#include <math.h>
#include <stdio.h>
#include "cfunc.h"


COMPLEX calgo::cal(DOUBLE x) { // 基类的基本算法
	return yfactor*calculate(xfactor*(x-xshift))+addconst;
}


calgo * calgo::clone() // 克隆自己，必须被继承子类改写
{
	return new calgo(*this);
}

calgo * calgo::mul(COMPLEX a)	// 乘a
{
	calgo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->yfactor *= a;
	alg->addconst *= a;
	return alg;
}

calgo * calgo::add(COMPLEX a)	// 加a
{
	calgo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->addconst += a;
	return alg;
}

calgo * calgo::neg() // 取负
{
	calgo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->addconst = -alg->addconst;
	alg->yfactor = -alg->yfactor;
	return alg;
}

calgo * calgo::setxfactor(DOUBLE x)		// 设置x轴因子
{
	calgo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->xfactor = x;
	return alg;
}

calgo * calgo::xroom(DOUBLE x)	// 将xfactor扩大x倍
{
	calgo* c = NULL;
	if (x != 0)
		c = setxshift(xshift / x);
	if(!c) throw TMESSAGE("shift error");
	return c->setxfactor(x*xfactor);
}

calgo * calgo::setxshift(DOUBLE x) // 设置xshift的值
{
	calgo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->xshift = x;
	return alg;
}

calgo * calgo::xshiftas(DOUBLE x)	// 从当前开始平移x
{
	return setxshift(xshift+x);
}

calgo * calgojoin::clone() // 克隆自己
{
	return new calgojoin(*this);
}

COMPLEX calgojoin::calculate(DOUBLE x)	// 实施结合算法
{
	if(leftalgo==0 || rightalgo==0)
		throw TMESSAGE("empty algo pointor!");
	COMPLEX a,b;
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
		return std::pow(a,b);
	return 0.0;
}

calgo * calgofun::clone() // 克隆自己
{
	return new calgofun(*this);
}

COMPLEX calgofun::calculate(DOUBLE x)	// 实施函数算法
{
	if(f)
		return f(x);
	return 0.0;
}

COMPLEX calgoenter::calculate(DOUBLE x)	// 实施函数算法
{
	return COMPLEX(er->cal(x),ei->cal(x));
}

calgo * calgoenter::clone()	// 克隆自己
{
	return new calgoenter(*this);
}

cfunc::cfunc()// 缺省构造函数，产生函数f(x)=x;
{
	alg = new calgo();
}

cfunc::cfunc(const cfunc & fn) // 拷贝构造函数
{
	alg = fn.alg;
	alg->refnum++;
}

cfunc::cfunc(COMPLEX (*fun)(DOUBLE))	// 函数指针的构造函数
{
	alg = new calgofun(fun);
}

cfunc::cfunc(COMPLEX a) // 常函数构造函数
{
	alg = new calgo(a);
}

cfunc& cfunc::operator=(const cfunc& fn)	// 赋值运算符
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

cfunc& cfunc::operator=(COMPLEX (*fn)(DOUBLE)) // 用函数指针的赋值运算符
{
	if(alg) {
		alg->refnum--;	// 原算法引用数减1
		if(!alg->refnum)	// 如结果为0则删除原算法
			delete alg;
	}
	alg = new calgofun(fn);
	return (*this);
}

cfunc& cfunc::operator=(COMPLEX a) // 常函数的赋值运算符
{
	if(alg) {
		alg->refnum--;	// 原算法引用数减1
		if(!alg->refnum)	// 如结果为0则删除原算法
			delete alg;
	}
	alg = new calgo(a);
	return (*this);
}

cfunc& cfunc::operator+=(cfunc& fn) // 自身加一个函数
{
	calgo * a = new calgojoin(alg, fn.alg, method::cadd);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

cfunc& cfunc::operator+=(COMPLEX (*f)(DOUBLE)) // 自身加一个函数指针
{
	cfunc fn(f);	// 将函数指针包装为函数类
	operator+=(fn);	// 实施加操作
	return (*this);
}

cfunc cfunc::operator+(cfunc& fn)	// 相加产生新函数
{
	calgo * a = new calgojoin(alg, fn.alg, method::cadd);
	cfunc f(a);
	return f;
}

cfunc cfunc::operator+(COMPLEX a)	// 与常数相加产生新函数
{
	cfunc f(*this);
	f += a;
	return f;
}

cfunc cfunc::operator+(COMPLEX (*f)(DOUBLE)) // 加一个函数指针产生新函数
{
	cfunc ff(*this);
	ff += f;
	return ff;
}

cfunc& cfunc::neg() // 自身取负
{
	alg=alg->neg();
	return (*this);
}

cfunc cfunc::operator-() // 产生负函数
{
	cfunc f(*this);
	f.neg();
	return f;
}

cfunc& cfunc::operator-=(cfunc& fn) // 自身减一个函数
{
	calgo * a = new calgojoin(alg, fn.alg, method::csub);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

cfunc& cfunc::operator-=(COMPLEX (*f)(DOUBLE)) // 自身减一个函数指针
{
	cfunc fn(f);	// 将函数指针包装为函数类
	operator-=(fn);	// 实施减操作
	return (*this);
}

cfunc cfunc::operator-(cfunc& fn)	// 相减产生新函数
{
	calgo * a = new calgojoin(alg, fn.alg, method::csub);
	cfunc f(a);
	return f;
}

cfunc cfunc::operator-(COMPLEX a)	// 与常数相减产生新函数
{
	cfunc f(*this);
	f -= a;
	return f;
}

cfunc operator-(COMPLEX a, cfunc& f) // 常数减函数
{
	return (-f)+a;
}

cfunc cfunc::operator-(COMPLEX (*f)(DOUBLE)) // 减一个函数指针产生新函数
{
	cfunc ff(*this);
	ff -= f;
	return ff;
}

cfunc operator-(COMPLEX (*f)(DOUBLE),cfunc& fn) // 函数指针减函数
{
	cfunc ff(f);
	ff -= fn;
	return ff;
}

cfunc& cfunc::operator*=(cfunc& fn) // 自身乘一个函数
{
	calgo * a = new calgojoin(alg, fn.alg, method::cmul);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

cfunc& cfunc::operator*=(COMPLEX (*f)(DOUBLE)) // 自身乘一个函数指针
{
	cfunc fn(f);	// 将函数指针包装为函数类
	operator*=(fn);	// 实施乘操作
	return (*this);
}

cfunc cfunc::operator*(cfunc& fn)	// 相乘产生新函数
{
	calgo * a = new calgojoin(alg, fn.alg, method::cmul);
	cfunc f(a);
	return f;
}

cfunc cfunc::operator*(COMPLEX a)	// 与常数相乘产生新函数
{
	cfunc f(*this);
	f *= a;
	return f;
}

cfunc cfunc::operator*(COMPLEX (*f)(DOUBLE)) // 乘一个函数指针产生新函数
{
	cfunc ff(*this);
	ff *= f;
	return ff;
}

cfunc& cfunc::operator/=(cfunc& fn) // 自身除以一个函数
{
	calgo * a = new calgojoin(alg, fn.alg, method::cdiv);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

cfunc& cfunc::operator/=(COMPLEX (*f)(DOUBLE)) // 自身除以一个函数指针
{
	cfunc fn(f);	// 将函数指针包装为函数类
	operator/=(fn);	// 实施除法操作
	return (*this);
}

cfunc cfunc::operator/(cfunc& fn)	// 相除产生新函数
{
	calgo * a = new calgojoin(alg, fn.alg, method::cdiv);
	cfunc f(a);
	return f;
}

cfunc cfunc::operator/(COMPLEX a)	// 与常数相除产生新函数
{
	cfunc f(*this);
	f /= a;
	return f;
}

cfunc operator/(COMPLEX a, cfunc& f) // 常数除以函数
{
	cfunc ff(a);
	return ff/f;
}

cfunc cfunc::operator/(COMPLEX (*f)(DOUBLE)) // 除以一个函数指针产生新函数
{
	cfunc ff(*this);
	ff /= f;
	return ff;
}

cfunc operator/(COMPLEX (*f)(DOUBLE),cfunc& fn) // 函数指针除以函数
{
	cfunc ff(f);
	ff /= fn;
	return ff;
}

void cfunc::setxfactor(DOUBLE a)	// 设置x因子为a
{
	alg = alg->setxfactor(a);
}

void cfunc::xroom(DOUBLE a)	  // x方向扩大a倍
{
	alg = alg->xroom(a);
}

cfunc& cfunc::power(cfunc& f)	// 函数的f次乘幂，函数自身改变
{
	calgo * a = new calgojoin(alg, f.alg, method::cpow);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

cfunc cfunc::operator^(cfunc& fn)	// f次乘幂，产生新函数
{
	calgo * a = new calgojoin(alg, fn.alg, method::cpow);
	cfunc f(a);
	return f;
}

cfunc& cfunc::power(COMPLEX a)	// 函数的a次幂，函数自身改变
{
	cfunc f(a);
	return power(f);
}

cfunc cfunc::operator^(	COMPLEX a)  // 函数的a次幂，产生新函数，原函数不变
{
	cfunc f(a);
	cfunc ff(*this);
	return ff.power(f);
}

COMPLEX calgopoly::calculate(DOUBLE x)	// 实施函数算法
{
	size_t i;
	COMPLEX u;
	size_t n=data.rownum-1;
	u=data(n);
	for (i=n; i>0; i--)
		u=u*x+data(i-1);
	return u;
}

calgo * calgopoly::clone()	// 克隆自己
{
	return new calgopoly(*this);
}

cfunc::cfunc(cmatrix& m)	// 构造数值相关函数，
				// m为nX1矩阵，是n-1阶多项式系数，
				// 其中m(0,0)为常数项，m(n-1,0)为n-1次项。
{
	alg = new calgopoly(m);	// 产生多项式算法
}

void cfunc::setxshift(DOUBLE a)	// 设置函数沿x轴平移a
{
	alg = alg->setxshift(a);
}

void cfunc::shiftxas(DOUBLE a) // 函数沿x轴右移a
{
	alg = alg->xshiftas(a);
}

cfuncenter::cfuncenter(cmatrix& s,DOUBLE t0, DOUBLE dt):
	cfunc(new calgoenter(s.real(),s.image(),t0,dt))
{}


cfunc fourier(func& f,DOUBLE tb, DOUBLE te, DOUBLE dt, DOUBLE df)
	// 利用fft技术对函数f作傅里叶变换，其中tb为采样窗口的起始点，te为结束点，
	// 必须te>tb，dt为采样间隔，df为频率采样间隔，但返回的cfunc是作了插值的
	// 插值函数
{
	if(tb>=te) throw TMESSAGE("begin point must less than end point");
	DOUBLE tspace;
	tspace = te-tb;
	size_t n,i;
	if(tspace > 1.0/df) df = 1.0/tspace;
	n = (size_t)ceil(1.0/(df*dt));
	cmatrix s(n);
	s = COMPLEX(0.0);
	n = (size_t)ceil(tspace/dt);
	for(i=0; i<n; i++)
		s.set(i,f(tb+i*dt));
	s.fft();
	n = s.rownum;
	cmatrix p(n);
	df = 1.0/(dt*n);
	for(i=0;i<n/2;i++) {
		p.set(i,s(i+n/2));
		p.set(i+n/2,s(i));
	}
	for(i=1; i<n/2; i++) {
		COMPLEX c;
		DOUBLE phi;
		phi = 2.0*M_PI*i*df*(-tb);
		c = COMPLEX(cos(phi),sin(phi));
		p.set(i+n/2,p(i+n/2)*c);
		p.set(n/2-i,p(n/2-i)*std::conj(c));
	}
	p *= dt;
	DOUBLE fbegin = -0.5/dt;
	return cfuncenter(p,fbegin,df);
}

