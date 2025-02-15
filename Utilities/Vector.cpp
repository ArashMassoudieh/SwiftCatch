// Vector.cpp: implementation of the CVector class.
//
//////////////////////////////////////////////////////////////////////

#include "Vector.h"
#include "math.h"
#include "Matrix.h"
#include <cfloat>
#ifdef _arma
#include "Vector_arma.h"
#endif
#include "Utilities.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVector::CVector()
{
	num=0;
}

CVector::CVector(int n)
{
	num = n;
	vec.resize(num);
}

static CVector CreateVector(int n,double value)
{
    CVector out = CVector(n);
    out=value;

}

CVector::CVector(const std::vector<double> a, int n)
{
	num = n;
	vec = a;
}

CVector::CVector(const double x, int n)
{
	num = n;
	vec.resize(num);
	for (int i=0; i<num; i++) vec[i] = x;
}

CVector::CVector(const std::string &filename)
{
    std::ifstream file(filename);

    if (file.good())
    {
    std::vector<double> s = aquiutils::ATOF(aquiutils::getline(file));
        if (s.size() > 1)
        {
            append(s[0]);
            append(s[1]);
        }
    }

}



CVector::CVector(const double x_min, const double x_max, int n)
{
	num = n+1;
	vec.resize(num);
	for (int i=0; i<num; i++) vec[i] = x_min+(x_max-x_min)/double(n)*i;
}

CVector::CVector(const CVector &v)

{
	num = v.num;
	vec = v.vec;
}

CVector::CVector(const std::vector<double> &v)
{
	num = v.size();
	vec = v;
}

#ifdef _arma
CVector::CVector(const CVector_arma &v)
{
    num = v.num();
    vec.resize(num);
	for (int i = 0; i<num; i++)
        vec[i] = v.vect(i);
}
#endif
CVector::CVector(const std::vector<int> &v)
{
	num = v.size();
	vec.resize(num);
	for (int i=0; i<num; i++)
		vec[i] = v[i];
}

CVector::~CVector()
{
	vec.clear();
}

double& CVector::operator[](int i)
{
	double *p = 0;
	if ((i<num) & (i>-1))
		return vec[i];
	else
		return *p;
}

double CVector::operator[](int i) const
{
    double *p = 0;
    if ((i<num) & (i>-1))
        return vec.at(i);
    else
        return *p;
}


double CVector::at(int i) const
{
    if ((i<num) & (i>-1))
        return vec[i];
    else
        return -9999;
}

int CVector::range(int i)
{
	return i;
}

CVector& CVector::operator=(const CVector &v)
{
	num = v.num;
	vec = v.vec;
	return *this;
}

CVector& CVector::operator=(const std::vector<double> &v)
{
	num = v.size();
	vec = v;
	return *this;
}

#ifdef _arma
CVector &CVector::operator=(CVector_arma &v)
{
    vec.reserve(v.num());
    num = v.num();
	for (int i = 0; i<num; ++i)
		vec[i] = v[i];
	return *this;
}
#endif

CVector& CVector::operator=(const double &v)
{
	for (int i=0; i<num; ++i)
		vec[i] = v;
	return *this;
}

void CVector::SetAllValues(const double &v)
{
    for (int i=0; i<num; ++i)
        vec[i] = v;
}


CVector& CVector::operator+()
{return *this;}

void CVector::swap(int i, int j)
{	double tmp = vec[range(i)];
	vec[i] = vec[range(j)];
	vec[j] = tmp;

}

int CVector::getsize() const {return num;}

CVector& CVector::operator*=(double x)
{
	for (int i=0; i<num; ++i)
		vec[i] *= x;
	return *this;

}

CVector& CVector::operator/=(double x)
{
	for (int i=0; i<num; ++i)
		vec[i] /= x;
	return *this;

}

bool CVector::haszeros() const
{
    bool out = false;
    for (int i=0; i<num; ++i)
        if (vec[i] == 0) out = true;

    return out;

}

CVector& CVector::operator+=(const CVector &v)
{
	for (int i=0; i<num; ++i)
		vec[i] += v.vec[i];
	return *this;
}

