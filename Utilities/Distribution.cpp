#include "Distribution.h"
#include "gsl/gsl_cdf.h"
#include "Utilities.h"
#ifdef _interface
#include "interface.h"
#include "command.h"
#endif

#ifdef _interface
CDistribution::CDistribution(void):Interface()
#else
CDistribution::CDistribution(void)
#endif // interface
{
	pi = 4 * atan(1.0);
}

bool CDistribution::CreateDistribution(const map<string,string> &Arguments)
{
    if (Arguments.count("type")==0)
        return false;
    name = Arguments.at("type");
    params.clear();
    for (map<string,string>::const_iterator it=Arguments.begin(); it!=Arguments.end(); it++)
    {
        if (aquiutils::left(it->first,1) == "p" && aquiutils::isnumber(aquiutils::right(it->first,it->first.size()-1)))
            params.push_back(atof(it->second.c_str()));
    }
    return true;
}

CDistribution::~CDistribution(void)
{

}

CDistribution::CDistribution(string _name)
{
	pi = 4 * atan(1.0);
	name = _name;
    if (name == "normal") {DistributionType=distribution__type::normal;}
    if (name == "lognormal") {DistributionType=distribution__type::lognormal;}
    if (name == "levy") {DistributionType=distribution__type::levy;}
    if (name == "exp") {DistributionType=distribution__type::exponential;}
    if (name == "invgaussian") {DistributionType=distribution__type::inverse_gaussian;}
    if (name == "gamma") {DistributionType=distribution__type::gamma;}
    if (name == "nonparametric") {DistributionType=distribution__type::nonparameteric;}
    params.resize(NumberOfCoreParameters(name)+1);

}

int CDistribution::NumberOfCoreParameters(string _name)
{
    if (_name == "normal") {return 2;}
    if (_name == "lognormal") {return 2;}
    if (_name == "levy") {return 1;}
    if (_name == "exp") {return 1;}
    if (_name == "invgaussian") {return 2;}
    if (_name == "gamma") {return 2;}
    if (_name == "nonparametric") {return 0;}
    return -1;
}


double CDistribution::evaluate(double x)
{
    double out = 0;
    if (name == "nonparameteric")
        return density.interpol(x);
    int m = NumberOfCoreParameters(name)+1;
    int n=params.size()/m;
    for (unsigned int i=0; i<n; i++)
    {
        if (name == "normal")
            out+= params[i*m] / (sqrt(2*pi)*params[i*m+2])*exp(-pow(x - params[i*m+1], 2) / (2 * params[i*m+2] * params[i*m+2]));
        if (name == "lognormal")
        {
            if (x <= 0)
                out = 0; 
            else
                out += params[i * m] / (sqrt(2 * pi) * params[i * m + 2] * x) * exp(-pow(log(x) - params[i * m + 1], 2) / (2 * params[i * m + 2] * params[i * m + 2]));
        }
        if (name == "levy")
            out += params[i*m]*sqrt(params[i*m+1] / (2 * pi))*exp(-params[i*m+1] / (2 * x)) / pow(x, 1.5);
        if (name == "exp")
            out +=  params[i*m] / params[i*m+1] * exp(x / params[i*m+1]);
        if (name == "invgaussian")
            out +=  params[i*m]*sqrt(params[i*m+2] / (2 * pi*pow(x, 3)))*exp(-params[i*m+2] * pow(x - params[i*m+1], 2) / (2 * params[i*m+1] * params[i*m+1] * x));
        if (name == "gamma")
            out+= params[i*m]*Gammapdf(x,params[i*m+1],params[i*m+2]);

    }
    return out;
}



double Gammapdf(double x, double k, double theta)
{
	return 1 / pow(theta, k)*pow(x, k - 1)*exp(-x / theta) / gamma(k);
}

