#include <float.h>
#include <math.h>
#include <stdio.h>
#include "vfunc.h"


matrix valgo::cal(matrix& x) { // 基类的基本算法
	matrix xx(x);
	if(xshift != 0.0) xx -= xshift;
	if(xfactor != 1.0) xx *=xfactor;
	calculate(xx);
	if(yfactor != 1.0) xx *= yfactor;
	if(addconst != 0.0) xx += addconst;
	return xx;
}

valgo * valgo::clone() // 克隆自己，必须被继承子类改写
{
	return new valgo(*this);
}

valgo * valgo::mul(DOUBLE a)	// 乘a
{
	valgo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->yfactor *= a;
	alg->addconst *= a;
	return alg;
}

valgo * valgo::add(DOUBLE a)	// 加a
{
	valgo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->addconst += a;
	return alg;
}

valgo * valgo::neg() // 取负
{
	valgo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->addconst = -alg->addconst;
	alg->yfactor = -alg->yfactor;
	return alg;
}

valgo * valgo::setxfactor(DOUBLE x)		// 设置x轴因子
{
	valgo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->xfactor = x;
	return alg;
}

valgo * valgo::xroom(DOUBLE x)	// 将xfactor扩大x倍
{
	valgo * alg;
	alg = setxfactor(x*xfactor);
	if(x!=0.0)
		alg->setxshift(alg->xshift/x);
	return alg;
}

valgo * valgo::setxshift(DOUBLE x) // 设置xshift的值
{
	valgo * alg;
	alg = this;
	if(refnum>1) { refnum--;
		alg = clone();	// 如引用数大于1，则产生新的对象
	}
	alg->xshift = x;
	return alg;
}

valgo * valgo::xshiftas(DOUBLE x)	// 从当前开始平移x
{
	return setxshift(xshift+x);
}

valgo * valgojoin::clone() // 克隆自己
{
	return new valgojoin(*this);
}

matrix& valgojoin::calculate(matrix& x)	// 实施结合算法
{
	if(leftalgo==0 || rightalgo==0)
		throw TMESSAGE("empty valgo pointor!");
	if(met == ccom) {		// 复合函数
		matrix x1;
		x1 = rightalgo->cal(x);
		x = leftalgo->cal(x1);
		return x;
	}
	matrix xr;
	xr = rightalgo->cal(x);
	x = leftalgo->cal(x);
	if(met == cadd)	// 返回各结合运算值
		x += xr;
	else if(met == csub)
		x -= xr;
	else if(met == cmul)
		x *= xr;
	else if(met == cdiv)
		x /= xr;
	return x;
}

valgo * valgofun::clone() // 克隆自己
{
	return new valgofun(*this);
}

matrix& valgofun::calculate(matrix& x)	// 实施函数算法
{
	if(f)
		return x=f(x);
	return (x*=0.0);
}

valgo * valgofun1::clone() // 克隆自己
{
	return new valgofun1(*this);
}

matrix& valgofun1::calculate(matrix& x)	// 实施函数算法
{
	DOUBLE xx;
	xx = x(0);
	if(f)
		return (x=f(xx));
	return (x*=0.0);
}

matrix& valgodiff::calculate(matrix& x)	// 实施函数算法
{
	DOUBLE t;
	t = x(0);
	return calcul(t);
}

valgo * valgodiff::clone() // 克隆自己
{
	return new valgodiff(*this);
}