CVector& CVector::operator-=(const CVector &v)
{
	for (int i=0; i<num; ++i)
		vec[i] -= v.vec[i];
	return *this;
}

CVector operator+(const CVector &v1, const CVector &v2)
{
	CVector v=v1;
	for (int i=0; i<v1.num; i++) v[i]=v1.vec[i]+v2.vec[i];
	return v;
}

CVector operator-(const CVector &v1, const CVector &v2)
{
	CVector v=v1;
	for (int i=0; i<v1.num; i++) v[i]=v1.vec[i]-v2.vec[i];
	return v;

}

double dotproduct(CVector v1, CVector v2)
{
	double d;
	if (v1.num == v2.num)
	{
        d = 0;
        for (int i=0; i<v1.num; ++i)
            d += v1.vec[i]*v2.vec[i];
        return d;
	}
	return 0;
}

CVector& CVector::operator*=(const CVector& v)
{
	for (int i=0; i<num; ++i)
		vec[i] *= v.vec[i];
	return *this;
}


CVector operator*(const CVector &v1, const CVector &v2)
{
    CVector vout(v1.num);
    for (int i=0; i<v1.num; i++)
        vout[i] = v1.at(i)*v2.at(i);
    return vout;
}

double norm(CVector v)
{
	double sum = 0;
	for (int i=0; i<v.num; i++)
		sum += pow(v.vec[i],2);
	return sqrt(sum);
}

CVector operator*(double a, const CVector &v)
{
    CVector vout(v.num);
    for (int i=0; i<v.num; i++)
        vout[i] = v.at(i)*a;
    return vout;

}

CVector operator/(const CVector &v, double a)
{
    CVector v1(v.num);
    for (int i=0; i<v.num; i++)
        v1[i] = v.vec[i]/a;
    return v1;
}

CVector operator+(const CVector &v, double a)
{
	CVector v1(v.num);
	for (int i=0; i<v.num; i++)
        v1[i] = a + v.vec[i];
	return v1;
}

CVector operator+(double a, const CVector &v)
{
	CVector v1(v.num);
	for (int i=0; i<v.num; i++)
        v1[i] = a + v.vec[i];
	return v1;
}

CVector operator-(double a, const CVector &v)
{
	CVector v1(v.num);
	for (int i=0; i<v.num; i++)
        v1[i] = a - v.vec[i];
	return v1;

}

CVector operator-(const CVector &v, double a)
{
	CVector v1(v.num);
	for (int i=0; i<v.num; i++)
        v1[i] = v.vec[i]-a;
	return v1;

}


CVector operator/(const CVector &v1, const CVector &v2)
{
	CVector x(v1.getsize());
	for (int i = 0; i<v1.getsize(); ++i)
        x[i] = v1.vec[i]/v2.vec[i];
	return x;
}

CVector operator/(double a, const CVector &v2)
{
	CVector x(v2.getsize());
	for (int i = 0; i<v2.getsize(); ++i)
        x[i] = a/v2.vec[i];
	return x;

}

bool CVector::operator==(double v)
{
	bool r=true;
	for (int i=0; i<num; ++i)
		if (vec[i] != v)
			r=false;
	return r;

}

bool CVector::operator==(CVector &v)
{
	bool r=true;
	if (num!=v.num) return false;
	for (int i=0; i<num; ++i)
		if ((vec[i]== v[i])!=true)
			r=false;
	return r;

}

bool CVector::is_finite()
{
	bool r=true;
	for (int i=0; i<num; ++i)
        if (aquiutils::isfinite(vec[i])!=true)
			r=false;
	return r;
}

std::string CVector::toString()
{
	std::string s;
    s += aquiutils::numbertostring(vec);
	return s; 
}

double CVector::max() const
{
	double a = -1E14;
	for (int i=0;i<num; i++)
	{
		if (vec[i]>a)
			a = vec[i];
	}
	return a;

}

std::vector<int> CVector::maxelements()
{
    double a = -1E14;
    std::vector<int> max_elements;
	for (int i=0;i<num; i++)
	{
		if (vec[i]>a)
			a = vec[i];
	}
	for (int i=0;i<num; i++)
    {
        if (a==vec[i])
        {
            max_elements.push_back(i);
        }
    }
    return max_elements;
}

