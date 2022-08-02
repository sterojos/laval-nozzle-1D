#define NO_FMTLIB 1

#include <cstdio>
#include "CoolProp.h"

int main()
{
	FILE *src = fopen("pTrel.txt", "r");
//	FILE *dest = fopen("ZpT.txt", "w");
	double p, T, Z;
	double Zmin = 500;
	while(fscanf(src, "%lf %lf\n", &p, &T) == 2)
		if((Z = CoolProp::PropsSI("Z", "P", p, "T", T, "Air")) < Zmin) Zmin = Z;
	printf("%lf\n", Zmax);
	return 0;
}