matrix& valgodiff::calcul(DOUBLE t, DOUBLE eps)	// 标量算法
{
	DOUBLE h,t00;
	matrix y00(y0);
	t00 = t0;
	h = t-t0;
	if(t0==t)
		return y0;
	if(tnow==t)
		return ynow;
	if(t>tnow) {
		t00 = tnow;
		h = t - tnow;
		y00 = ynow;
	}
	matrix y(y00);
	size_t n = y.rownum;
	size_t m,i,j,k;
	DOUBLE hh,p,dt,x,tt,q,a[4];
	matrix g(n),b(n),c(n),d(n),e(n);
	hh=h; m=1; p=1.0+eps; x=t00;
	c = y;
	while (p>=eps)
		{ a[0]=hh/2.0; a[1]=a[0]; a[2]=hh; a[3]=hh;
		  g = y; y = c;
		  dt=h/m; t00=x;
		  for (j=0; j<m; j++)
			 { d = f(t00,y);  // grkt2f(t,y,n,d);
				b = y; e = y;
				for (k=0; k<=2; k++)
				  {
						y = e+(d*a[k]);
						b += d*(a[k+1]/3.0);
						tt=t00+a[k];
						d = f(tt,y);
				  }
				y = b + d*(hh/6.0);
				t00+=dt;
			 }
		  p=0.0;
		  for (i=0; i<n; i++)
			 { q=fabs(y(i)-g(i));
				if (q>p) p=q;
			 }
		  hh=hh/2.0; m=m+m;
		}	// end of while(p>eps)
	tnow = t;
	ynow = y;
	return ynow;
}

vfunc::vfunc()// 缺省构造函数，产生函数f(x)=x;
{
	alg = new valgo();
}

vfunc::vfunc(const vfunc & fn) // 拷贝构造函数
{
	alg = fn.alg;
	alg->refnum++;
}

vfunc::vfunc(matrix& (*fun)(matrix&))	// 函数指针的构造函数
{
	alg = new valgofun(fun);
}

vfunc::vfunc(matrix (*fun)(DOUBLE)) // 标量到向量的函数指针构造函数
{
	alg = new valgofun1(fun);
}

vfunc::vfunc(DOUBLE a) // 常函数构造函数
{
	alg = new valgo(a);
}

vfunc& vfunc::operator=(const vfunc& fn)	// 赋值运算符
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

vfunc& vfunc::operator=(matrix& (*fn)(matrix&)) // 用函数指针的赋值运算符
{
	if(alg) {
		alg->refnum--;	// 原算法引用数减1
		if(!alg->refnum)	// 如结果为0则删除原算法
			delete alg;
	}
	alg = new valgofun(fn);
	return (*this);
}

vfunc& vfunc::operator=(DOUBLE a) // 常函数的赋值运算符
{
	if(alg) {
		alg->refnum--;	// 原算法引用数减1
		if(!alg->refnum)	// 如结果为0则删除原算法
			delete alg;
	}
	alg = new valgo(a);
	return (*this);
}

