/**
 * gcc -o main main.c
 * ./main
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h> 
#include <unistd.h>
#define training_min 20
#define alpha 0.15

typedef enum { false, true } bool;
typedef enum { MEAN, ZMEAN, RANGE, EMA } Mode;

Mode mode = ZMEAN;

typedef struct
{
    time_t time;
    float y_axis;
    long int x_axis;
} Point;

float CD = 0;
int count = 0;
bool trained = false;
int n_z = training_min;
long int first_time_value = 0;
float s_u, s_l;
float s_u_max, max = -INFINITY; float s_l_min, min = INFINITY;
Point c, d, u, l;

Point generate_point(){
    Point newTuple;
    srand(time(NULL));
    newTuple.y_axis = (rand() % 33) + 27;
    newTuple.time = time(NULL);
    newTuple.x_axis = newTuple.time - first_time_value;
    printf("Uncompressed value: %.2f \n", newTuple.y_axis);
    return newTuple;
}

void new_window(){
    u.x_axis = c.x_axis; u.y_axis = c.y_axis + CD;
    l.x_axis = c.x_axis; l.y_axis = c.y_axis - CD;
    s_u = (d.y_axis - u.y_axis)/(d.x_axis - u.x_axis);
    s_l = (d.y_axis - l.y_axis)/(d.x_axis - l.x_axis);
    s_u_max = s_u;
    s_l_min = s_l;
}

void broadcast(Point c){
    printf("y: %.2f | x: %ld | time: %ld \n", c.y_axis, c.x_axis, c.time);
}

void SDT(long int x_axis, float y_axis){
    Point p = d;
    d.x_axis = x_axis; d.y_axis = y_axis;
    s_u = (d.y_axis - u.y_axis)/(d.x_axis - u.x_axis);
    s_l = (d.y_axis - l.y_axis)/(d.x_axis - l.x_axis);
    if(s_u > s_u_max){
        s_u_max = s_u;
        if(s_u_max > s_l_min){
            float s_o = (d.y_axis - p.y_axis)/(d.x_axis - p.x_axis);
            float c_u = (u.y_axis - p.y_axis + (s_o * p.x_axis) - (s_l_min * u.x_axis))/(s_o - s_l_min);
            c.x_axis = c_u;
            c.y_axis = u.y_axis + (s_l_min * (c_u - u.x_axis) - CD/2);
            broadcast(c);
            new_window();
        }
    }
    if(s_l < s_l_min){
        s_l_min = s_l;
        if(s_u_max > s_l_min){
            float s_o = (d.y_axis - p.y_axis)/(d.x_axis - p.x_axis);
            float c_l = (l.y_axis - p.y_axis + (s_o * p.x_axis) - (s_u_max * l.x_axis))/(s_o - s_u_max);
            c.x_axis = c_l;
            c.y_axis = l.y_axis + (s_u_max * (c_l - l.x_axis) + CD/2);
            broadcast(c);
            new_window();
        }
    }
    
}

float mean(Point d, Point p){
    return (d.y_axis + p.y_axis)/2;
}

float zmean(Point d, Point p){
    float s_i = abs((d.y_axis - p.y_axis)/(d.x_axis - p.x_axis));
    if(s_i != 0){
        return CD + (s_i/n_z);
    }
    return (CD * (n_z + 1))/n_z;
}

float range(Point d, Point p){
    return (max - min)/2;
}

float ema(Point d, Point p){
    float s_i = abs((d.y_axis - p.y_axis)/(d.x_axis - p.x_axis));
    return (1.0 - alpha) * CD + (alpha * s_i);
}

void train(long int x_axis, int y_axis){
    Point p = d;
    d.x_axis = x_axis; d.y_axis = y_axis;
    broadcast(d);
    switch (mode)
    {
    case MEAN:
        CD = mean(d, p);
        break;
    case ZMEAN:
        CD = zmean(d, p);
        break;
    case RANGE:
        CD = range(d, p);
        break;
    case EMA:
        CD = ema(d, p);
        break;
    default:
        break;
    }
    if(++count == training_min){
        c = d;
        s_u_max = -INFINITY;
        s_l_min = INFINITY;
        trained = true;
    }
}

void SSDT(long int x_axis, int y_axis){
    if(trained == false){
        train(x_axis, y_axis);
    }else{
        SDT(x_axis, y_axis);
    }
}

int main(int argc, char const *argv[])
{
    /** Initialization */
    
    c = generate_point(); c.x_axis = 0; d = c; d.x_axis = 0;
    first_time_value = c.time;
    u.x_axis = 0 + c.x_axis; u.y_axis = c.y_axis + CD;
    l.x_axis = 0 + c.x_axis; l.y_axis = c.y_axis - CD;
    broadcast(c);

    while (1)
    {
        sleep(1);
        Point p = generate_point();
        SSDT(p.x_axis, p.y_axis);
    }
    
    return 0;
}