double gamma(double x)
{
	int i, k, m;
	double ga, gr, r, z;
	double m_PI = atan(1.0)*4.0;
	static double g[] = {
		1.0,
		0.5772156649015329,
		-0.6558780715202538,
		-0.420026350340952e-1,
		0.1665386113822915,
		-0.421977345555443e-1,
		-0.9621971527877e-2,
		0.7218943246663e-2,
		-0.11651675918591e-2,
		-0.2152416741149e-3,
		0.1280502823882e-3,
		-0.201348547807e-4,
		-0.12504934821e-5,
		0.1133027232e-5,
		-0.2056338417e-6,
		0.6116095e-8,
		0.50020075e-8,
		-0.11812746e-8,
		0.1043427e-9,
		0.77823e-11,
		-0.36968e-11,
		0.51e-12,
		-0.206e-13,
		-0.54e-14,
		0.14e-14 };

	if (x > 171.0) return 1e308;    // This value is an overflow flag.
	if (x == (int)x) {
		if (x > 0.0) {
			ga = 1.0;               // use factorial
			for (i = 2; i<x; i++) {
				ga *= i;
			}
		}
		else
			ga = 1e308;
	}
	else {
		if (fabs(x) > 1.0) {
			z = fabs(x);
			m = (int)z;
			r = 1.0;
			for (k = 1; k <= m; k++) {
				r *= (z - k);
			}
			z -= m;
		}
		else
			z = x;
		gr = g[24];
		for (k = 23; k >= 0; k--) {
			gr = gr*z + g[k];
		}
		ga = 1.0 / (gr*z);
		if (fabs(x) > 1.0) {
			ga *= r;
			if (x < 0.0) {
				ga = -m_PI / (x*ga*sin(m_PI*x));
			}
		}
	}
	return ga;
}


CDistribution::CDistribution(int nn)
{
	pi = 4 * atan(1.0);
	n = nn;
	e.resize(n);
	s.resize(n);
}

CDistribution::CDistribution(const CDistribution &C)
{
	pi = 4 * atan(1.0);
	n = C.n;
	e.resize(n);
	s.resize(n);
	for (int i = 0; i<n; i++)
	{
		e[i] = C.e[i];
		s[i] = C.s[i];
	}
    inverse_cumulative = C.inverse_cumulative;
    density = C.density;

}

CDistribution CDistribution::operator = (const CDistribution &C)
{
	pi = 4 * atan(1.0);
	n = C.n;
	e.resize(n);
	s.resize(n);
	for (int i = 0; i<n; i++)
	{
		e[i] = C.e[i];
		s[i] = C.s[i];
	}
    inverse_cumulative = C.inverse_cumulative;
    density = C.density;
	return *this;

}

int CDistribution::GetRand()
{
    double x = unitrandom();
	int ii = 0;
	for (int i = 0; i<n - 1; i++)
	{
		if (x<e[i] && x>s[i])
			ii = i;
	}
	return ii;

}



double CDistribution::inverseCDF(double u,bool flux_w)
{
	if (aquiutils::tolower(name) == "exp" || aquiutils::tolower(name) == "exponential")
	{
		return -params[0] * log(1.0 - u);
	}
	if (aquiutils::tolower(name) == "gamma")
	{
		return gsl_cdf_gamma_Pinv(u, params[0], params[1]);
	}
	if (aquiutils::tolower(name) == "lognormal" || aquiutils::tolower(name) == "log-normal")
	{
		if (flux_w)
            return gsl_cdf_lognormal_Pinv(u, params[0]+pow(params[1],2), params[1]);
        else
            return gsl_cdf_lognormal_Pinv(u, params[0], params[1]);
	}
	if (aquiutils::tolower(name) == "normal" || aquiutils::tolower(name) == "gaussian")
	{
		return gsl_cdf_gaussian_Pinv(u, params[1])+ params[0];
	}
	if (aquiutils::tolower(name) == "power" || aquiutils::tolower(name) == "power_law")
	{
		return pow(2 * u, 1 / (params[0] - 1))*params[1];
	}
	if (aquiutils::tolower(name) == "nonparameteric")
	{
        return inverse_cumulative.interpol(u);
	}
	return 0;

}

double unifrandom(double xmin, double xmax)
{
	double a = double(rand());
	double k = double(RAND_MAX);
	return a / k*(xmax - xmin) + xmin;
}

