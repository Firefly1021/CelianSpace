#include <stdio.h>
#include <string.h>
#include <fstream>
#include "cmatrix.h"

// 缺省的构造函数，产生0行0列空复矩阵
cmatrix::cmatrix(cbuffer * b): rownum(0),colnum(0),
	isneg(0),istrans(0),isconj(0)
{
		if(b){
			buf=b;
			buf->alloc(0);
			}
		else
			buf = getnewcbuffer(0); // 如果缺省的b没有给出，则产生一
				// 长度为行乘列个复数和缓存
}
// 构造一r行c列的复矩阵，可选的复缓存指向b
cmatrix::cmatrix(size_t r, size_t c, cbuffer * b):
		rownum(r),colnum(c),istrans(0),isneg(0),isconj(0)
{
		if(b){
			buf=b;
			buf->alloc(r*c);
			}
		else
			buf = getnewcbuffer(r*c); // 如果缺省的b没有给出，则产生一
				// 长度为行乘列个复数和缓存
}

cmatrix::cmatrix(size_t n, cbuffer * b):
		rownum(n),colnum(1),istrans(0),isneg(0),isconj(0)
{
		if(b){
			buf=b;
			buf->alloc(n);
			}
		else
			buf = getnewcbuffer(n); // 如果缺省的b没有给出，则产生一
				// 长度为行乘列个复数和缓存
}

cmatrix cunit(size_t n) // 产生n阶单位矩阵
{
	cmatrix m(n,n);
	for(size_t i=0; i<n; i++)
	for(size_t j=0; j<n; j++)
			if(i==j) m.set(i,i,1.0);
			else m.set(i,j,0.0);
	return m;
}

// 拷贝构造函数
cmatrix::cmatrix(const cmatrix& m)
{
	rownum = m.rownum; // 复制行数和列数
	colnum = m.colnum;
	istrans = m.istrans; // 复制标志
	isneg = m.isneg;
	isconj = m.isconj;
	buf = m.buf; // 指向相同的缓存
	buf->refnum++;	// 缓存的引用数增1
}

// 用数据文件构造复矩阵
cmatrix::cmatrix(const char * filename, cbuffer * b) :
	istrans(0),isneg(0),isconj(0)
{
	char label[10];
	std::ifstream in(filename); // 打开文件输入
	in >> label; // 文件以cmatrix关键词打头
	if(strcmp(label, "cmatrix")!=0) throw TMESSAGE("format error!");
	in >> rownum >> colnum; // 接着是行数和列数
	if(!in.good()) throw TMESSAGE("read file error!");
	if(b) { buf=b;	// 产生所需长度的缓存
		buf->alloc(rownum*colnum);
	}
	else buf = getnewcbuffer(rownum*colnum);
	size_t line;
	for(size_t i=0; i<rownum; i++) { // 按行循环输入
		in >> line;	// 每行以行号开始
		if(line != i+1) throw TMESSAGE("format error!");
		in.width(sizeof(label));
		in >> label; // 紧接着是冒号
		if(label[0] != ':') throw TMESSAGE("format error!");
		COMPLEX a;
		for(size_t j=0; j<colnum; j++) { // 随后是一行的内容
			in >> a;
			set(i,j,a);
		}
		if(!in.good()) throw TMESSAGE("read file error!");
	}
}

// 从内存数据块构造复矩阵。data指向内存数据块，构造完毕后data指向的内存可以释放
cmatrix::cmatrix(void * data, size_t r, size_t c, cbuffer * b):
		rownum(r),colnum(c),istrans(0),isneg(0),isconj(0)
{
	if(b){
		buf=b;
		buf->alloc(r*c);
		}
	else
		buf = getnewcbuffer(r*c);
	COMPLEX * d = (COMPLEX *)data;
	for(size_t i=0; i<r*c; i++)
		buf->set(i,d[i]);
}

cmatrix::cmatrix(matrix& m, cbuffer* b) : // 由实矩阵产生一个内容相同的复矩阵
	istrans(0),isneg(0),isconj(0)
{
	rownum = m.rownum;
	colnum = m.colnum;
	if(b) { buf=b;	// 产生所需长度的缓存
		buf->alloc(rownum*colnum);
	}
	else buf = getnewcbuffer(rownum*colnum);
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++)
		set(i,j,m.value(i,j));
}

