#include "func.h"

DOUBLE gamma(DOUBLE x)
{
	size_t i;
	DOUBLE y,t,s,u;
	static DOUBLE a[11]={ 0.0000677106,-0.0003442342,
			  0.0015397681,-0.0024467480,0.0109736958,
			  -0.0002109075,0.0742379071,0.0815782188,
			  0.4118402518,0.4227843370,1.0};
	if (x<=0.0)
		throw "err**x<=0!\n";
	y=x;
	if (y<=1.0)
		{ t=1.0/(y*(y+1.0)); y+=2.0;}
	else if (y<=2.0)
		{ t=1.0/y; y+=1.0;}
	else if (y<=3.0) t=1.0;
	else
		{ t=1.0;
		  while (y>3.0)
			 { y-=1.0; t=t*y;}
		}
	 s=a[0]; u=y-2.0;
	 for (i=1; i<=10; i++)
		s=s*u+a[i];
	 s*=t;
	 return(s);
}

DOUBLE gamma2(DOUBLE a, DOUBLE x)
{	size_t n;
	DOUBLE p,q,d,s,s1,p0,q0,p1,q1,qq;
	if ((a<=0.0)||(x<0.0))
		{ if (a<=0.0) throw "err**a<=0!\n";
		  if (x<0.0) throw "err**x<0!\n";
		  return(-1.0);
		}
	if (x+1.0==1.0) return(0.0);
	if (x>1.0e+35) return(1.0);
	q=log(x); q=a*q; qq=exp(q);
	if (x<1.0+a)
		{ p=a; d=1.0/a; s=d;
		  for (n=1; n<=100; n++)
			 { p=1.0+p; d=d*x/p; s=s+d;
		 if (fabs(d)<fabs(s)*defaulterr)
				  { s=s*exp(-x)*qq/gamma(a);
					 return(s);
				  }
			 }
		}
	 else
		{ s=1.0/x; p0=0.0; p1=1.0; q0=1.0; q1=x;
		  for (n=1; n<=100; n++)
			 { p0=p1+(n-a)*p0; q0=q1+(n-a)*q0;
				p=x*p0+n*p1; q=x*q0+n*q1;
				if (fabs(q)+1.0!=1.0)
				  { s1=p/q; p1=p; q1=q;
					 if (fabs((s1-s)/s1)<defaulterr)
						{ s=s1*exp(-x)*qq/gamma(a);
						  return(1.0-s);
						}
					 s=s1;
				  }
				p1=p; q1=q;
			 }
		}
//	 printf("a too large !\n");
	 s=1.0-s*exp(-x)*qq/gamma(a);
	 return(s);
}

DOUBLE erf1(DOUBLE x)
{	double y;
	if (x>=0.0)
		y=gamma2(0.5,x*x);
	else
		y=-gamma2(0.5,x*x);
	return(y);
}


static DOUBLE bet(DOUBLE a,DOUBLE b,DOUBLE x);

DOUBLE beta(DOUBLE a,DOUBLE b,DOUBLE x) // 计算不完全贝塔分布函数
  { DOUBLE y;
	 if (a<=0.0) throw "err**a<=0!";
	 if (b<=0.0) throw "err**b<=0!";
	 if ((x<0.0)||(x>1.0)) throw "err**x<0 or x>1 !";
	 if ((x==0.0)||(x==1.0)) y=0.0;
	 else
		{ y=a*log(x)+b*log(1.0-x);
		  y=exp(y);
		  y*=gamma(a+b)/(gamma(a)*gamma(b));
		}
	 if (x<(a+1.0)/(a+b+2.0))
		y*=bet(a,b,x)/a;
	 else
		y=1.0-y*bet(b,a,1.0-x)/b;
	 return(y);
  }

static DOUBLE bet(DOUBLE a,DOUBLE b,DOUBLE x)
{ size_t k;
	 DOUBLE d,p0,q0,p1,q1,s0,s1;
	 p0=0.0; q0=1.0; p1=1.0; q1=1.0;
	 for (k=1; k<=100; k++)
		{ d=(a+k)*(a+b+k)*x;
		  d=-d/((a+k+k)*(a+k+k+1.0));
		  p0=p1+d*p0; q0=q1+d*q0; s0=p0/q0;
		  d=k*(b-k)*x;
		  d=d/((a+k+k-1.0)*(a+k+k));
		  p1=p0+d*p1; q1=q0+d*q1; s1=p1/q1;
		  if (fabs(s1-s0)<fabs(s1)*defaulterr)
			 return(s1);
		}
//	 printf("a or b too big !");
	 return(s1);
}

DOUBLE gass(DOUBLE a,DOUBLE d,DOUBLE x)
{	DOUBLE y;
	if (d<=0.0) d=1.0e-10;
	y=0.5+0.5*erf((x-a)/(sqrt(2.0)*d));
	return(y);
}