double std_normal_phi_inv(const double &u)
{
	double u1 = gsl_cdf_ugaussian_Pinv(u);
    double out = 1.0 / sqrt(2.0 * 4.0 * atan(1.0))*exp(-0.5*pow(u1, 2.0));
    return out;
}

double stdnormal_inv(const double &u)
{
    return gsl_cdf_ugaussian_Pinv(u);
}

double CDistribution::evaluate_CDF(double x, bool flux_w)
{
    if (aquiutils::tolower(name) == "lognormal" || aquiutils::tolower(name) == "log-normal")
	{
		if (flux_w)
            return gsl_cdf_lognormal_P(x, params[0]+pow(params[1],2), params[1]);
        else
            return gsl_cdf_lognormal_P(x, params[0], params[1]);
	}
}

bool CDistribution::readfromfile(const string &filename)
{
    if (!density.readfile(filename))
    {
        cout<< "File [" << filename << "] was not found!"<<endl;
        return false;
    }
    CTimeSeries<double> cumulative = density.getcummulative();
    cumulative = cumulative/cumulative.GetC(cumulative.n-1);
    inverse_cumulative = cumulative.inverse_cumulative_uniform();
    return true;
}
#ifdef _interface
vector<string> CDistribution::commands()
{
    return Commands();
}

vector<string> CDistribution::Commands()
{
    vector<string> cmds;
    for (map<string,command_parameters>::iterator i=Command::Command_Structures.begin(); i!=Command::Command_Structures.end(); i++)
    {
        if (i->second.Object==object_type::distribution)
            cmds.push_back(i->first);
    }
    return cmds;
}

bool CDistribution::HasCommand(const string &cmd)
{
    if (aquiutils::lookup(Commands(),cmd)!=-1)
        return true;
    else
        return false;
}
#endif // interface
bool CDistribution::WriteToFile(const map<string,string> Arguments)
{

    int nbins = 100;
    if (Arguments.count("filename")==0)
        return false;
    if (DistributionType == distribution__type::nonparameteric)
    {
        density.writefile(Arguments.at("filename"));
        return true;
    }
    if (params.size()==0)
        return false;
    if (Arguments.count("nbins")>0)
        nbins = aquiutils::atof(Arguments.at("nbins"));

    vector<double> range = SetRangeBasedOnMeanStd(3);
    CTimeSeries<double> timeseries;
    for (double x = range[0]; x<=range[1]; x+=(range[1]-range[0])/nbins)
    {
        timeseries.append(x, evaluate(x));
    }
    timeseries.writefile(Arguments.at("filename"));
    return true;
}

CTimeSeries<double> CDistribution::ToTimeSeries(int nbins, const double& stdcoeff)
{
    vector<double> range = SetRangeBasedOnMeanStd(stdcoeff);
    CTimeSeries<double> timeseries;
    for (double x = range[0]; x <= range[1]; x += (range[1] - range[0]) / nbins)
    {
        timeseries.append(x, evaluate(x));
    }
    return timeseries;
}

vector<double> CDistribution::SetRangeBasedOnMeanStd(const double &stdcoeff)
{
    vector<double> range(2);
    int n=NumberOfCoreParameters(name);
    range[0] = 1e12;
    range[1] = -1e12;
    for (int i=0; i<params.size()/NumberOfCoreParameters(name); i++)
    {   if (name=="normal")
        {
            range[0]=min(params[i*n+1]-stdcoeff*params[i*n+2],range[0]);
            range[1]=max(params[i*n+1]+stdcoeff*params[i*n+2],range[1]);
        }
        else if (name=="lognormal")
        {
            range[0]=min(exp(params[i*n+1])*exp(-stdcoeff*params[i*n+2]),range[0]);
            range[1]=max(exp(params[i*n+1])*exp(stdcoeff*params[i*n+2]),range[1]);
        }
        else if (name=="exponential")
        {
            range[0] = 0;
            range[1] = max(stdcoeff/params[i*n+1],range[1]);

        }
    }
    return range;
}

#ifdef _interface