cmatrix::cmatrix(matrix& mr, matrix& mi, cbuffer * b): // 两个实矩阵成为实部和虚部
	istrans(0),isneg(0),isconj(0)
{
	if(mr.rownum != mi.rownum || mr.colnum != mi.colnum)
		throw TMESSAGE("dimension not same!");
	rownum = mi.rownum;
	colnum = mr.colnum;
	if(b) { buf=b;	// 产生所需长度的缓存
		buf->alloc(rownum*colnum);
	}
	else buf = getnewcbuffer(rownum*colnum);
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++)
		set(i,j,COMPLEX(mr.value(i,j),mi.value(i,j)));
}

matrix cmatrix::real()const // 由实部生成实矩阵
{
	matrix m(rownum, colnum);
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++) {
		m.set(i,j,std::real(value(i,j)));
	}
	return m;
}

matrix cmatrix::image()const // 由虚部生成实矩阵
{
	matrix m(rownum, colnum);
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++)
		m.set(i,j,std::imag(value(i,j)));
	return m;
}

// 矩阵赋值
cmatrix& cmatrix::operator=(const cmatrix& m)
{
	rownum = m.rownum; // 复制行数和列数
	colnum = m.colnum;
	if(buf == m.buf) // 如果本来就指向相同的缓存，则返回
		return (*this);
	buf->refnum--;	// 否则放弃原来的缓存，先将原缓存引用数减1
						// 如果不为0说明还有其它的矩阵引用数据
	if(!buf->refnum)delete buf; // 否则删除缓存
	istrans = m.istrans;	// 复制标志
	isneg = m.isneg;
	isconj = m.isconj;
	buf = m.buf; // 复制缓存指针
	buf->refnum++; // 缓存引用数增1
	return (*this);
}

cmatrix& cmatrix::operator=(matrix& m)
{
	rownum = m.rownum; // 复制行数和列数
	colnum = m.colnum;
	buf->refnum--;
	if(!buf->refnum) delete buf; // 放弃原来缓存
	istrans = isneg = isconj = 0; // 标志清零
	buf = getnewcbuffer(rownum*colnum); // 申请新的缓存
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++)
		set(i,j,m.value(i,j));
	return (*this);
}

cmatrix& cmatrix::operator=(COMPLEX a) // 将矩阵所有元素设为常数a
{
	if(rownum == 0 || colnum==0 ) return (*this);
	istrans = isneg = isconj = 0; // 标志清零
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++)
		set(i,j,a);
	return (*this);
}

COMPLEX cmatrix::operator()(size_t r, size_t c)
 // 重载函数运算符，返回第r行c列的值
{
	if(r>= rownum || c >= colnum)
		throw TMESSAGE("range overflow");
	return value(r,c);
}

COMPLEX cmatrix::operator()(size_t r)
	// 重载函数运算符，返回第r行0列的值，用于向量
{
	return (*this)(r,0);
}


// 流输出
std::ostream& operator<<(std::ostream& o, cmatrix& m)
{	// 先输出关键词cmatrix,后跟行数和列数
	o << "cmatrix " << m.rownum << '\t' << m.colnum << std::endl;
	// 每行由行号，冒号，后跟一行的复数构成
	for(size_t i=0; i<m.rownum; i++) {
		o<< (i+1) <<':';
		size_t k=8;
		for(size_t j=0; j<m.colnum; j++) {
			o<<'\t'<<m.value(i,j);
			if(--k==0) {
				k=8;
				o<<std::endl;
			}
		}
		o<<std::endl;
	}
	return o;
}

// 流输入
std::istream& operator>>(std::istream& in, cmatrix& m)
{
	char label[10];
	in.width(sizeof(label));
	in >> label;
	if(strcmp(label, "cmatrix")!=0) // 以关键词cmatrix打到
		throw TMESSAGE("format error!");
	in >> m.rownum >> m.colnum; // 后跟行数和列数
	if(!in.good()) throw TMESSAGE("read file error!");
	m.buf->refnum--; // 放弃原来的缓存
	if(!m.buf->refnum) delete m.buf;
	m.buf = getnewcbuffer(m.rownum*m.colnum); // 申请新的缓存
	m.istrans = m.isneg = m.isconj = 0; // 标志清零
	size_t line;
	for(size_t i=0; i<m.rownum; i++) {
		in >> line;
		if(line != i+1) throw TMESSAGE("format error!"); // 每行以行号开始
		in.width(sizeof(label));
		in >> label;
		if(label[0] != ':') throw TMESSAGE("format error!"); // 后跟冒号
		COMPLEX a;
		for(size_t j=0; j<m.colnum; j++) { // 然后是这一行的数据
			in >> a;
			m.set(i,j,a);
		}
		if(!in.good()) throw TMESSAGE("read file error!");
	}
	return in;
}