DOUBLE student(DOUBLE t, size_t n)
  { DOUBLE y;
	 if (t<0.0) t=-t;
	 y=1.0-beta(n/2.0,0.5,n/(n+t*t));
	 return(y);
  }

DOUBLE chii(DOUBLE x,size_t n)
{   DOUBLE y;
	 if (x<0.0) x=-x;
	 y=gamma2(n/2.0,x/2.0);
	 return(y);
}

DOUBLE fdisp(DOUBLE f,size_t n1,size_t n2)
{	DOUBLE y;
	if (f<0.0) f=-f;
	y=beta(n2/2.0,n1/2.0,n2/(n2+n1*f));
	return(y);
}

DOUBLE integral(DOUBLE (*f)(DOUBLE),DOUBLE a, DOUBLE b, DOUBLE eps)
 // 函数f,在(a,b)区间积分,采用勒让德高斯求积法
{
	 size_t m,i,j;
	 DOUBLE s,p,ep,h,aa,bb,w,x,g;
	 g = 0;
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
					 w=w+f(x)*c[j];  //flrgsf(x)*c[j];
				  }
				g=g+w;
			 }
		  g *= h/2.0;
		  ep=fabs(g-p)/(1.0+fabs(g));
		  p=g; m=m+1; h=(b-a)/m;
		}
	 return g;
}

static DOUBLE sinc(DOUBLE x)
{
	return sin(x)/x;
}

DOUBLE sinn(DOUBLE x)
{
	if(x==0.0)return 0;
	if(x<0.0) throw "x less than 0\n";
	return integral(sinc,0,x);
}

static DOUBLE cosc(DOUBLE x)
{
	return (1-cos(x))/x;
}

#define EULER 0.57721566490153286060651

DOUBLE coss(DOUBLE x)
{
	DOUBLE r=EULER;
	if (x<0) throw "x less than 0\n";
	if (x==0) x=1.0e-35;
	return r+log(x)-integral(cosc,0,x);
}

static DOUBLE ex(DOUBLE x)
{
	return (exp(-x)-1)/x;
}

DOUBLE expp(DOUBLE x)
{
	if(x<0) throw "x less than 0\n";
	if(x==0) x=1.0e-10;
	DOUBLE r=EULER;
	return r+log(x)+integral(ex,0,x);
}

DOUBLE getroot(DOUBLE (*f)(DOUBLE), DOUBLE x0, DOUBLE eps)
{	int i,j,m,l;// it
	double a[10],y[10],z,h,q;
	l=10; q=1.0e+35; h=0.0;
	while (l!=0)
		{ l=l-1; j=0; // it=l;
		  while (j<=7)
			  { if (j<=2) z=x0+0.1*j;
				 else z=h;
				 y[j]=f(z);
				 h=z;
				 if (j==0) a[0]=z;
				 else
					{ m=0; i=0;
					  while ((m==0)&&(i<=j-1))
						 { if (fabs(h-a[i])==0.0) m=1;
							else h=(y[j]-y[i])/(h-a[i]);
							i=i+1;
						 }
					  a[j]=h;
					  if (m!=0) a[j]=q;
					  h=0.0;
					  for (i=j-1; i>=0; i--)
						 { if (fabs(a[i+1]+h)==0.0) h=q;
							else h=-y[i]/(a[i+1]+h);
						 }
					  h=h+a[0];
					}
				 if (fabs(y[j])>=eps) j=j+1;
				 else { j=10; l=0;}
			  }
			x0=h;
		 }
	if(fabs(f(x0)) > eps) throw "no root!";
	return x0;
}

DOUBLE getroot(algo & alg, DOUBLE x0, DOUBLE eps)
{	int i,j,m,l;
	double a[10],y[10],z,h,q;
	l=10; q=1.0e+35; h=0.0;
	while (l!=0)
		{ l--; j=0;
		  while (j<=7)
			  { if (j<=2) z=x0+0.1*j;
				 else z=h;
				 y[j]=alg.cal(z);
				 h=z;
				 if (j==0) a[0]=z;
				 else
					{ m=0; i=0;
					  while ((m==0)&&(i<=j-1))
						 { if (fabs(h-a[i])==0.0) m=1;
							else h=(y[j]-y[i])/(h-a[i]);
							i=i+1;
						 }
					  a[j]=h;
					  if (m!=0) a[j]=q;
					  h=0.0;
					  for (i=j-1; i>=0; i--)
						 { if (fabs(a[i+1]+h)==0.0) h=q;
							else h=-y[i]/(a[i+1]+h);
						 }
					  h=h+a[0];
					}
				 if (fabs(y[j])>=eps) j=j+1;
				 else { j=10; l=0;}
			  }
			x0=h;
		 }
	if(fabs(alg.cal(x0)) > eps) throw "no root!";
	return x0;
}

