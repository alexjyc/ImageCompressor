#include <math.h>
#include "utest.h"
#include "formulas.h"

UTEST(FormulasTest, CalculatePbMaxNegative)
{
    float r = 1.0, g = 1.0, b = 0;
    float result = calculate_pb(r, g, b);
    float gt = -0.5;
    EXPECT_EQ(result, gt);
}

UTEST(FormulasTest, CalculatePbMaxPositive)
{
    float r = 0.0, g = 0.0, b = 1.0;
    float result = calculate_pb(r, g, b);
    float gt = 0.5;
    EXPECT_EQ(result, gt);
}

UTEST(FormulasTest, CalculatePrMaxNegative)
{
    float r = 0.0, g = 1, b = 1;
    float result = calculate_pr(r, g, b);
    float gt = -0.5;
    EXPECT_EQ(result, gt);
}

UTEST(FormulasTest, CalculatePrMaxPositive)
{
    float r = 1.0, g = 0.0, b = 0.0;
    float result = calculate_pr(r, g, b);
    float gt = 0.5;
    EXPECT_EQ(result, gt);
}

UTEST(FormulasTest, AveragePositiveInteger)
{
    float input[] = {1.0, 5.0, 25.0, 40.0};
    float length = sizeof(input) / sizeof(input[0]);
    float avg = average(input, length);
    float gt = 17.75;
    printf("Groundtruth: %f Output: %f\n", gt, avg);
}

UTEST(FormulasTest, AveragePositiveFloat)
{
    float input[] = {0.58, 9.12, 15.13, 35.72};
    float length = sizeof(input) / sizeof(input[0]);
    float avg = average(input, length);
    float gt = 15.1375;
    printf("Groundtruth: %f Output: %f\n", gt, avg);
}

UTEST(FormulasTest, AverageSmallPositiveFloat)
{
    float input[] = {0.005, 0.387, 0.923, 0.999};
    float length = sizeof(input) / sizeof(input[0]);
    float avg = average(input, length);
    float gt = 0.5785;
    printf("Groundtruth: %f Output: %f\n", gt, avg);
}

UTEST(FormulasTest, AverageNegativeInteger)
{
    float input[] = {-1.0, -5.0, -25.0, -40.0};
    float length = sizeof(input) / sizeof(input[0]);
    float avg = average(input, length);
    float gt = -17.75;
    printf("Groundtruth: %f Output: %f\n", gt, avg);
}

UTEST(FormulasTest, AverageNegativeFloat)
{
    float input[] = {-0.387, -5.573, -13.345, -19.22};
    float length = sizeof(input) / sizeof(input[0]);
    float avg = average(input, length);
    float gt = -9.63125;
    printf("Groundtruth: %f Output: %f\n", gt, avg);
}

UTEST(FormulasTest, AverageSmallNegativeFloat)
{
    float input[] = {-0.005, -0.387, -0.923, -0.999};
    float length = sizeof(input) / sizeof(input[0]);
    float avg = average(input, length);
    float gt = -0.5785;
    printf("Groundtruth: %f Output: %f\n", gt, avg);
}