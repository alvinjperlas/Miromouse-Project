/* main - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 1/14/2013 11:31:18 AM
 *  Author: Timothy Chan, class of 2013, EE/CSE double major
 Co-author: Alvin Perlas, class of 2013, CSE, aperlas@uci.edu
 
 Project Lead: Irvin Huang, EE, irvinbhuang@gmail.com
 */


#include <pololu/orangutan.h>
#include "MouseHeader.h"


//==============================================================================
//=================									 			================
//=================					AI							================
//================												================
//==============================================================================

/*use map and position and direction to figure out next move*/
int think()
{
	/*	0 = forward
		1 = right
	   -1 = left
		2 = U turn
	*/
	const int front_wall= is_digital_input_high(FRONT_DIGITAL)?0:1;
	const int right1	= sensor_read(2);
	const int right2	= sensor_read(3);
	const int right_wall= (is_digital_input_high(RIGHT_DIGITAL)?0:1)||(right1>nearby_walls_threshold&&right2>nearby_walls_threshold);

	if(right_wall){
		if(front_wall)
			return -1;
		else								
			return 0;					
	}else{
		if(storage)
		{
			storage=0;
			return 0;
		}			
		else
		{
			storage=1;
			return 1;
		}
	}
}

void blink(int delay_amount)
{
	red_led(1);     // Turn on the red LED.
	delay_ms(delay_amount/2);
	red_led(0);     // Turn off the red LED.
	delay_ms(delay_amount/2);
	clear();
}
//0 is forward 1 is right, -1 is left, 2 is 180
void move(int move_choice)
{
	switch(move_choice)
	{
		case 0:						
			move_forward_1();
			break;
		case 1:
			turn_right();
			break;
		case -1:
			turn_left();
			break;
		case 2:
			turn_180();
			break;
	}
}

void use_answers()
{
	const int solutions[]={0,0,1,0,0,1,0,0,1,0,1,0,2,0,-1,0,0,0,0,-1,0,-1,0,0,1,0,1,0,0,2,0};
	const int solution_length=31;
	for(int i=0;i<solution_length;i++)
	{
		clear();
		print_all_sensors(2);
		//update_map();
		blink(200);
		move(solutions[i]);
	}

	while(1)
	{
		blink(300);
		move(0);
		blink(300);
		move(-1);
	}
}



//==============================================================================
//=================												================
//=================					Maps						================
//================												================
//==============================================================================




int orientation(int facing,int direction){//0=front,1=right,2=back,3=left
	return (facing+direction)%4;
}
int x_offset(int facing){
	if(facing==east)
		return 1;
	if(facing==west)
		return -1;
	return 0;
}
int y_offset(int facing){
	if(facing==north)
		return 1;
	if(facing==south)
		return -1;
	return 0;
}
int in_maze(int x,int y)
{
	return (x<xDim&&x>=0) || (y<yDim&&y>=0);
}
void print_coordinates()
{
	clear();
	lcd_init_printf();
	switch(currentDirection)
	{
		case north:
		print("N:(");
		break;
		case east:
		print("E:(");
		break;
		case south:
		print("S:(");
		break;
		case west:
		print("W:(");
		break;
	}
	print_long(currentPositionX);
	print(",");
	print_long(currentPositionY);
	print(")");
}

void init_map()
{
	for(int i=0;i<xDim;i++){
		for(int j=0;j<yDim;j++){
			//Initialize each grid of the map to be zero all.
			maze_map[i][j].all_data=0;
			maze_map[i][j].best_path=0xFF;
			maze_map[i][j].worst_path=0xFF;
		}
	}
	maze_map[0][0].wall|=(1<<south);
	maze_map[0][0].seen|=(1<<south);
	maze_map[0][0].visited=1;
	maze_map[0][0].best_path=0;
	maze_map[0][0].worst_path=0;

}

void update_pos()
{
	currentPositionX+=x_offset(currentDirection);
	currentPositionY+=y_offset(currentDirection);
}

