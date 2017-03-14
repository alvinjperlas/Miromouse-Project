/*
 * debugginTools.h
 *
 * Created: 4/18/2013 7:00:57 PM
 *  Author: Administrator
 */


#ifndef MOUSEHEADER_H_
#define MOUSEHEADER_H_

// sensors
#define LEFT_DIGITAL IO_D2
#define RIGHT_DIGITAL IO_D3
#define FRONT_DIGITAL IO_D0
#define num_sensors 4

//directions
enum compass_direction
{
	east=0,south,west,north	
};
enum robot_direction
{
	front=0,right,back,left
};
#define xDim 16
#define yDim 16

//Motors		~5% boost to m1
#define motor_1_boost 1.05
#define catchup_ratio 2

//environment
#define nearby_walls_threshold 400
#define untilt_threshold 10
#define untilt_speed_drop .5
#define offset_threshold 30
#define offset_speed_drop .5
#define single_wall_offset_threshold 40
#define single_wall_untilt_speed_drop .5
#define single_wall_offset_speed_drop .5


#define TRUE 1
#define FALSE 0

const char smile[] PROGMEM = {
	0b00000,
	0b01010,
	0b01010,
	0b01010,
	0b00000,
	0b10001,
	0b01110,
	0b00000
};
const char frown[] PROGMEM = {
	0b00000,
	0b01010,
	0b01010,
	0b01010,
	0b00000,
	0b01110,
	0b10001,
	0b00000
};
const char backslash[] PROGMEM = {
	0b00000,
	0b10000,
	0b01000,
	0b00100,
	0b00010,
	0b00001,
	0b00000,
	0b00000
};

int currentPositionX = 0;		// starts at (0,0) grid
int currentPositionY = 0;
int currentDirection = north;
int x,y;//bottom left is 0, increases going up and right
int dir;//dir is 0-3 where it represents dir*pi/2
int storage=0;		// for use with think()

union
{
	unsigned int all_data;
	struct
	{
		unsigned short best_path	: 8;
		unsigned short best_from	: 2;//direction
		unsigned short worst_path	: 8;
		unsigned short worst_from	: 2;//direction
		unsigned short seen			: 4;//whether each wall is seen
		unsigned short wall			: 4;//whether each is empty or a wall
		unsigned short visited		: 1;//whether it is visited
		//unsigned short garbage		: 3;//no use at the moment, will implement more later?
	};
} maze_map[xDim][yDim];


int think();
void blink(int delay_amount);
void move(int move_choice);
void use_answers();
void PrintCoordinates();
void create_map();
void updateCoordinate(int currDir);
void remember();
int sensor_to_long_mm(int sensor_value);
int sensor_to_short_mm(int sensor_value);
inline int sensor_read(int sensor_choice);
void print_all_sensors(int mode);
void move_motors_straight(int distance,int speed, int turn,int nudge);
void move_forward_recenter(int distance,int speed);
void move_forward_1();
void turn_right();
void turn_left();
void turn_180();
void flood_fill_best(int,int,int);
void flood_fill_worst(int,int,int);



#endif /*MOUSEHEADER_H_ */