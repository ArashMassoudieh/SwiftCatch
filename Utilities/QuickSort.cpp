#include "QuickSort.h"
#include <iostream>
#include <algorithm>	// std::swap()
#include <vector>
#include "time.h"
#ifdef QT_version
#include "qdebug.h"
#endif // QT_version
//using namespace std;

CQuickSort::CQuickSort(void)
{
}


CQuickSort::~CQuickSort(void)
{

}

std::vector<double> QSort(const std::vector<double> &V1)
{

	if (V1.size() <= 1) return V1;
	std::vector<double> V = V1;
	int end = V.size();
	if (V[end - 1]<V[0]) V = reverse_order(V);
	std::vector<double> less, greater;
	greater.push_back(V[end - 1]);
	for (int i = 0; i<end - 1; i++)
		if (V[i]<V[end - 1]) less.push_back(V[i]);
		else greater.push_back(V[i]);


		if ((V == greater) && (less.size() == 0))
			return greater;
		std::vector<double> res = QSort(less);
		std::vector<double> x2 = QSort(greater);

		res.insert(res.end(), x2.begin(), x2.end());
		less.clear();
		greater.clear();
		x2.clear();
		return res;

}

std::vector<double> QbSort(const std::vector<double> &V1)
{

	if (V1.size() < 100) return bubbleSort(V1);
	if (V1.size() <= 1) return V1;
	std::vector<double> V = V1;
	int end = V.size();
	if (V[end - 1]<V[0]) V = reverse_order(V);
	std::vector<double> less, greater;
	greater.push_back(V[end - 1]);
	for (int i = 0; i<end - 1; i++)
		if (V[i]<V[end - 1]) less.push_back(V[i]);
		else greater.push_back(V[i]);


		if ((V == greater) && (less.size() == 0))
			return greater;
		std::vector<double> res = QSort(less);
		std::vector<double> x2 = QSort(greater);

		res.insert(res.end(), x2.begin(), x2.end());
		less.clear();
		greater.clear();
		x2.clear();
		return res;

}

std::vector<int> QSort(const std::vector<int> &V1)
{
	if (V1.size() <= 1) return V1;
	std::vector<int> V = V1;
	int end = V.size();
	if (V[end - 1]<V[0]) V = reverse_order(V);
	std::vector<int> less, greater;
	greater.push_back(V[end - 1]);
	for (int i = 0; i<end - 1; i++)
		if (V[i]<V[end - 1]) less.push_back(V[i]);
		else greater.push_back(V[i]);


		std::vector<int> res = QSort(less);
		if ((V == greater) && (less.size() == 0)) return greater;
		std::vector<int> x2 = QSort(greater);

		res.insert(res.end(), x2.begin(), x2.end());
		less.clear();
		greater.clear();
		x2.clear();
		return res;

}

std::vector<int> QbSort(const std::vector<int> &V1)
{
	if (V1.size() < 100) return bubbleSort(V1);
	if (V1.size() <= 1) return V1;
	std::vector<int> V = V1;
	int end = V.size();
	if (V[end - 1]<V[0]) V = reverse_order(V);
	std::vector<int> less, greater;
	greater.push_back(V[end - 1]);
	for (int i = 0; i<end - 1; i++)
		if (V[i]<V[end - 1]) less.push_back(V[i]);
		else greater.push_back(V[i]);


		std::vector<int> res = QSort(less);
		if ((V == greater) && (less.size() == 0)) return greater;
		std::vector<int> x2 = QSort(greater);

		res.insert(res.end(), x2.begin(), x2.end());
		less.clear();
		greater.clear();
		x2.clear();
		return res;

}

std::vector<double> reverse_order(const std::vector<double> &V)
{
	std::vector<double> A;
	for (int i = V.size() - 1; i >= 0; i--)
		A.push_back(V[i]);

	return A;
}

std::vector<int> reverse_order(const std::vector<int> &V)
{
	std::vector<int> A;
	for (int i = V.size() - 1; i >= 0; i--)
		A.push_back(V[i]);

	return A;
}

std::vector<double> bubbleSort(const std::vector<double> &V)
{

	if (V.size() <= 1) return V;
	std::vector<double> A;
	if (V[V.size() - 1] < V[0])
		A = reverse_order(V);
	else
		A = V;

	int n = A.size();

	bool swapped = false;
	do
	{
		swapped = false;
		for (int i = 1; i < n - 1; i++)
		{
			if (A[i - 1] > A[i])
			{
				//temp = A[i - 1];
				//A[i - 1] = A[i];
				//A[i] = temp;
				std::swap(A[i], A[i - 1]);
				swapped = true;
			}
		}
	} while (swapped);
	//clock_t t1 = clock() - t0;
	//float run_time = ((float)t1) / CLOCKS_PER_SEC;
	//qDebug() << "sorting finished in" << run_time << " sec";
	return A;
}
std::vector<int> bubbleSort(const std::vector<int> &V)
{
	if (V.size() <= 1) return V;
	std::vector<int> A;
	if (V[V.size() - 1] < V[0])
		A = reverse_order(V);
	else
		A = V;
	int n = A.size();

	bool swapped = false;
	do
	{
		swapped = false;
		for (int i = 1; i < n - 1; i++)
		{
			if (A[i - 1] > A[i])
			{
				//temp = A[i - 1];
				//A[i - 1] = A[i];
				//A[i] = temp;
				std::swap(A[i], A[i - 1]);
				swapped = true;
			}
		}
	} while (!swapped);
	return A;
}