void update_map()
{
	const int left1		= sensor_read(0);
	const int left2		= sensor_read(1);
	const int right1	= sensor_read(2);
	const int right2	= sensor_read(3);

	// coordinates in the mouses perception.
	int front_wall= is_digital_input_high(FRONT_DIGITAL)?0:1;
	int left_wall = (is_digital_input_high(LEFT_DIGITAL)?0:1)||(left1>nearby_walls_threshold&&left2>nearby_walls_threshold);
	int right_wall = (is_digital_input_high(RIGHT_DIGITAL)?0:1)||(right1>nearby_walls_threshold&&right2>nearby_walls_threshold);
	const int front_direction=orientation(currentDirection,front);
	const int left_direction=orientation(currentDirection,left);
	const int right_direction=orientation(currentDirection,right);
	const int back_direction=orientation(currentDirection,back);
	maze_map[currentPositionX][currentPositionY].wall |= (front_wall<<front_direction);
	maze_map[currentPositionX][currentPositionY].wall |= (left_wall<<left_direction);
	maze_map[currentPositionX][currentPositionY].wall |= (right_wall<<right_direction);
	maze_map[currentPositionX][currentPositionY].seen |= (1<<front_direction);
	maze_map[currentPositionX][currentPositionY].seen |= (1<<left_direction);
	maze_map[currentPositionX][currentPositionY].seen |= (1<<right_direction);
	
	int tempPosX=currentPositionX+x_offset(front_direction);
	int tempPosY=currentPositionY+y_offset(front_direction);
	if(in_maze(tempPosX,tempPosY)){
		maze_map[tempPosX][tempPosY].wall |=(front_wall<<back_direction);
		maze_map[tempPosX][tempPosY].seen |=(1<<back_direction);		
		if(!front_wall)
		{
			flood_fill_best(tempPosX,tempPosY,maze_map[currentPositionX][currentPositionY].best_path+1);
			flood_fill_worst(tempPosX,tempPosY,maze_map[currentPositionX][currentPositionY].worst_path+1);
		}
	}
	tempPosX=currentPositionX+x_offset(left_direction);
	tempPosY=currentPositionY+y_offset(left_direction);
	if(in_maze(tempPosX,tempPosY)){
		maze_map[tempPosX][tempPosY].wall |=(left_wall<<right_direction);
		maze_map[tempPosX][tempPosY].seen |=(1<<right_direction);
		if(!left_wall)
		{
			flood_fill_best(tempPosX,tempPosY,maze_map[currentPositionX][currentPositionY].best_path+1);
			flood_fill_worst(tempPosX,tempPosY,maze_map[currentPositionX][currentPositionY].worst_path+1);
		}
	}
	tempPosX=currentPositionX+x_offset(right_direction);
	tempPosY=currentPositionY+y_offset(right_direction);
	if(in_maze(tempPosX,tempPosY)){
		maze_map[tempPosX][tempPosY].wall |=(right_wall<<left_direction);
		maze_map[tempPosX][tempPosY].seen |=(1<<left_direction);
		if(!right_wall)
		{
			flood_fill_best(tempPosX,tempPosY,maze_map[currentPositionX][currentPositionY].best_path+1);
			flood_fill_worst(tempPosX,tempPosY,maze_map[currentPositionX][currentPositionY].worst_path+1);
		}
	}
	
	maze_map[currentPositionX][currentPositionY].visited=1;
	
}
void flood_fill_best(int x,int y,int distance){
	maze_map[x][y].best_path=distance;
	short wall_status=~maze_map[x][y].seen|~maze_map[x][y].wall;
	for(int dir=0;dir<4;dir++){
		if((wall_status&1<<dir)&&maze_map[x+x_offset(dir)][y+y_offset(dir)].best_path>distance+1){
			flood_fill_best(x+x_offset(dir),y+y_offset(dir),distance+1);
		}
	}
}
void flood_fill_worst(int x,int y,int distance){
	maze_map[x][y].worst_path=distance;
	short wall_status=maze_map[x][y].seen&~maze_map[x][y].wall;
	for(int dir=0;dir<4;dir++){
		if((wall_status&1<<dir)&&maze_map[x+x_offset(dir)][y+y_offset(dir)].worst_path>distance+1){
			flood_fill_worst(x+x_offset(dir),y+y_offset(dir),distance+1);
		}
	}
}

//==============================================================================
//=================												================
//=================					Sensors						================
//================												================
//==============================================================================
						// 0   1   2   3
const char sensor_name[]={'L','l','R','r'};

const int sensors[]={6,5,7,4};

const int lookup[30][2]={{0,0},		{10,200},	{20,390},	{30,596},	{40,613},
						 {45,637},	{50,662},	{65,654},	{70,630},	{75,609},
						 {80,590},	{90,513},	{100,470},	{120,400},	{140,330},
						 {160,297},	{180,271},	{200,252},	{250,202},	{300,173},
						 {350,153},	{400,145},	{450,136},	{500,130},	{550,121},
						 {600,117},	{650,107},	{700,106},	{750,105},	{800,104}};
