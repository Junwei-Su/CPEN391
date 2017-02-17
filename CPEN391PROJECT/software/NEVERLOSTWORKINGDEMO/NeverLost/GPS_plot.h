#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "gps.h"


#define PI 3.14159265
#define DATANUMBER 8
#define SCALE_FACTOR 5

struct latlng{
	double lat;
	double lng;
	int flag;
};

struct polor{
    double angle;
    double distance;
};

struct latlng default_loc;
struct latlng GPS_data[DATANUMBER];
int GPS_index;

float scan_angle;
const float speed = PI/8;

char* itoa(int i, char b[]){
    char const digit[] = "0123456789";
    char* p = b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    return b;
}

void gps(){
	clearScreen(BLACK);
	write(50,50, "GPS LOCATION", 16, WHITE, BLACK);
	write(50,100, "Direction: ", 16, WHITE, BLACK);
	write(50,200, "Distance: ", 16, WHITE, BLACK);
	draw_radar(525, 270, 200);

	default_loc.lat = 49.166590;
	default_loc.lng = -123.133569;

	drawLocation(525, 270);
	waitFor(1);
}

/**
 * Converts unit of degree to unit of radians
 */
double toRadians(double degree){
    return degree * M_PI / 180.0;
}

/**
 * Get the heading value between 2 latlng positions
 *
 * return: The angle in radians relative to North of East
 */
double latlng_heading(double lat1, double lon1, double lat2, double lon2){
    // Credits to: http://mathforum.org/library/drmath/view/55417.html

    return fmod(atan2(sin(lon2-lon1)*cos(lat2),
               cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon2-lon1)),
               2*M_PI);
}

/**
 * Get the distance between 2 latlng positions
 *
 * return: The distance in km
 */
double latlng_distance(double lat1, double lon1, double lat2, double lon2){
    double dLat = toRadians(lat2 - lat1);
    double dLon = toRadians(lon2 - lon1);

    double a = sin(dLat / 2) * sin(dLat / 2)
           + cos(toRadians(lat1))
           * cos(toRadians(lat2)) * sin(dLon / 2)
           * sin(dLon / 2);

    double c = 2 * asin(sqrt(a));
    return 6371 * c;
}

void draw_radar(int cx, int cy, int radius){

	//draw the circle
	int i=0;
	int circle_radius = radius;
	int increment = floor(circle_radius/3);

	for(;i<3; i++){
		drawcircle(cx,cy,circle_radius,GREEN);
		circle_radius=circle_radius - increment;
	}

	//draw radial line
	i=0;
	int x_diff = radius * cos(PI/4);
	int y_diff = radius * sin(PI/4);
	DrawHline(cx-radius,cy, cx+radius, cy, GREEN);
	DrawVline(cx,cy-radius, cx, cy+radius, GREEN);
	Drawline(cx+x_diff,cy+y_diff,cx-x_diff,cy-y_diff, GREEN);
	Drawline(cx-x_diff,cy+y_diff,cx+x_diff,cy-y_diff, GREEN);

	//draw red dot center
	drawSolidcircle(cx,cy,10,RED);

	scan_line(cx,cy,radius);

}

void scan_line(int cx, int cy,int radius){
	double period = 2* 3.142;
	if(scan_angle>= 2*PI){
		scan_angle=0;
	}else{
		scan_angle+=speed;
	}

	int x_diff = radius * cos(scan_angle);
	int y_diff = radius * sin(scan_angle);
	Drawline(cx,cy,cx+x_diff,cy+y_diff, RED);
}

void init_gps(){

	GPS_index = 0;
	scan_angle = 0;

	int i;
	for(i=0; i<DATANUMBER; i++){
		GPS_data[i].lat=0;
		GPS_data[i].lng=0;
		GPS_data[i].flag=0;
	}
}

struct polor *computeLoc(struct latlng loc1, struct latlng loc2){
	double distance = latlng_distance(loc1.lat, loc1.lng, loc2.lat, loc2.lng);
	double heading = latlng_heading(loc1.lat, loc1.lng, loc2.lat, loc2.lng);

	struct polor *result;
	result = malloc(sizeof(struct polor));

	result->distance = distance;
	result->angle = heading;

	return result;
}


void drawLocation(int cx, int cy){


	Coord current = getGPSdata();
	char *s = current.lat;
	char *r = current.lng;
   	struct latlng dependant_gps;
   	dependant_gps.lat = atof(s);
   	dependant_gps.lng = atof(r);

	struct polor *info = computeLoc(default_loc,dependant_gps);
	int x_diff = info->distance * cos(info->angle) * SCALE_FACTOR;
	int y_diff = info->distance * sin(info->angle) * SCALE_FACTOR;

	// Print the angle and distance on the touch screen
	char value[30];

	snprintf(value, 20, "%lf", info->angle);
	write(100,140, value, 16, WHITE, BLACK);

	snprintf(value, 20, "%lf", info->distance);
	write(100,240, value, 16, WHITE, BLACK);


	drawSolidcircle(cx+x_diff,cy+y_diff,10,BLUE);

}

void gpsB(){

	drawSolidcircle(120, 350, 80, BLUE);
	write(95, 330, "GET", 16, WHITE, BLUE);
	write(95, 360, "GPS", 16, WHITE, BLUE);


}