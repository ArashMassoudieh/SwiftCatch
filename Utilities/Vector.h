
#ifndef C_VECTOR
#define C_VECTOR

#include <iostream>
#include <vector>
#include "QuickSort.h"
#ifdef _arma
#include "armadillo"
using namespace arma;
//using namespace std;
class CVector_arma;
#endif
class CMatrix;
class SizeDist;
class CVector
{
private:


public:
    std::vector<double> vec;
	CVector();
	CVector(int);
	CVector(const std::vector<double>, int);
	CVector(const std::vector<double> &v);
	CVector(const std::vector<int> &v);
	CVector(const std::string &filename);
	CVector(const double x, int n);
    static CVector CreateVector(int n,double value=0);
    CVector(const double x_min, const double x_max, int n);  //cvector:: is redundant. However, works fine here.
	CVector(const CVector&);
    double& operator[](int);
    double operator[](int) const;
    double at(int i) const;
	virtual ~CVector();
    CVector Extract(int start, int end);
    static CVector Extract(const std::vector<double> &x, int start, int end);
	int num;
	int range(int);
	CVector& operator=(const CVector&);
	CVector& operator=(const std::vector<double>&);
#ifdef _arma
    CVector& operator=(CVector_arma&);
    CVector operator=(mat);
    CVector(const CVector_arma&);
#endif
	CVector& operator=(const double &v);
    void SetAllValues(const double &v);
	//mat CVector::operator=(const CVector&);
    CVector& operator+();
    void swap(int , int );
    int getsize() const;
    bool haszeros() const;
    CVector& operator*=(double);
    CVector& operator/=(double);
    CVector& operator+=(const CVector&);
    CVector& operator-=(const CVector&);
    CVector& operator*=(const CVector&);
	friend double dotproduct(CVector, CVector);
	friend CVector mult(CMatrix&, CVector&);
	friend double norm(CVector);			//Friend can be deleted. we don't have any private or protected variable in this class  //
	friend double dotproduct(CVector v1, CVector v2);
	bool operator==(double v);
	bool operator==(CVector &v);
    double max() const;
    double min() const;
	double norm2();
    double sum() const;
    double mean() const;
    double stdev() const;
	double abs_max();
    int abs_max_elems();
	std::vector<int> maxelements();
	CMatrix T();
    CVector Log() const;
	CVector abs();
	CVector H();
	void writetofile(FILE *f);
	void writetofile(std::string filename);
	void writetofile(std::ofstream &f);
	void writetofile_app(std::string filename);
    CVector Exp() const;
	std::vector<int> Int();
	CMatrix diagmat();
	CVector append(const CVector& V1);
	CVector append(double d);
	CVector sort();
	std::vector<int> lookup(double val);
	void print(std::string s);
	CVector sub(int i, int j);
	bool is_finite();
    std::string toString();
    std::vector<int> negative_elements();


};

CVector Log(const CVector &);
CVector Exp(const CVector &);
CVector abs(const CVector &);  //works w/o reference. if const included means read only
double abs_max(const CVector &);
double min(const CVector &);
double max(const CVector &);
CVector H(CVector );
double H(double x);
CVector operator+(const CVector&, const CVector&);
CVector operator+(double, const CVector&);
CVector operator+(const CVector&, double);
CVector operator-(const CVector&, const CVector&);
CVector operator-(double, CVector&);
CVector operator-(const CVector&, double);
CVector operator*(const CVector&, const CVector&);
CVector operator*(double, const CVector&);
CVector operator/(const CVector&, double);
CVector operator/(const CVector&, const CVector&);
CVector operator/(double, const CVector&);
CVector zeros(int i);
CVector combinesort(const CVector& V1, const CVector &V2);
CVector combinesort_s(const CVector& V1, const CVector &V2);
int lookup(std::vector<int> v, int val);
int lookup(std::vector<double> v, double val);
int lookup(std::vector<std::string> v, std::string val);
double avg(CVector &);
double stdev(CVector &V);
CVector NormalizetoGaussian(CVector &V);
std::vector<double> create_vector(int i);
std::vector<std::vector<double> > create_vector(int i, int j);
template<typename T> bool isfinite(T arg);


#endif
