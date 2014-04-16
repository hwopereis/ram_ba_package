#include <ros/ros.h>
#include <std_msgs/Empty.h>
#include <geometry_msgs/Twist.h>
#include <termios.h> // This and down is for the keyboard input
#include <stdio.h>
#include <sys/poll.h>


// 2D!
#define KEY_UP 0x87 // W
#define KEY_LEFT 0x65 // A
#define KEY_DOWN 0x83 // S
#define KEY_RIGHT 0x68 // D
#define KEY_RESET 0x82 // R
#define KEY_LAUNCH 0x81 // O
#define KEY_LAND 0x76 // L

geometry_msgs::Twist twist_msg;
std_msgs::Empty emp_msg;
int speed = 0;
int turn = 0;
double speed_factor = 0.5;
double turn_factor = 0.5;
bool dirty = false; // To make sure to stop the robot after a command is given
int kfd = 0;
struct termios cooked, raw;

int main(int argc, char** argv)
{
	ros::init(argc, argv, "RAM_fly_from_keyboard");
	ros::NodeHandle node;
	ros::Rate loop_rate(50); // Check if fast enough / consistent with launch files
	ros::Publisher pub_twist; // We need to publish some twists
	ros::Publisher pub_empty; // and some empty messages

	pub_twist = node.advertise<geometry_msgs::Twist>("cmd_vel",1); // command velocity
	
	ROS_INFO("Waiting for keyboard input");
	
	char c;
	tcgetattr(kfd, &cooked);
	memcpy(&raw, &cooked, sizeof(struct termios));
	raw.c_lflag &=~ (ICANON | ECHO);
	raw.c_cc[VEOL] = 1;
	raw.c_cc[VEOF] = 2;
	tcsetattr(kfd, TCSANOW, &raw);

	puts("Reading from keyboard.");
	
	struct pollfd ufd;
	ufd.fd = kfd;
	ufd.events = POLLIN;

	for(;;)
	{
		int num;
		if ((num = poll(&ufd, 1, 250)) < 0)
		{
			perror("poll():");
			return;
		}
		else if(num > 0)
		{
			if(read(kfd, &c, 1) < 0)
			{
				perror("read():");
				return;
			}
		}
		else
		{
			if(dirty == true)
			{
				// Stop robot		
				twist_msg.linear.x= 0.0;
				twist_msg.angular.z = 0.0;
				pub_twist.publish(twist_msg);
				dirty = false;
			}

			continue;
		}
		
		switch(c)
		{

			case KEY_LAUNCH:
				dirty = true;
				break;
			case KEY_RESET:
				dirty = true;
				break;
			case KEY_LAND:
				dirty = true;
				break;
			case KEY_UP:
				speed = 1;
				turn = 0;
				dirty = true;
				break;
			case KEY_DOWN:
				speed = -1;
				turn = 0;
				dirty = true;
				break;
			case KEY_LEFT:
				speed = 0;
				turn = 1;
				dirty = true;
				break;
			case KEY_RIGHT;
				speed = 0;
				turn = -1;
				dirty = true;
				break;
			default:
				speed = 0;
				turn = 0;
			
		}

		twist_msg.linear.x = speed*speed_factor;
		//twist_msg.linear.y = key_y;
		twist_msg.angular.z = turn*turn_factor;
		ros::spinOnce(); // ??
		loop_rate.sleep();
	}
	
	
	
ROS_ERROR("ROS:ok() failed - Node Closing");
return(0);
} // main