FunctionOutPut CDistribution::Execute(const string &cmd, const map<string,string> &arguments)
{
    FunctionOutPut output;
    if (cmd=="CreateDistribution")
    {   output.success = CreateDistribution(arguments);
        output.output = this;

    }
    if (cmd=="WriteDistToFile")
    {   output.success = WriteToFile(arguments);

    }
    if (cmd=="SetInverseCumulative")
    {   output.success = SetInverseCumulative(arguments);

    }
    if (cmd=="WriteInverseCumulativeToFile")
    {   output.success = WriteInverseCumulativeToFile(arguments);

    }
    return output;
}
#endif // interface
bool CDistribution::SetInverseCumulative(const map<string,string> &arguments)
{
    if (arguments.count("ninc")==0) return false;
    int ninc = aquiutils::atoi(arguments.at("ninc"));
    inverse_cumulative.clear();
    double epsilon = 1e-8;

    for (int i = 0; i < ninc+1; i++)
    {
        double u = double(i) / double(ninc)*(1 - 2 * epsilon) + epsilon;
        inverse_cumulative.append(u, InverseCumulativeValue(u));
    }
    return true;
}

bool CDistribution::WriteInverseCumulativeToFile(const map<string,string> &arguments)
{
    if (inverse_cumulative.n==0)
        return false;
    if (arguments.count("filename")==0) return false;
    return inverse_cumulative.writefile(arguments.at("filename"));
}

double CDistribution::InverseCumulativeValue(double u)
{
    double x2 = 1000;
    double x1 = 0.001;
    double tol = 1e-8;
    double err1 = log(CumulativeValue(x1)) - log(u);
    double err2 = log(CumulativeValue(x2)) - log(u);
    while (err1 > 0)
    {
        x1 /= 2;
        err1 = log(CumulativeValue(x1)) - log(u);
    }

    while (err2 < 0)
    {
        x2 *= 2;
        err2 = log(CumulativeValue(x2)) - log(u);
    }

    while (min(fabs(err1),fabs(err2)) > tol && fabs(x1-x2)>tol)
    {
        double slope = (err2 - err1) / (log(x2) - log(x1));
        double x_p = exp(log(x1) - err1 / slope);
        double err_p = log(CumulativeValue(x_p)) - log(u);
        if (err_p > 0)
        {
            x2 = x_p;
            err2 = log(CumulativeValue(x2)) - log(u);
        }
        else
        {
            x1 = x_p;
            err1 = log(CumulativeValue(x1)) - log(u);
        }

    }
    if (fabs(err1) > fabs(err2))
        return x2;
    else
        return x1;
}

double CDistribution::CumulativeValue(double x)
{
    double out = 0;
    int m = NumberOfCoreParameters(name)+1;
    int n = params.size() / (NumberOfCoreParameters(name)+1);
    if (this->name == "lognormal")
    {
        for (int i = 0; i < n; i++)
        {
            double _erf = erf((log(x) - params[m * i + 1]) / (sqrt(2.0)*params[m * i + 2]));
            out += params[m * i] * 0.5*(1.0 + _erf);

        }
    }
    else
    {

    }
    return out;

}

double CDistribution::unitrandom()
{
    double x1 = rand();
    double x2 = rand();
    return (x1 + x2 * (double(RAND_MAX) - 1)) / (double(RAND_MAX) * double(RAND_MAX));
}

double CDistribution::getstdnormalrand()
{
    double x1 = unitrandom();
    double x2 = unitrandom();
    double pi = atan(1.0) * 4;
    double y1 = sqrt(-2 * log(x1)) * cos(2 * pi * x2);
    return y1;
}

double CDistribution::getnormalrand(double mu, double std)
{
    return getstdnormalrand() * std + mu;
}

double unitrandom()
{
    double x1 = rand();
    double x2 = rand();
    return (x1 + x2 * (double(RAND_MAX) - 1)) / (double(RAND_MAX) * double(RAND_MAX));
}

double getstdnormalrand()
{
    double x1 = unitrandom();
    double x2 = unitrandom();
    double pi = atan(1.0) * 4;
    double y1 = sqrt(-2 * log(x1)) * cos(2 * pi * x2);
    return y1;
}