//const unsigned short front_right_close_lookup
int sensor_to_long_mm(int sensor_value)
{
	if(sensor_value<104)
	return 999;
	else
	{
		int dist;
		for(dist=7;dist<30-1&&sensor_value<lookup[dist][1];dist++);
		return lookup[dist-1][0]+(lookup[dist][0]-lookup[dist-1][0])*(lookup[dist-1][1]-sensor_value)/(lookup[dist-1][1]-lookup[dist][1]);
	}
}

int sensor_to_short_mm(int sensor_value)
{
	if(sensor_value>662)
		return (sensor_value-662)/4+50;
	int dist;
	for(dist=6;dist>0&&sensor_value<lookup[dist][1];dist--);
	return lookup[dist+1][0]+(lookup[dist][0]-lookup[dist+1][0])*(lookup[dist+1][1]-sensor_value)/(lookup[dist+1][1]-lookup[dist][1]);
}

inline int sensor_read(int sensor_choice)
{
		int return_value=analog_read_average(sensors[sensor_choice],20);
	return return_value;
}
//mode anything else=sensor values, 1=short, 2=long
void print_all_sensors(int mode)
{
	for(int current_sensor=0;current_sensor<num_sensors;current_sensor++)
	{
		//sensor test
		lcd_goto_xy((current_sensor/2)*5,current_sensor%2);
		print_character(sensor_name[current_sensor]);
		print(":");
		int temp_value=sensor_read(current_sensor);
		if(mode==1)
			temp_value=sensor_to_short_mm(temp_value);
		else if(mode==2)
			temp_value=sensor_to_long_mm(temp_value);
		print_long(temp_value);
		while(temp_value<100)
		{
			print(" ");
			temp_value*=10;
		}
	}
	lcd_goto_xy(10,1);
	print_character('F');
	print_long((is_digital_input_high(FRONT_DIGITAL))?0:1);
	print_character('L');
	print_long((is_digital_input_high(LEFT_DIGITAL))?0:1);
	print_character('R');
	print_long((is_digital_input_high(RIGHT_DIGITAL))?0:1);
}
//left is -1, right is 1, forward is 0

//==============================================================================
//=================												================
//=================					MOTORS						================
//================												================
//==============================================================================