double max(CVector &V)
{
	return V.max();
}

double CVector::min() const
{
	double a = 1E14;
	for (int i=0;i<num; i++)
	{
		if (vec[i]<a)
			a = vec[i];
	}
	return a;

}

double min(CVector &V)
{
	return V.min();
}
double CVector::abs_max()
{
	double a = -1E14;
	for (int i=0;i<num; i++)
	{
		if (fabs(vec[i])>a)
			a = fabs(vec[i]);
	}
	return a;
}

int CVector::abs_max_elems()
{
	double a = -1E14;
	int ii;
	for (int i = 0; i<num; i++)
	{
		if (fabs(vec[i]) > a)
		{
			a = fabs(vec[i]);
			ii = i;
		}
	}
	return ii;
}

double abs_max(CVector &V)
{
	return V.abs_max();
}

double CVector::norm2()
{
	double a = 0;
	for (int i=0;i<num; i++)
	{
		a+=vec[i]*vec[i];
	}
	return pow(a,0.5);

}

double CVector::sum() const
{
		double a = 0;
	for (int i=0;i<num; i++)
	{
		a+=vec[i];
	}
	return a;
}

double CVector::mean() const
{
    return sum()/double(num);
}

CMatrix CVector::T()
{
	CMatrix K(1,num);
	for (int i=0; i<num; i++)
		K[0][i] = vec[i];
	return K;
}

CVector CVector::Log() const
{
	CVector x(getsize());
	for (int i = 0; i<getsize(); i++)
		x[i] = log(vec[i]);
	return x;
}

CVector Log(const CVector &V)
{
	return V.Log();

}

double avg(CVector &V)
{
	return V.sum()/V.num;
}

double stdev(CVector &V)
{
    double average = avg(V);
	double sumsquared = 0;
	for (int i=0; i<V.num; i++)
	{
        sumsquared += pow(V[i]-average,2);
	}
	return sqrt(sumsquared/V.num);
}

CVector NormalizetoGaussian(CVector &V)
{
    CVector Vout = (V-avg(V))/stdev(V);
    return Vout;
}

CVector CVector::Exp() const
{
	CVector x(getsize());
	for (int i = 0; i<getsize(); i++)
		x[i] = exp(vec[i]);
	return x;
}

std::vector<int> CVector::Int()
{
	std::vector<int> x(getsize());
	for (int i = 0; i<getsize(); i++)
		x[i] = int(vec[i]);
	return x;
}

CVector CVector::abs()
{
	CVector x(getsize());
	for (int i = 0; i<getsize(); i++)
		x[i] = fabs(vec[i]);
	return x;
}


CVector Exp(const CVector &V)
{
	return V.Exp();

}

CVector abs(CVector &V)
{
	return V.abs();
}

void CVector::writetofile(FILE *f)
{
	for (int i=0; i<num; i++)
		fprintf(f, "%le, ", vec[i]);
	fprintf(f, "\n");
}

void CVector::writetofile(std::ofstream &f)
{
	for (int i=0; i<num-1; i++)
		f<<vec[i]<<",";
	f<<vec[num-1]<<std::endl;

}

void CVector::writetofile(std::string filename)
{
	FILE *f = fopen(filename.c_str(),"w");
	writetofile(f);
	fclose(f);
}

void CVector::writetofile_app(std::string filename)
{
	FILE *f = fopen(filename.c_str(),"a");
	writetofile(f);
	fclose(f);
}

CMatrix CVector::diagmat()
{
	CMatrix A(num,num);
	for (int i=0; i<num; i++)
		A[i][i] = vec[i];

	return A;

}

CVector zeros(int i)
{
	CVector V(i);
	return V;
}

CVector CVector::append(const CVector& V1)
{
	for (int i=0; i<V1.num; i++)
		vec.push_back(V1.vec[i]);
	num += V1.num;
	return *this;
}

CVector CVector::append(double d)
{
	vec.push_back(d);
	num += 1;
	return *this;
}

CVector CVector::sort()
{
	vec = QSort(vec);
	return *this;
}

