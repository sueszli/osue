/** 
 * @file cpair.h
 * @author Andreas Huber 11809629
 * @date 
 *
 * @brief 
 **/


typedef struct point{
    float xCord;
    float yCord;
} point;

typedef struct cpoints{
	point pointA;
	point pointB;
    bool initialized;
} cpoints;

#define MIN(a,b) ((a) < (b) ? a : b) 
#define MAX(a,b) ((a) > (b) ? a : b) 