void move_motors_straight(int distance,int speed, int turn,int nudge)
{
	encoders_get_counts_and_reset_m1();
	encoders_get_counts_and_reset_m2();
	int counts1=0;
	int counts2=0;
	set_m1_speed((int)(speed*(turn==1?1:-1)*motor_1_boost));
	set_m2_speed(speed*(turn==-1?-1:1));
	while((counts1<distance||counts2<distance))
	{
		lcd_goto_xy(0,1);
		print("M1:");
		print_long(counts1);
		print("  ");
		lcd_goto_xy(8,1);
		print("M2:");
		print_long(counts2);
		print("  ");
		if(counts1>counts2){
			set_m1_speed((int)(speed*(turn==1?1:-1)*motor_1_boost));
			set_m2_speed((speed+(counts1-counts2)*catchup_ratio)*(turn==-1?-1:1));
		}
		else if(counts2>counts1){
			set_m1_speed((int)((speed+(counts2-counts1)*catchup_ratio)*(turn==1?1:-1)*motor_1_boost));
			set_m2_speed(speed*(turn==-1?-1:1));
		}
		else{
			set_m1_speed((int)(speed*(turn==1?1:-1)*motor_1_boost));
			set_m2_speed(speed*(turn==-1?-1:1));
		}
		counts1=encoders_get_counts_m1()*(turn==1?1:-1);
		counts2=encoders_get_counts_m2()*(turn==-1?-1:1);
	}
	set_m1_speed((int)(speed*(turn==1?1:-1)*motor_1_boost));
	set_m2_speed(speed*(turn==-1?-1:1));
	delay_ms(nudge);
	set_m1_speed(0);
	set_m2_speed(0);
	lcd_goto_xy(0,1);
	print("M1:");
	print_long(encoders_get_counts_m1()*(turn==1?1:-1));
	print("  ");
	lcd_goto_xy(8,1);
	print("M2:");
	print_long(encoders_get_counts_m2()*(turn==-1?-1:1));
	print("  ");
}
void move_forward_recenter(int distance,int speed)
{
	encoders_get_counts_and_reset_m1();
	encoders_get_counts_and_reset_m2();
	int counts1=0;
	int counts2=0;
	int dont_hit_wall=0;
	int i=0;
	set_m1_speed(-(int)(speed*motor_1_boost));
	set_m2_speed(speed);
	while((counts1<distance-dont_hit_wall||counts2<distance-dont_hit_wall))
	{
		//while more to move
		if(i++%50==0)
		{
			print_all_sensors(0);
		}
		const int left1=sensor_read(0);
		const int left2=sensor_read(1);
		const int right1=sensor_read(2);
		const int right2=sensor_read(3);
		const int leftfront=sensor_to_short_mm(left1);
		const int leftback=sensor_to_short_mm(left2);;
		const int rightfront=sensor_to_short_mm(right1);;
		const int rightback=sensor_to_short_mm(right2);;
		const int left_dgtl = is_digital_input_high(LEFT_DIGITAL)?0:1;
		const int right_dgtl = is_digital_input_high(RIGHT_DIGITAL)?0:1;
		const int digitals=right_dgtl-left_dgtl;
		//5 cases, tilted left, tilted right, offset left, offset right, or good for both 2 walls and 1 wall
		if((left1>nearby_walls_threshold&&left2>nearby_walls_threshold)&&(right1>nearby_walls_threshold&&right2>nearby_walls_threshold)){
			//if 2 walls
			if(digitals==1||(leftfront+leftback)-(rightfront+rightback)>offset_threshold){
				//too close to right wall
				//print "| |?||"
				lcd_goto_xy(10,0);
				print("| |");
				print_character(1);
				print("||");
				set_m1_speed(-speed*motor_1_boost);
				set_m2_speed(speed*offset_speed_drop);
			}else if(digitals==-1||(rightfront+rightback)-(leftfront+leftback)>offset_threshold){
				//too close to left wall
				//print "||?| |"
				lcd_goto_xy(10,0);
				print("||");
				print_character(1);
				print("| |");
				set_m1_speed(-speed*motor_1_boost*offset_speed_drop);
				set_m2_speed(speed);
			}else if(rightback+leftfront-rightfront-leftback>untilt_threshold){
				//tilted right
				//print "| /?/|"
				lcd_goto_xy(10,0);
				print("| /");
				print_character(1);
				print("/|");
				set_m1_speed(-speed*motor_1_boost);
				set_m2_speed(speed*untilt_speed_drop);
			}else if(rightfront+leftback-rightback-leftfront>untilt_threshold){
				//tilted left
				//print "|\?\ |"
				lcd_goto_xy(10,0);
				print("|");
				print_character(3);
				print_character(1);
				print_character(3);
				print(" |");
				set_m1_speed(-speed*motor_1_boost*untilt_speed_drop);
				set_m2_speed(speed);
			}else{
				//good and 2 walls
				//print "| :D |"
				lcd_goto_xy(10,0);
				print("| ");
				print_character(2);
				print_character(2);
				print(" |");
				set_m1_speed(-(int)(speed*motor_1_boost));
				set_m2_speed(speed);
			}
		}else{
			//else at least one wall missing wall configuration
			if(left_dgtl){
				//dangerously close to left
				//print "||?|  "
				lcd_goto_xy(10,0);
				print("||");
				print_character(1);
				print("|  ");
				set_m1_speed(-speed*motor_1_boost*untilt_speed_drop);
				set_m2_speed(speed);
			}
			else if(right_dgtl){
				//dangerously close to right
				//print "  |?||"
				lcd_goto_xy(10,0);
				print("  |");
				print_character(1);
				print("||");
				set_m1_speed(-speed*motor_1_boost);
				set_m2_speed(speed*untilt_speed_drop);
			}else if((left1>nearby_walls_threshold&&left2>nearby_walls_threshold)){
				//only left wall
				if(leftback-leftfront>untilt_threshold/2){
					//tilted left
					//print "|\?\  "
					lcd_goto_xy(10,0);
					print("|");
					print_character(3);
					print_character(1);
					print_character(3);
					print("  ");
					set_m1_speed(-speed*motor_1_boost*single_wall_untilt_speed_drop);
					set_m2_speed(speed);
				}else if(leftfront-leftback>untilt_threshold/2){
					//tilted right
					//print "| /?/ "
					lcd_goto_xy(10,0);
					print("| /");
					print_character(1);
					print("/ ");
					set_m1_speed(-speed*motor_1_boost);
					set_m2_speed(speed*single_wall_untilt_speed_drop);
				}else if(leftfront+leftback>single_wall_offset_threshold){
					//offset right
					lcd_goto_xy(10,0);
					print("| |");
					print_character(1);
					print("| ");
					set_m1_speed(-speed*motor_1_boost);
					set_m2_speed(speed*single_wall_offset_speed_drop);
				}else{
					//unknown walls, assume good
					//print "| ??  "
					lcd_goto_xy(10,0);
					print("| ");
					print_character(2);
					print_character(2);
					print("  ");
					set_m1_speed(-(int)(speed*motor_1_boost));
					set_m2_speed(speed);
				}
			}else if((right1>nearby_walls_threshold&&right2>nearby_walls_threshold)){
				//only right wall
				if(rightfront-rightback>untilt_threshold/2){
					//tilted left
					//print " \?\ |"
					lcd_goto_xy(10,0);
					print(" ");
					print_character(3);
					print_character(1);
					print_character(3);
					print(" |");
					set_m1_speed(-speed*motor_1_boost*single_wall_untilt_speed_drop);
					set_m2_speed(speed);
				}else if(rightback-rightfront>untilt_threshold/2){
					//tilted right
					//print "  /?/|"
					lcd_goto_xy(10,0);
					print("  /");
					print_character(1);
					print("/|");
					set_m1_speed(-speed*motor_1_boost);
					set_m2_speed(speed*single_wall_untilt_speed_drop);
				}else if(rightfront+rightback>single_wall_offset_threshold){
					//offset left
					lcd_goto_xy(10,0);
					print(" |");
					print_character(1);
					print("| |");
					set_m1_speed(-speed*motor_1_boost*single_wall_offset_speed_drop);
					set_m2_speed(speed);
				}else{
					//unknown walls, assume good
					//print "  ?? |"
					lcd_goto_xy(10,0);
					print("  ");
					print_character(2);
					print_character(2);
					print(" |");
					set_m1_speed(-(int)(speed*motor_1_boost));
					set_m2_speed(speed);
				}
			}else{
				//unknown walls, assume good
				//print "  ??  "
				lcd_goto_xy(10,0);
				print("  ");
				print_character(2);
				print_character(2);
				print("  ");
				set_m1_speed(-(int)(speed*motor_1_boost));
				set_m2_speed(speed);
			}
		}
		counts1=-encoders_get_counts_m1();
		counts2=encoders_get_counts_m2();
		if(dont_hit_wall==0 && (is_digital_input_high(FRONT_DIGITAL)?0:1))
		{
			dont_hit_wall=distance-(counts1>counts2?counts1:counts2)-distance/20;
			dont_hit_wall=((dont_hit_wall>0)?dont_hit_wall:0);
		}
	}
	set_m1_speed(0);
	set_m2_speed(0);
}