// 数乘矩阵，原矩阵内容改变为结果
cmatrix& cmatrix::operator*=(COMPLEX a)
{
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++)
		set(i,j,a*value(i,j));
	return (*this);
}

// 数乘矩阵，原矩阵内容不变，返回结果矩阵
cmatrix cmatrix::operator*(COMPLEX a)
{
	cmatrix m(rownum, colnum);
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++)
		m.set(i,j,a*value(i,j));
	return m;
}

cmatrix cmatrix::operator+(COMPLEX a) // 矩阵加常数，产生新矩阵
{
	cmatrix m(rownum, colnum);
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++)
		m.set(i,j,a+value(i,j));
	return m;

}

cmatrix& cmatrix::operator+=(COMPLEX a) // 矩阵加常数，更动原矩阵
{
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++)
		set(i,j,a+value(i,j));
	return (*this);
}

// 矩阵相加
cmatrix cmatrix::operator+(const cmatrix& m)const
{
	if(rownum != m.rownum || colnum != m.colnum) // 行列必须对应相等
		throw TMESSAGE("can not do add of cmatrix");
	cmatrix mm(rownum, colnum);
	COMPLEX a;
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++)
	{
		a = value(i,j)+m.value(i,j);
		mm.set(i,j,a);
	}
	return mm;
}

// 矩阵相加，不产生新的矩阵，修改原矩阵的内容
cmatrix& cmatrix::operator+=(const cmatrix &m)
{
	COMPLEX a;
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<colnum; j++)
	{
		a = value(i,j)+m.value(i,j);
		set(i,j,a);
	}
	return (*this);
}

cmatrix cmatrix::operator-(cmatrix& a)	// 矩阵相减
{
	return (*this)+(-a);
}

cmatrix& cmatrix::operator-=(cmatrix& a) // 矩阵自身减矩阵
{
	(*this)+=(-a);
	return (*this);
}

// 矩阵相乘，返回新产生的结果矩阵
cmatrix cmatrix::operator*(cmatrix& m)
{
	if(colnum != m.rownum) // 前矩阵的列数必须等于后矩阵的行数
		throw TMESSAGE("can not multiply!");
	cmatrix mm(rownum,m.colnum);
	COMPLEX a;
	for(size_t i=0; i<rownum; i++)
	for(size_t j=0; j<m.colnum; j++){
		a = 0.0;
		for(size_t k=0; k<colnum; k++)
			a += value(i,k)*m.value(k,j);
		mm.set(i,j,a);
	}
	return mm;
}

// 矩阵相乘，改变原矩阵的内容
cmatrix& cmatrix::operator*=(cmatrix& m)
{
	(*this) = (*this)*m;
	return (*this);
}

cmatrix cmatrix::operator-()const // 矩阵求负，产生新的矩阵
{
	cmatrix m(*this);
	m.neg();
	return m;
}

cmatrix& cmatrix::neg() // 矩阵自身求负
{
	isneg = !isneg;	// 取负标志求反
	return (*this);
}

cmatrix cmatrix::t()		// 矩阵转置，产生新的矩阵
{
	cmatrix m(*this);
	m.trans();
	return m;
}

cmatrix cmatrix::operator!()		// 矩阵共轭，产生新的矩阵
{
	cmatrix m(*this);
	return m.conj();
}

cmatrix cmatrix::tc()		// 矩阵共轭转置，产生新的矩阵
{
	cmatrix m(*this);
	return m.transconj();
};

cmatrix& cmatrix::operator-=(COMPLEX a) // 矩阵自身减常数
{
	(*this) += (-a);
	return (*this);
}

cmatrix cmatrix::operator-(COMPLEX a)	// 矩阵减常数，产生新的矩阵
{
	return (*this)+(-a);
}

cmatrix operator-(COMPLEX a, cmatrix m) // 常数减矩阵，产生新的矩阵
{
	return (-m)+a;
}

