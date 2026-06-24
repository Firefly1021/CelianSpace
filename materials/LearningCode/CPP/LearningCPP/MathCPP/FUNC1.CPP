#include <float.h>
#include <math.h>
#include <stdio.h>
#include "func.h"
#include "cmatrix.h"

algopair::algopair(matrix& xy, size_t m, DOUBLE & dt0,DOUBLE &dt1, DOUBLE &dt2)
// 最小二乘拟合构造函数
// m为拟合多项式的项数，dt0为误差平方和，dt1为误差绝对值和，
// dt2为最大误差绝对值
{
	size_t n = xy.rownum;
	size_t i,j,k;
	DOUBLE z,p,c,g,q,d1,d2,s[20],t[20],b[20];
	if (m>n) m=n;
	if (m>20) m=20;
	data = matrix(m);
	data = 0.0;
	z=0.0;
	for (i=0; i<n; i++) z+=xy(i)/(1.0*n);
	b[0]=1.0; d1=1.0*n; p=0.0; c=0.0;
	for (i=0; i<=n-1; i++)
		{p+=xy(i)-z; c+=xy(i,1);}
	c/=d1; p/=d1;
	data.set(0,c*b[0]);
	if (m>1)
		{ t[1]=1.0; t[0]=-p;
		  d2=0.0; c=0.0; g=0.0;
		  for (i=0; i<=n-1; i++)
			 { q=xy(i)-z-p; d2=d2+q*q;
				c=c+xy(i,1)*q;
				g=g+(xy(i)-z)*q*q;
			 }
		  c=c/d2; p=g/d2; q=d2/d1;
		  d1=d2;
		  data.set(1,c*t[1]);
		  data.set(0,c*t[0]+data(0));
		}
	for (j=2; j<=m-1; j++)
		{ s[j]=t[j-1];
		  s[j-1]=-p*t[j-1]+t[j-2];
		  if (j>=3)
			 for (k=j-2; k>=1; k--)
				s[k]=-p*t[k]+t[k-1]-q*b[k];
		  s[0]=-p*t[0]-q*b[0];
		  d2=0.0; c=0.0; g=0.0;
		  for (i=0; i<=n-1; i++)
			 { q=s[j];
				for (k=j; k>0; k--)
				  q=q*(xy(i)-z)+s[k-1];
				d2+=q*q; c+=xy(i,1)*q;
				g+=(xy(i)-z)*q*q;
			 }
		  c=c/d2; p=g/d2; q=d2/d1;
		  d1=d2;
		  data.set(j,c*s[j]);
		  t[j]=s[j];
		  for (k=j; k>0; k--)
			 { data.set(k-1,data(k-1)+c*s[k-1]);
				b[k-1]=t[k-1]; t[k-1]=s[k-1];
			 }
		}
	 dt0=0.0; dt1=0.0; dt2=0.0;
	 for (i=0; i<=n-1; i++)
		{ q=data(m-1);
		  for (k=m-1; k>0; k--)
			 q=data(k-1)+q*(xy(i)-z);
		  p=q-xy(i,1);
		  if (fabs(p)>dt2) dt2=fabs(p);
		  dt0+=p*p;
		  dt1+=fabs(p);
		}
	 xshift = 0.0;
	 for (i=0; i<n; i++)
		xshift += xy(i);
	 xshift /= n;
}

funcpair::funcpair(matrix& xy, size_t m, DOUBLE & t0,DOUBLE &t1,DOUBLE &t2):
		// xy为n行2列数组，存放n个样本点
			// m为拟合多项式的项数，dt0为误差平方和，dt1为误差绝对值和，
		// dt2为最大误差绝对值
	func(new algopair(xy,m,t0,t1,t2)) {}


cmatrix algopoly::getroots()	// 求出此多项式的所有根
{
	size_t n = data.rownum-1;
	matrix m(n,n);
	matrix a,b;
	m = 0.0;	// 先初始化成零矩阵
	size_t i;
	DOUBLE s;
	s = data(n);
	for(i=0;i<n;i++)
		m.set(0,n-i-1,-data(i)/s);
	for(i=1;i<n;i++)
		m.set(i,i-1,1.0);
	m.qreigen(a,b);
	return cmatrix(a,b);
}

algoregress::algoregress(matrix & xy, matrix* dt)
// xy为n行2列的矩阵为n个样本点
// dt必须指向一个6列一行的矩阵向量，六个数依次为偏差平方和，平均标准
// 偏差，回归平方和，最大偏差，最小偏差，偏差平均值
{
	size_t i,n;
	double xx,yy,e,f,q,u,p,umax,umin,s,a,b;
	xx=0.0; yy=0.0;
	n = xy.rownum;
	for (i=0; i<n; i++)
		{ xx+=xy(i); yy+=xy(i,1);}
	xx/=n; yy/=n;
	e=0.0; f=0.0;
	for (i=0; i<n; i++)
		{ q=xy(i)-xx; e+=q*q;
		  f+=q*(xy(i,1)-yy);
		}
	a=f/e; b=yy-a*xx;
	q=0.0; u=0.0; p=0.0;
	umax=0.0; umin=DBL_MAX; //MAXDOUBLE;
	for (i=0; i<n; i++)
		{ s=a*xy(i)+b;
		  q=q+(xy(i,1)-s)*(xy(i,1)-s);
		  p=p+(s-yy)*(s-yy);
		  e=fabs(xy(i,1)-s);
		  if (e>umax) umax=e;
		  if (e<umin) umin=e;
		  u+=e/n;
		}
	yfactor = a;
	addconst = b;
	if(!dt || dt->rownum < 6) return;
	dt->set(0,q);
	dt->set(1,sqrt(q/n));
	dt->set(2,p);
	dt->set(3,umax);
	dt->set(4,umin);
	dt->set(5,u);
}

funcregress::funcregress(matrix & xy, matrix* dt):
  // xy为n行2列的矩阵为n个样本点
		// dt必须指向一个6列一行的矩阵向量，六个数依次为偏差平方和，平均标准
		// 偏差，回归平方和，最大偏差，最小偏差，偏差平均值
 func(new algoregress(xy,dt)){}