CVector combinesort(const CVector V1, const CVector V2)
{
	CVector V3 = V1;
	CVector V = V3.append(V2);
	return V.sort();

}

CVector combinesort_s(const CVector V1, const CVector V2)
{
	int n1=0;
	int n2=0;
	CVector V3;
	for (int i=0; i<V1.num+V2.num; i++)
	{
		if (n2==V2.num)
		{	V3.append(V1.vec[n1]);
			n1++;
		}
		else if (n1==V1.num)
		{	V3.append(V2.vec[n2]);
			n2++;
		}
		else
		{
			if (V1.vec[n1]>V2.vec[n2])
			{	V3.append(V2.vec[n2]);
				n2++;
			}
			else
			{	V3.append(V1.vec[n1]);
				n1++;
			}
		}
	}

	return V3;

}

int lookup(std::vector<int> v, int val)
{
	int res = -1;
	for (unsigned int i=0; i<v.size(); i++)
		if (v[i] == val)
			res = i;

	return res;

}

int lookup(std::vector<std::string> v, std::string val)
{
	int res = -1;
	for (unsigned int i=0; i<v.size(); i++)
		if (v[i] == val)
			res = i;

	return res;

}

int lookup(std::vector<double> v, double val)
{
	int res = -1;
	for (unsigned int i=0; i<v.size(); i++)
		if (v[i] == val)
			res = i;
	return res;
}

std::vector<int> CVector::lookup(double val)
{
	std::vector<int> res;
	for (int i=0; i<num; i++)
		if (vec[i] == val)
			res.push_back(i);
	return res;
}

CVector H(CVector &V)
{
	CVector x(V.num);
	for (int i = 0; i<V.num; i++)
		x[i] = H(V[i]);
	return x;

}

double H(double x)
{
	if (x>0) return x; else return 1e-25;

}

std::vector<double> H(std::vector<double> x)
{
	std::vector<double> X(x.size());
	for (unsigned int i=0; i<x.size(); i++)
		X[i] = H(x[i]);

	return X;
}

void CVector::print(std::string s)
{
	std::ofstream Afile;
	Afile.open(s);

	for (int i=0; i<num; ++i)
		Afile << vec[i] << std::endl;

	Afile.close();

}

#ifdef _arma
CVector CVector::operator=(mat A)
{
	num = A.n_rows;
	vec.resize(num);

	for (int i = 0; i<num; ++i)
			vec[i]=A(i,0);

	return *this;
}
#endif

CVector CVector::sub(int i, int j)
{
	CVector C(j-i);
	for (int ii=i; ii<j; ii++)
		C[ii-i] = vec[ii];

	return C;

}
//mat CVector::operator=(const CVector&V)
//{
//	mat A(num,1);

//	for (int i = 0; i<num; ++i)
//			A(i,0) = vec[i];

//	return A;
//}

std::vector<double> create_vector(int i)
{
	std::vector<double> X(i);
	return X;

}

std::vector<std::vector<double>> create_vector(int i, int j)
{
	std::vector<std::vector<double>> X(i);
	for (int ii=0; ii<i; i++)
		X[i].resize(j);

	return X;

}

std::vector<int> CVector::negative_elements()
{
    std::vector<int> out;
    for (int i=0; i<num; i++)
        if (vec[i]<0)
            out.push_back(i);
    return out;

}

CVector CVector::Extract(int start, int end)
{
    if (start<0) start = 0;
    if (end>num-1) end = num-1;
    CVector out(end-start+1);
    for (int i=start; i<=end; i++)
        out[i-start] = vec[i];

    return out;
}

CVector CVector::Extract(const std::vector<double> &x, int start, int end)
{
    if (start<0) start = 0;
    if (end>x.size()-1) end = x.size()-1;
    CVector out(end-start+1);
    for (int i=start; i<=end; i++)
        out[i-start] = x[i];

    return out;
}

double CVector::stdev() const
{
    double avg = mean();
    double sum=0;
    for (int i=0; i<getsize(); i++)
    {
        sum+=pow(vec[i]-avg,2);
    }
    return sqrt(sum/double(getsize()-1));
}