void move_forward_1()
{
	//print("Move Forward 1");
	lcd_goto_xy(0,0);
	//distance 32, speed 30, no turn
	move_forward_recenter(31,60);
	update_pos();
	print_coordinates();
}

void turn_right()
{
	//Prints the turn Right to LCD
	lcd_goto_xy(0,0);
	print("Turn Right");
	currentDirection = (currentDirection+1)%4;	// updates the current direction the mouse is facing when it turns right.
	move_motors_straight(12,30,1,40);			//distance 12, speed 30, turn right
}

void turn_left()
{
	//Prints the turn left to LCD
	lcd_goto_xy(0,0);
	print("Turn Left");
	currentDirection = (currentDirection+3)%4;	// updates the current direction the mouse is facing when it turns left.
	move_motors_straight(12,30,-1,47);			//distance 12, speed 30, turn left
}

void turn_180()
{
	//Prints the turn 180 to LCD
	lcd_goto_xy(0,0);
	print("Turn 180");
	currentDirection = (currentDirection+2)%4;	// updates the current direction the mouse is facing when it turns right.
	move_motors_straight(24,30,1,95);			//distance 24, speed 30, turn right
}

//==============================================================================
//=================												================
//=================					Driver						================
//================												================
//==============================================================================

int main()
{
	
	play_from_program_space(PSTR(">g32>>c32"));  // Play welcoming notes.
	
	lcd_load_custom_character(frown, 1);
	lcd_load_custom_character(smile, 2);
	lcd_load_custom_character(backslash, 3);
	encoders_init(IO_B3, IO_B4,IO_C1, IO_C0); // assigning the pins for encoders
	set_analog_mode(MODE_10_BIT);
	set_digital_input(FRONT_DIGITAL,PULL_UP_ENABLED);
	set_digital_input(LEFT_DIGITAL,PULL_UP_ENABLED);
	set_digital_input(RIGHT_DIGITAL,PULL_UP_ENABLED);
	
	init_map();
	update_map();

	
	while(1)
	{
		blink(400);
		move(think());
		update_map();
	}
}