void cmatrix::swapr(size_t r1, size_t r2, size_t k) // 交换矩阵r1和r2两行
{
	COMPLEX a;
	for(size_t i=k; i<colnum; i++) {
		a = value(r1, i);
		set(r1, i, value(r2, i));
		set(r2, i, a);
	}
}

void cmatrix::swapc(size_t c1, size_t c2, size_t k) // 交换c1和c2两列
{
	COMPLEX a;
	for(size_t i=k; i<colnum; i++) {
		a = value(i, c1);
		set(i, c1, value(i, c2));
		set(i, c2, a);
	}
}

DOUBLE cmatrix::maxabs(size_t &r, size_t &c, size_t k)
 // 求第k行和第k列后的主元及位置
{
	DOUBLE a=0.0;
	DOUBLE br,bi;
	for(size_t i=k;i<rownum;i++)
	for(size_t j=k;j<colnum;j++) {
		br = std::real(value(i,j));
		bi = std::imag(value(i,j));
		br = br*br+bi*bi;
		if(a < br) {
			r=i;c=j;a=br;
		}
	}
	return a;
}

size_t cmatrix::zgsxy(cmatrix & m, int fn) // 进行主高斯消元运算，fn为参数，缺省为0
 /* 本矩阵其实是常数阵，而矩阵m必须是方阵
	运算过程其实是对本矩阵和m同时作行初等变换，
	运算结果m的对角线相乘将是行列式，而本矩阵则变换成
	自己的原矩阵被m的逆阵左乘，m的秩被返回，如果秩等于阶数
	则本矩阵中的内容已经是唯一解
 */
{
	if(rownum != m.rownum || m.rownum != m.colnum) // 本矩阵行数必须与m相等
						// 且m必须是方阵
		throw TMESSAGE("can not divide!");
	lbuffer * bb = getnewlbuffer(rownum); // 产生一维数为行数的长整数缓存区
	lbuffer & b = (*bb); // 用引用的办法使下面的程序容易懂
	size_t is;
	COMPLEX a;
	size_t i,j,k,rank=0;
	for(k=0; k<rownum; k++) { // 从第0行到第k行进行主高斯消元
		if(m.maxabs(is, i, k)==0) // 求m中第k级主元，主元所在的行，列存在is,i中
				// 返回的主元值放在a中
			break; // 如果主元为零，则m不可逆，运算失败
		rank = k+1; // rank存放当前的阶数
		b.retrieve(k) = (long)i;  // 将长整数缓存区的第k个值设为i
		if(i != k)
			m.swapc(k, i); // 交换m中i,k两列
		if(is != k) {
			m.swapr(k, is, k); // 交换m中i,k两行,从k列以后交换
			swapr(k, is); // 交换本矩阵中i,k两行
		}
		a = m.value(k,k); // 取出主元值
		for (j=k+1;j<rownum;j++) // 本意是将m的第k行除以主元
				// 但只需把第k行的第k+1列以上除以主元即可
				// 这样还保留了主元作行列式运算用
			m.set(k,j,m.value(k,j)/a);
		for (j=0;j<colnum;j++) // 将本矩阵的第k行除以主元
			set(k,j,value(k,j)/a);
		// 上面两步相当于将m和本矩阵构成的增广矩阵第k行除以主元
		// 下面对增广矩阵作行基本初等变换使第k行的其余列均为零
		// 但0值无必要计算，因此从第k+1列开始计算
		for(j=k+1;j<rownum;j++) // j代表列，本矩阵的行数就是m的列数
		for(i=0;i<rownum;i++) //i代表行，依次对各行计算，k行除外
			if(i!=k)
				m.set(i,j,m.value(i,j)-m.value(i,k)*m.value(k,j));
		// 对本矩阵亦作同样的计算
		for(j=0;j<colnum;j++)
		for(i=0;i<rownum;i++)
			if(i!=k)
				set(i,j,value(i,j)-m.value(i,k)*value(k,j));
	} // 主高斯消元循环k结束
	if(fn == 1) {
		for(j=0; j<rank; j++)
		for(i=0; i<rownum; i++)
			if(i==j) m.set(i,i,1.0);
			else
				m.set(i,j,0.0);
		for(k = rank; k>0; k--)
			m.swapc(k-1,(size_t)b[k-1]);
	}
	for(k = rank; k>0; k--) // 将本矩阵中的各行按b中内容进行调节
		if((size_t)b[k-1] != k-1)
			swapr(k-1,(size_t)b[k-1]); // 行交换
	delete bb; // 释放长整数缓存
	return rank; // 返回mm的秩
}

