#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex>
#include <iostream>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <vector>
#include <signal.h>

//Ascii Characters sorted by brightness (pixel density)
std::string asciiList = " .'`^\"i,:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
//Holds all of the strings that will make up the screen
std::vector<std::string> screen;
//Holds all of the screens
std::vector< std::vector< std::string > > screen_buffer;

//Maximum number of frames for the screen buffer
int max_frames = 500;
int lumSameCount;

struct winsize size;
double width;
double height;

double prev_x; 	
double prev_y; 
double prev_scale; 

double MAX_LUM;
double MID_LUM;
double prevLum;

double totalLum;
double scale;

double center_x;
double center_y;

bool animate_scale;
double max_iterations;

//Maps x from terminal coordinates to cartesian 
double map_x(double x){
	return  center_x + ( (4.0/width) * x - 2.0 ) / scale;
}

//Maps y from terminal coordinates to cartesian 
double map_y(double y){
	return center_y + ( 2.0 - (4.0/height) * y ) / scale;
}
//Takes in a luminocity from [0,1] and converts it to the corresponding character with 
//that brightness.
char get_ascii(double x){
	totalLum += x;
	return( asciiList[ ((int)asciiList.length()-1) * x ] );
}

double get_brot(double x, double y){
	std::vector< std::complex<double> > z;
	std::complex<double> c(x,y);
	z.push_back(c);
	double i = 1;
	while(i < max_iterations){
		
		z.push_back(z[i-1]*z[i-1] + c);

		if( (real(z[i])*real(z[i]) + imag(z[i])*imag(z[i])) > 4 ) break;
		i++;
	}
	return i/max_iterations;
}
//Initializes screen with the mandlebrot set centered at 0,0
void init_screen(){
	for(int y = 0; y < height; y++){
		screen.push_back("");
		for(int x = 0; x < width; x++){
			screen[y]+= get_ascii(get_brot(map_x(x),map_y(y)));
		}
	}

}
//Prints the screen to the terminal line by line
void print_screen(){
	for(int i = 0; i < (int)screen.size(); i++){
		std::cout << screen[i] << "\n";
	}
}
//Populates the screen strings with the right characters to
//display the mandlebrot set at x,y with the given scale.
void fill_screen(){
	for(int y = 0; y < height; y++){
		screen[y]="";
		for(int x = 0; x < width; x++){
			screen[y]+= get_ascii(get_brot(map_x(x),map_y(y)));
		}
	}
}

void print_frame(std::vector< std::string > frame){
	for(int y = 0; y < (int)frame.size(); y++){
			std::cout << frame[y] << "\n";
	}
}

void render_frame(){

		std::vector< std::string > frame;
		totalLum = 0;
		for(int y = 0; y < height; y++){
			frame.push_back("");
			for(int x = 0; x < width; x++){
				frame[y]+= get_ascii(get_brot(map_x(x),map_y(y)));
			}
		}
		screen_buffer.push_back(frame);
		return;
}

int main(int argc, char ** argv){
	
    //If x and y were givin by command line, populate center_x, and center_y respectively.
	if(argc > 2){
		sscanf(argv[1],"%lf",&center_x);	
		sscanf(argv[2],"%lf",&center_y);
	}
    //Otherwise prompt user for input.
	else{
		printf("ENTER: \"x y\"\n");
	}
	printf("(%f,%f)\n",center_x,center_y);
    //Get the terminal width and height in characters
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	height = size.ws_row;
	width = size.ws_col;

    //Init values

	max_iterations = 200;

	scale = 1;

	totalLum = 0;

	std::string input;

	init_screen();
	
	MAX_LUM = width*height;
	MID_LUM = MAX_LUM/2;
	
    fill_screen();
	scale = 1;

	lumSameCount = 0;
    //Progressively zooms in until we reach a point which has no detail (calculating the required number of iterations
    //at this point would become too computationally expensive).
	for(int i = 0; i < max_frames; i++){
			render_frame();
			double RLum = abs(MID_LUM - totalLum) / MID_LUM;
			if(RLum == prevLum) lumSameCount++;
			else(lumSameCount = 0);
			scale = 0.3 + scale*1.1;	
			printf("\e[1;1H\e[2J\n(%d/%d) | %.2fx | %.2fitr | %.2flum | %.2fRlum,%drep\n",i,max_frames,scale,max_iterations,totalLum,RLum,lumSameCount);
			prevLum = RLum;
			if(lumSameCount == 4) break;
	}
	//shouldRun = true;
	max_frames = (int)screen_buffer.size();
	while(1){
		for(int i = 0; i < max_frames; i++){
			//if(!shouldRun) return 0;
			print_frame(screen_buffer[i]);
			usleep(16666*6);
		}
		for(int i = max_frames-2; i >=0; i--){
			//if(!shouldRun) return 0;
			print_frame(screen_buffer[i]);
			usleep(16666*6);
		}
	}
	return 0;
}