double getnormalrand(double mu, double std)
{
    return getstdnormalrand() * std + mu;
}
double getnormalcdf(double x, double mu, double std)
{
    return stdnormal_cdf((x - mu) / std);
}

CVector getnormal(CVector& mu, CMatrix& sigma)
{
    CMatrix L = sigma.Cholesky_factor();
    CVector V = getnormal(L.getnumrows(), 0, 1);
    return L * V + mu;
}

CMatrix getnormal(int m, int n, double mu, double std)
{
    CMatrix M(m, n);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            M[i][j] = getstdnormalrand() * std + mu;

    return M;
}

CVector getnormal(int m, double mu, double std)
{
    CVector M(m);
    for (int i = 0; i < m; i++)
        M[i] = getstdnormalrand() * std + mu;

    return M;
}

double getlognormalrand(double mu, double std)
{
    return exp(getstdnormalrand() * std + mu);
}

CVector getlognormal(CVector& mu, CMatrix& sigma)
{
    CVector V = getnormal(mu, sigma);
    return Exp(V);
}

CMatrix getlognormal(int m, int n, double mu, double std)
{
    CMatrix M(m, n);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            M[i][j] = exp(getstdnormalrand() * std + mu);

    return M;
}

CVector getlognormal(int m, double mu, double std)
{
    CVector M(m);
    for (int i = 0; i < m; i++)
        M[i] = exp(getstdnormalrand() * std + mu);

    return M;
}

double getpdfnormal(CVector& X, CVector& mu, CMatrix& std)
{
    int k = X.num;
    double pi = atan(1.0) * 4;
    CMatrix Mu = X - mu;
    CMatrix Mu_p = X.T() - mu.T();
    CMatrix M_inv = Invert(std);
    CMatrix M1 = Mu_p * M_inv;
    CMatrix exparg = 0.5 * (M1 * Mu);

    double pdf = 1 / pow(2 * pi, k) / sqrt(std.det()) * exp(-exparg[0][0]);
    return pdf;

}

double getpdflognormal(CVector& X, CVector& mu, CMatrix& std)
{
    int k = X.num;
    double pi = atan(1.0) * 4;
    CMatrix Mu = Log(X) - mu;
    CMatrix TransposeMu = X.T();
    CMatrix Mu_p = Log(TransposeMu) - mu.T();
    CMatrix M_inv = Invert(std);
    CMatrix M1 = Mu_p * M_inv;
    CMatrix exparg = 0.5 * (M1 * Mu);

    double pdf = 1 / pow(2 * pi, k) / sqrt(fabs(std.det())) * exp(-exparg[0][0]);
    return pdf;

}
double stdnormal_cdf(double u)
{
    const double a[5] = {
        1.161110663653770e-002,3.951404679838207e-001,2.846603853776254e+001,
        1.887426188426510e+002,3.209377589138469e+003
    };
    const double b[5] = {
        1.767766952966369e-001,8.344316438579620e+000,1.725514762600375e+002,
        1.813893686502485e+003,8.044716608901563e+003
    };
    const double c[9] = {
        2.15311535474403846e-8,5.64188496988670089e-1,8.88314979438837594e00,
        6.61191906371416295e01,2.98635138197400131e02,8.81952221241769090e02,
        1.71204761263407058e03,2.05107837782607147e03,1.23033935479799725E03
    };
    const double d[9] = {
        1.00000000000000000e00,1.57449261107098347e01,1.17693950891312499e02,
        5.37181101862009858e02,1.62138957456669019e03,3.29079923573345963e03,
        4.36261909014324716e03,3.43936767414372164e03,1.23033935480374942e03
    };
    const double p[6] = {
        1.63153871373020978e-2,3.05326634961232344e-1,3.60344899949804439e-1,
        1.25781726111229246e-1,1.60837851487422766e-2,6.58749161529837803e-4
    };
    const double q[6] = {
        1.00000000000000000e00,2.56852019228982242e00,1.87295284992346047e00,
        5.27905102951428412e-1,6.05183413124413191e-2,2.33520497626869185e-3
    };
    double y, z;

    if (std::isnan(u))
        return NAN;
    if (!std::isfinite(u))
        return (u < 0 ? 0.0 : 1.0);
    y = fabs(u);
    if (y <= 0.46875 * sqrt(double(2))) {
        /* evaluate erf() for |u| <= sqrt(2)*0.46875 */
        z = y * y;
        y = u * ((((a[0] * z + a[1]) * z + a[2]) * z + a[3]) * z + a[4])
            / ((((b[0] * z + b[1]) * z + b[2]) * z + b[3]) * z + b[4]);
        return 0.5 + y;
    }
    z = exp(-y * y / 2) / 2;
    if (y <= 4.0) {
        /* evaluate erfc() for sqrt(2)*0.46875 <= |u| <= sqrt(2)*4.0 */
        y = y / sqrt(double(2.0));
        y =
            ((((((((c[0] * y + c[1]) * y + c[2]) * y + c[3]) * y + c[4]) * y + c[5]) * y + c[6]) * y + c[7]) * y + c[8])


            / ((((((((d[0] * y + d[1]) * y + d[2]) * y + d[3]) * y + d[4]) * y + d[5]) * y + d[6]) * y + d[7]) * y + d[8]);

        y = z * y;
    }
    else {
        /* evaluate erfc() for |u| > sqrt(2)*4.0 */
        z = z * sqrt(double(2)) / y;
        y = 2 / (y * y);
        y = y * (((((p[0] * y + p[1]) * y + p[2]) * y + p[3]) * y + p[4]) * y + p[5])
            / (((((q[0] * y + q[1]) * y + q[2]) * y + q[3]) * y + q[4]) * y + q[5]);
        y = z * (sqrt(atan(double(1.0) * 4)) - y);
    }
    return (u < 0.0 ? y : 1 - y);
};

