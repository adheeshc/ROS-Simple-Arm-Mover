#include "ros/ros.h"
#include "simple_arm/GoToPosition.h"
#include <sensor_msgs/JointState.h>
#include <sensor_msgs/Image.h>

//Define a global vector of joints last position, moving state of arm and client that can request services

std::vector<double> joints_last_position{0,0};
bool moving_state = false;
ros::ServiceClient client;

// Function to call safe move

void move_arm_center(){
	ROS_INFO_STREAM("Moving arm to centre");

	//Request centre joint angles
	simple_arm::GoToPosition srv;
	srv.request.joint_1=1.57;
	srv.request.joint_2=1.57;
	
	//Call safe move service and pass requested joint angles
	if(!client.call(srv)){
		ROS_ERROR("Failed to call service safe_move");
	}
}

// Callback function to continuously execute and read joint angle position

void joint_states_callback(const sensor_msgs::JointState js){
	//Get current joint position
	std::vector<double> joints_current_position = js.position;

	//Define tolerance threshold
	double tolerance = 0.0005;

	//Check if arm is moving
	if (fabs(joints_current_position[0]-joints_last_position[0])<tolerance && fabs(joints_current_position[0]-joints_last_position[0])<tolerance)
		moving_state=false;
	else{
		moving_state=true;
		joints_last_position=joints_current_position;
	}
}

// Callback function to continuously execute and read image data

void look_away_callback(const sensor_msgs::Image img){
	bool uniform_image = true;

	//Loop through each pixel and check if its equal to the first one
	for (int i = 0;i<img.height*img.step;i++){
		if (img.data[i] - img.data[0] != 0){
			uniform_image=false;
			break;
		}
	}

	// If image is uniform and arm is not moving, move arm to centre
	if (uniform_image == true && moving_state == false)
		move_arm_center();
}

int main(int argc,char** argv){
	//Initialize the look away node and create a handle
	ros::init(argc,argv,"look_away");
	ros::NodeHandle n;

	//Define client service to request services from safe move
	client = n.serviceClient<simple_arm::GoToPosition>("/arm_mover/safe_move");

	//Subscribe to joint states topic to read arm joints position in joints_state_callback 
	ros::Subscriber sub1=n.subscribe("/simple_arm/joint_states",10,joint_states_callback);

	//Subscribe to the rgb camera to read image data in look_away_callback 
	ros::Subscriber sub2=n.subscribe("rgb_camera/image_raw",10,look_away_callback);

	//Handle ros communication events
	ros::spin();

	return 0;
}