vfunc& vfunc::operator+=(vfunc& fn) // 自身加一个函数
{
	valgo * a = new valgojoin(alg, fn.alg, cadd);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

vfunc& vfunc::operator+=(matrix& (*f)(matrix&)) // 自身加一个函数指针
{
	vfunc fn(f);	// 将函数指针包装为函数类
	operator+=(fn);	// 实施加操作
	return (*this);
}

vfunc vfunc::operator+(vfunc& fn)	// 相加产生新函数
{
	valgo * a = new valgojoin(alg, fn.alg, cadd);
	vfunc f(a);
	return f;
}

vfunc vfunc::operator+(DOUBLE a)	// 与常数相加产生新函数
{
	vfunc f(*this);
	f += a;
	return f;
}

vfunc vfunc::operator+(matrix& (*f)(matrix&)) // 加一个函数指针产生新函数
{
	vfunc ff(*this);
	ff += f;
	return ff;
}

vfunc& vfunc::neg() // 自身取负
{
	alg=alg->neg();
	return (*this);
}

vfunc vfunc::operator-() // 产生负函数
{
	vfunc f(*this);
	f.neg();
	return f;
}

vfunc& vfunc::operator-=(vfunc& fn) // 自身减一个函数
{
	valgo * a = new valgojoin(alg, fn.alg, csub);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

vfunc& vfunc::operator-=(matrix& (*f)(matrix&)) // 自身减一个函数指针
{
	vfunc fn(f);	// 将函数指针包装为函数类
	operator-=(fn);	// 实施减操作
	return (*this);
}

vfunc vfunc::operator-(vfunc& fn)	// 相减产生新函数
{
	valgo * a = new valgojoin(alg, fn.alg, csub);
	vfunc f(a);
	return f;
}

vfunc vfunc::operator-(DOUBLE a)	// 与常数相减产生新函数
{
	vfunc f(*this);
	f -= a;
	return f;
}

vfunc operator-(DOUBLE a, vfunc& f) // 常数减函数
{
	return (-f)+a;
}

vfunc vfunc::operator-(matrix& (*f)(matrix&)) // 减一个函数指针产生新函数
{
	vfunc ff(*this);
	ff -= f;
	return ff;
}

vfunc operator-(matrix& (*f)(matrix&),vfunc& fn) // 函数指针减函数
{
	vfunc ff(f);
	ff -= fn;
	return ff;
}

vfunc& vfunc::operator*=(vfunc& fn) // 自身乘一个函数
{
	valgo * a = new valgojoin(alg, fn.alg, cmul);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

vfunc& vfunc::operator*=(matrix& (*f)(matrix&)) // 自身乘一个函数指针
{
	vfunc fn(f);	// 将函数指针包装为函数类
	operator*=(fn);	// 实施乘操作
	return (*this);
}

vfunc vfunc::operator*(vfunc& fn)	// 相乘产生新函数
{
	valgo * a = new valgojoin(alg, fn.alg, cmul);
	vfunc f(a);
	return f;
}

vfunc vfunc::operator*(DOUBLE a)	// 与常数相乘产生新函数
{
	vfunc f(*this);
	f *= a;
	return f;
}

vfunc vfunc::operator*(matrix& (*f)(matrix&)) // 乘一个函数指针产生新函数
{
	vfunc ff(*this);
	ff *= f;
	return ff;
}

vfunc& vfunc::operator/=(vfunc& fn) // 自身除以一个函数
{
	valgo * a = new valgojoin(alg, fn.alg, cdiv);
	alg->refnum--;	// 因为联合算法对两个算法都加了引用数，因此再减回来
	alg = a;	// 将新指针赋给alg
	return (*this);
}

vfunc& vfunc::operator/=(matrix& (*f)(matrix&)) // 自身除以一个函数指针
{
	vfunc fn(f);	// 将函数指针包装为函数类
	operator/=(fn);	// 实施除法操作
	return (*this);
}

vfunc vfunc::operator/(vfunc& fn)	// 相除产生新函数
{
	valgo * a = new valgojoin(alg, fn.alg, cdiv);
	vfunc f(a);
	return f;
}

vfunc vfunc::operator/(DOUBLE a)	// 与常数相除产生新函数
{
	vfunc f(*this);
	f /= a;
	return f;
}

vfunc operator/(DOUBLE a, vfunc& f) // 常数除以函数
{
	vfunc ff(a);
	return ff/f;
}

vfunc vfunc::operator/(matrix& (*f)(matrix&)) // 除以一个函数指针产生新函数
{
	vfunc ff(*this);
	ff /= f;
	return ff;
}

vfunc operator/(matrix& (*f)(matrix&),vfunc& fn) // 函数指针除以函数
{
	vfunc ff(f);
	ff /= fn;
	return ff;
}

void vfunc::setxfactor(DOUBLE a)	// 设置x因子为a
{
	alg = alg->setxfactor(a);
}

void vfunc::xroom(DOUBLE a)	  // x方向扩大a倍
{
	alg = alg->xroom(a);
}

vfunc vfunc::operator()(vfunc & fn)	// 复合函数，产生新的函数
{
	valgo * a = new valgojoin(alg, fn.alg, ccom);
	vfunc f(a);
	return f;
}

void vfunc::setxshift(DOUBLE a)	// 设置函数沿x轴平移a
{
	alg = alg->setxshift(a);
}

void vfunc::shiftxas(DOUBLE a) // 函数沿x轴右移a
{
	alg = alg->xshiftas(a);
}

linemodel::linemodel(matrix& va, matrix & vh, matrix & q,
	 matrix & r, matrix & vx) :a(va),h(vh),w(q),v(r),x(vx),
		y(vh.rownum,1)
			// 构造函数，va初始状态转移矩阵，vh初始观测矩阵，q当前模型噪声协
			// 方差阵，r当前观测噪声协方差阵，vx初始状态变量
{
	if(va.rownum != vx.rownum ||
		va.rownum != va.colnum ||
		vh.colnum != va.rownum ||
		q.rownum != va.rownum ||
		r.rownum != vh.rownum) throw TMESSAGE("Data not meet need!");
	y = h*x+v();
};

void linemodel::setdata(matrix& va, matrix & vh, matrix & q, matrix & r)
{
	if(va.rownum != va.colnum ||
		va.rownum != x.rownum ||
		vh.colnum != va.rownum ||
		q.rownum != va.rownum ||
		r.rownum != vh.rownum) throw TMESSAGE("Data not meet need!");
	a = va;
	h = vh;
	w.setdata(q);
	v.setdata(r);
}

void linemodel::seta(matrix& va)
{
	if(va.rownum != va.colnum ||
		va.rownum != x.rownum) throw TMESSAGE("a not meet need!");
	a = va;
}

void linemodel::seth(matrix& vh)
{
	if(vh.rownum != y.rownum ||
		vh.colnum != x.rownum ) throw TMESSAGE("h not meet nead!");
	h = vh;
}

void linemodel::setq(matrix& q)
{
	if(q.rownum != x.rownum ||
		!q.issym) throw TMESSAGE("q not meet nead!");
	w.setdata(q);
}

void linemodel::setr(matrix& r)
{
	if(r.rownum != y.rownum ||
		!r.issym) throw TMESSAGE("r not meet nead!");
	v.setdata(r);
}

matrix & linemodel::next()	// 计算下一级x值并返回新的x对应的y值
{
	x = a*x+w();		// 产生新的状态变量值
	return (y=h*x+v());	// 产生并返回新的观测值
}

kalman::kalman(matrix &va,matrix& vq,matrix& vr,matrix& vh,matrix& vx,
	matrix& vp):
		// 构造函数，va为状态转移矩阵，vh观测矩阵，vq模型噪声协方差阵，
		// vr当前观测噪声协方差阵，vx初始状态变量估值，vp初始估值协方差阵
	a(va),q(vq),r(vr),h(vh),x(vx),p(vp),y(vh.rownum,1)
{
	if( va.rownum != va.colnum ||
		 !vq.issym || !vr.issym || !vp.issym ||
		 vq.rownum != va.rownum || vr.rownum != vh.rownum ||
		 vx.rownum != va.rownum || vp.rownum != vx.rownum ||
		 vh.colnum != vx.rownum ) throw TMESSAGE("data not meet need!");
	if( !vq.ispositive() || !vr.ispositive() )
		 throw TMESSAGE("var matrix not positive!");
	y = 0.0;
}

void kalman::setdata(matrix &va,matrix& vq,matrix& vr,matrix& vh)
		// 为时变系统随时设置系统参数，va为状态转移矩阵，vh观测矩阵，vq模型噪
		// 声协方差阵，vr当前观测噪声协方差阵
{
	if(va.rownum != va.colnum || va.rownum != x.rownum ||
		va.rownum != vq.rownum || !vq.issym ||
		vr.rownum != y.rownum || vh.rownum != y.rownum ||
		vh.colnum != x.rownum ) throw TMESSAGE("data not meet need!");
	if( !vq.ispositive() || !vr.ispositive() )
	 throw TMESSAGE("var matrix not positive!");
	a = va;
	q = vq;
	h = vh;
	r = vr;
}

void kalman::seta(matrix& va)
{
	if(va.rownum != va.colnum || va.rownum != x.rownum)
		throw TMESSAGE("data not meet need!");
	a = va;
}

void kalman::seth(matrix& vh)
{
	if(vh.rownum != y.rownum || vh.colnum != x.rownum)
		throw TMESSAGE("data not meet need!");
	h = vh;
}

void kalman::setq(matrix& vq)
{
	if(vq.rownum != x.rownum || !vq.issym)
		throw TMESSAGE("data not meet need!");
	if(!vq.ispositive())
		throw TMESSAGE("q is not positive!");
	q = vq;
}

void kalman::setr(matrix& vr)
{
	if(vr.rownum != y.rownum || !vr.issym)
		throw TMESSAGE("data not meet need!");
	if(!vr.ispositive())
		throw TMESSAGE("r is not positive!");
	r = vr;
}

matrix& kalman::next(matrix& y)	// 根据下一个观测值获得新的状态变量的估值
{
	x = a*x;		// 预测
	p = a*p*a.t()+q;	// 预测误差协方差
	matrix k=p*h.t()*((h*p*h.t()+r).inv()); // 新息的增益
	x += k*(y-h*x);	// 由新息校正x得现时估计
	p = (unit(p.rownum)-k*h)*p;	// 获得最后的误差协方差，注意unit(n)是
											// n阶单位矩阵
	return x;	// 返回x的估计值
}


regress::regress(matrix& x, matrix& y)	// x为mXn维矩阵，y为n个观测值
{
	if(x.colnum != y.rownum || x.colnum < x.rownum)
		throw TMESSAGE("dimension error");
	matrix c(x.rownum+1,x.colnum);
	size_t i,j;
	for(i=0; i<x.rownum; i++)
	for(j=0; j<x.colnum; j++)
		c.set(i,j,x(i,j));
	for(j=0; j<c.colnum; j++)
		c.set(x.rownum,j,1.0);
	matrix y1(c*y);
	c *= c.t();
	a = c.cholesky(y1);
	v = matrix(x.rownum);
	// 计算偏差平方和
	q = 0;
	DOUBLE yy;
	matrix dd(y.rownum);
	for(i=0; i<y.rownum; i++) {
		yy = y(i);
		for(j=0; j<x.rownum; j++)
			yy -= a(j)*x(j,i);
		yy -= a(a.rownum-1);
		dd.set(i,yy);
		q += yy*yy;
	}
	s = sqrt(q/(y.rownum));
	yy = 0.0;
	for(i=0; i<y.rownum; i++)
		yy += y(i);
	DOUBLE ye;
	ye = yy/(y.rownum);
	r = 0.0;
	for(i=0; i<y.rownum; i++)
		r += (y(i)-ye)*(y(i)-ye);
	r = sqrt(1.0-q/r);
	for(j=0;j<v.rownum;j++) {
		yy = 0;
		for(i=0;i<y.rownum;i++)
			yy += (dd(i)+a(j)*x(j,i))*(dd(i)+a(j)*x(j,i));
		v.set(j,sqrt(1.0-q/yy));
	}
	u = 0.0;
	for(i=0; i<y.rownum; i++) {
		yy = ye;
		for(j=0; j<x.rownum; j++)
			yy -= a(j)*x(j,i);
		yy -= a(x.rownum);
		u += yy*yy;
	}
}

DOUBLE regress::operator()(matrix& x) // 回归后的线性函数，x是自变量
{
	if(x.rownum != a.rownum-1) throw TMESSAGE("wrong dimension!");
	DOUBLE y = a(a.rownum-1);
	for(size_t i=0; i<x.rownum; i++)
		y += x(i)*a(i);
	return y;
}

