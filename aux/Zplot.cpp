#define NO_FMTLIB 1

#include <cstdio>
#include "CoolProp.h"

#define P_MIN 100	
#define P_MAX 40000000
#define P_POINT_DENSITY 300

#define T_ISOTHERM 648

#define WATER_FILE_PATH "/home/pepik/bakalarka/tex/Zplot_water.txt"
#define METHANE_FILE_PATH "/home/pepik/bakalarka/tex/Zplot_methane.txt"
#define AIR_FILE_PATH "/home/pepik/bakalarka/tex/Zplot_air.txt"

int main()
{
	std::vector<double> pressure;
	double temper = T_ISOTHERM;

	/* cringe
	std::vector<std::vector<double>> output;
	*/

	double p_increment = ((double) P_MAX - P_MIN) / (P_POINT_DENSITY - 1);
	for(double p = P_MIN; p <= P_MAX; p += p_increment)
	{
		pressure.push_back(p);
	}
	std::vector<double> Z_water;
	std::vector<double> Z_methane;
	std::vector<double> Z_air;

	FILE *output_water = fopen(WATER_FILE_PATH, "w");
	FILE *output_methane = fopen(METHANE_FILE_PATH, "w");
	FILE *output_air = fopen(AIR_FILE_PATH, "w");
	for(double p : pressure)
	{
		Z_water.push_back(CoolProp::PropsSI("Z", "P", p, "T", temper, "Water"));
		Z_methane.push_back(CoolProp::PropsSI("Z", "P", p, "T", temper, "Methane"));
		Z_air.push_back(CoolProp::PropsSI("Z", "P", p, "T", temper, "Methane"));
	}
	printf("Z_water\n");
	for(int i = 0; i < pressure.size(); i++)
	{
		fprintf(output_water, "%lf %lf\n", pressure[i], Z_water[i]);
	}
	printf("Z_methane\n");
	for(int i = 0; i < pressure.size(); i++)
	{
		fprintf(output_methane, "%lf %lf\n", pressure[i], Z_methane[i]);
	}
	printf("Z_air\n");
	for(int i = 0; i < pressure.size(); i++)
	{
		fprintf(output_air, "%lf %lf\n", pressure[i], Z_air[i]);
	}

#if 0
	for(int i = 0; i < pressure.size(); i++)
	{
		printf("%lf %lf %lf %lf\n", pressure[i], Z_water[i], Z_methane[i], Z_air[i]);
	}

	{
		printf("%lf\n", Z_water.back());
		Z_water.pop_back();
	}
	puts("Z_methane");
	for(double Z : Z_methane)
	{
		printf("%lf\n", Z_methane.back());
		Z_methane.pop_back();
	}
	puts("Z_air");
	for(double Z : Z_air)
	{
		printf("%lf\n", Z_air.back());
		Z_air.pop_back();
	}
	for(double p : pressure)
	{
		printf("%lf\n", p);
	}
	std::vector<std::string> outputs;
	outputs.push_back("Z");

	std::vector<std::string> water;
	water.push_back("Water");
	std::vector<double> unity;
	unity.push_back((double) 1);

	std::vector<std::string> methane;
	methane.push_back("Methane");

	std::vector<std::string> air;
	air.push_back("Air");
	output = CoolProp::PropsSImulti(outputs, "P", pressure, "T", temperature, "HEOS", water, unity);
	if(output.empty())
	{
		puts("oops");
	}
	for(auto value : output)
	{
		for(double outval : value)
		{
			printf("%lf\n", outval);
		}
	}
	double test = CoolProp::PropsSI("Hmass", "P", 101325, "T", 300, "Water");
	printf("%lf\n", test);
	puts("hello world");
#endif
	return 0;
}