double stdnormal_inv(double p)
{
    const double a[6] = {
        -3.969683028665376e+01,  2.209460984245205e+02,
        -2.759285104469687e+02,  1.383577518672690e+02,
        -3.066479806614716e+01,  2.506628277459239e+00
    };
    const double b[5] = {
        -5.447609879822406e+01,  1.615858368580409e+02,
        -1.556989798598866e+02,  6.680131188771972e+01,
        -1.328068155288572e+01
    };
    const double c[6] = {
        -7.784894002430293e-03, -3.223964580411365e-01,
        -2.400758277161838e+00, -2.549732539343734e+00,
        4.374664141464968e+00,  2.938163982698783e+00
    };
    const double d[4] = {
        7.784695709041462e-03,  3.224671290700398e-01,
        2.445134137142996e+00,  3.754408661907416e+00
    };

    double q, t, u;

    if (std::isnan(p) || p > 1.0 || p < 0.0)
        return NAN;
    if (p == 0.0)
        return -1e303;
    if (p == 1.0)
        return  1e303;
    q = min(p, 1 - p);
    if (q > 0.02425) {
        /* Rational approximation for central region. */
        u = q - 0.5;
        t = u * u;
        u = u * (((((a[0] * t + a[1]) * t + a[2]) * t + a[3]) * t + a[4]) * t + a[5])
            / (((((b[0] * t + b[1]) * t + b[2]) * t + b[3]) * t + b[4]) * t + 1);
    }
    else {
        /* Rational approximation for tail region. */
        t = sqrt(-2 * log(q));
        u = (((((c[0] * t + c[1]) * t + c[2]) * t + c[3]) * t + c[4]) * t + c[5])
            / ((((d[0] * t + d[1]) * t + d[2]) * t + d[3]) * t + 1);
    }
    /* The relative error of the approximation has absolute value less
    than 1.15e-9.  One iteration of Halley's rational method (third
    order) gives full machine precision... */
    t = stdnormal_cdf(u) - q;    /* error */
    t = t * sqrt(atan(double(1.0) * 4)) * exp(u * u / 2);   /* f(u)/df(u) */
    u = u - t / (1 + u * t / 2);     /* Halley's method */

    return (p > 0.5 ? -u : u);
};