cmatrix& cmatrix::operator/=(cmatrix m) // 利用重载的除法符号/=来解方程
	// 本矩阵设为d,则方程为mx=d,考虑解写成x=d/m的形式，
	// 而方程的解也存放在d中，则实际编程时写d/=m
{
	if(zgsxy(m)!=rownum) // 如秩不等于m的阶数，则方程无解
		throw TMESSAGE("can not divide!");
	return *this;
}

cmatrix& cmatrix::fft(int l)
{
	if(colnum != 1) throw TMESSAGE("fft need vector!");
	size_t n,i;
	int k;
	n = 1;
	for(k=0;k<10000;k++) {
		if(n>=rownum) break;
		n*=2;
	}
	if(rownum < n) {	// 将数组扩大为2的乘幂维
		cmatrix m(n);
		for(i=0; i<rownum; i++)
			m.set(i,value(i,0));
		for(i=rownum; i<n; i++)
			m.set(i,0.0);
		(*this)=m;
	}
	size_t it,m,is,j,nv;
	int l0;
	COMPLEX q,s,v,podd;
	DOUBLE p;
	cmatrix f(n);
	for (it=0; it<n; it++)
		{ m=it; is=0;
		  for (i=0; i<(size_t)k; i++)
			 { j=m/2; is=2*is+(m-2*j); m=j;}
		  f.set(it,value(is,0));
		}
	set(0,1.0);
	cbuffer & pp = *buf;
	cbuffer & ff = *(f.buf);
	p = 2.0*M_PI/n;
	pp[1] = COMPLEX(cos(p),-sin(p));
	if (l!=0) pp[1] = std::conj(pp[1]);
	for (i=2; i<n; i++)
		pp[i] = pp[1]*pp[i-1];
	for (it=0; it<n-1; it+=2)
		{  v = ff[it];
			ff[it] += ff[it+1];
			ff[it+1] = v - ff[it+1];
		}
	m=n/2; nv=2;
	for (l0=k-2; l0>=0; l0--)
		{ m/=2; nv*=2;
		  for (it=0; it<=(m-1)*nv; it+=nv)
			 for (j=0; j<=(nv/2)-1; j++)
				{
					podd = pp[m*j]*ff[it+j+nv/2];
					ff[it+j+nv/2] = ff[it+j]-podd;
					ff[it+j] += podd;
				}
		}
	 if (l!=0)
		for (i=0; i<n; i++)
		  ff[i]/=(long)n;
	(*this) = f;
	return (*this);
}

cmatrix fft(matrix& m)
{
	cmatrix mm(m);
	return mm.fft();
}

matrix convolute(matrix&a, matrix& b)	// 求矩阵a与b的离散卷积
{
	if(a.colnum != 1 || b.colnum != 1)throw TMESSAGE("must be a vector");
	size_t nn = a.rownum+b.rownum;
	size_t n;
	int k;
	n = 1;
	for(k=0;k<10000;k++) {
		if(n>=nn) break;
		n*=2;
	}
	matrix aa(n),bb(n);
	size_t i;
	aa = 0.0;
	bb = 0.0;
	for(i=0; i<a.rownum; i++)
		aa.set(i,a(i));
	for(i=0; i<b.rownum; i++)
		bb.set(i,b(i));
	cmatrix aafft(fft(aa)),bbfft(fft(bb));
	for(i=0; i<n; i++)
		aafft.set(i,aafft(i)*bbfft(i));
	nn--;
	matrix c(nn),d;
	d = (aafft.fft(1)).real();
	for(i=0;i<nn;i++)
		c.set(i,0,d(i));
	return c;
}

cmatrix& cmatrix::inv()		// 用全选主元高斯-约当法求逆矩阵
{
	if(rownum != colnum) throw TMESSAGE("must be a square matrix");
	size_t n = rownum;
	lbuffer * isp = getnewlbuffer(n);
	lbuffer * jsp = getnewlbuffer(n);
	lbuffer& is = (*isp);
	lbuffer& js = (*jsp);
	size_t i,j,k,u,v;
	DOUBLE d;
	for (k=0; k<n; k++)
		{
			if((d=maxabs(u,v,k))== 0.0) {
				delete isp;
				delete jsp;
				throw TMESSAGE("can not inverse");
			}
			is[k]=(long)u;
			js[k]=(long)v;
/*
		 d=0.0;
		  for (i=k; i<=n-1; i++)
		  for (j=k; j<=n-1; j++)
			 { u=i*n+j;
				p=ar[u]*ar[u]+ai[u]*ai[u];
				if (p>d) { d=p; is[k]=i; js[k]=j;}
			 }
		  if (d+1.0==1.0)
			 { free(is); free(js); printf("err**not inv\n");
				return(0);
			 }
			 */
		  if ((size_t)is[k]!=k) swapr(k,(size_t)is[k]);
/*
			 for (j=0; j<=n-1; j++)
				{ u=k*n+j; v=is[k]*n+j;
				  t=ar[u]; ar[u]=ar[v]; ar[v]=t;
				  t=ai[u]; ai[u]=ai[v]; ai[v]=t;
				} */
		  if ((size_t)js[k]!=k) swapc(k,(size_t)js[k]);
/*			 for (i=0; i<=n-1; i++)
				{ u=i*n+k; v=i*n+js[k];
				  t=ar[u]; ar[u]=ar[v]; ar[v]=t;
				  t=ai[u]; ai[u]=ai[v]; ai[v]=t;
				} */
//		  l=k*n+k;
		  set(k,k,std::conj(value(k,k))/d);
//		  ar[l]=ar[l]/d; ai[l]=-ai[l]/d;
		  for (j=0; j<n; j++)
			 if (j!=k)
				set(k,j,value(k,j)*value(k,k));
/*
				{ u=k*n+j;
				  p=ar[u]*ar[l]; q=ai[u]*ai[l];
				  s=(ar[u]+ai[u])*(ar[l]+ai[l]);
				  ar[u]=p-q; ai[u]=s-p-q;
				} */
		  for (i=0; i<n; i++)
			 if (i!=k)
				{ // v=i*n+k;
				  for (j=0; j<n; j++)
					 if (j!=k)
						set(i,j,value(i,j)-value(k,j)*value(i,k));
/*
						{ u=k*n+j;  w=i*n+j;
						  p=ar[u]*ar[v]; q=ai[u]*ai[v];
						  s=(ar[u]+ai[u])*(ar[v]+ai[v]);
						  t=p-q; b=s-p-q;
						  ar[w]=ar[w]-t;
						  ai[w]=ai[w]-b;
						} */
				}
		  for (i=0; i<n; i++)
			 if (i!=k)
				set(i,k,-value(i,k)*value(k,k));
/*				{ u=i*n+k;
				  p=ar[u]*ar[l]; q=ai[u]*ai[l];
				  s=(ar[u]+ai[u])*(ar[l]+ai[l]);
				  ar[u]=q-p; ai[u]=p+q-s;
				} */
		}
	 for (k=n; k>0; k--)
		{ if ((size_t)js[k-1]!=k-1) swapr((size_t)js[k-1],k-1);
/*			 for (j=0; j<n; j++) swapr((size_t)js[k-1],k-1);
				{ u=k*n+j; v=js[k]*n+j;
				  t=ar[u]; ar[u]=ar[v]; ar[v]=t;
				  t=ai[u]; ai[u]=ai[v]; ai[v]=t;
				} */
		  if ((size_t)is[k-1]!=k-1) swapc((size_t)is[k-1],k-1);
/*			 for (i=0; i<=n-1; i++)
				{ u=i*n+k; v=i*n+is[k];
				  t=ar[u]; ar[u]=ar[v]; ar[v]=t;
				  t=ai[u]; ai[u]=ai[v]; ai[v]=t;
				} */
		}
	delete isp;
	delete jsp;
	return (*this);
}

cmatrix cmatrix::operator~()	// 求逆矩阵，但产生新矩阵
{
	cmatrix m(*this);
	m.inv();
	return m;
}

cmatrix operator/(COMPLEX a, cmatrix m) // 求逆矩阵再乘常数
{
	return (~m)*a;
}

cmatrix& cmatrix::operator/=(COMPLEX a) // 所有元素乘a的倒数，自身改变
{
	(*this)*=1.0/a;
	return (*this);
}

cmatrix cmatrix::operator/(COMPLEX a) // 所有元素乘a的倒数，产生新的矩阵
{
	cmatrix m(*this);
	m/=a;
	return m;
}

