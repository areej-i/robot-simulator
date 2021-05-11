#define ENV_SIZE          600

#define ROBOT_RADIUS      15   // The radius of the robot
#define MAX_ROBOTS        20   // Maximum number of robots allowed to connect
#define ROBOT_SPEED        3   // Forward pixels per movement
#define ROBOT_TURN_ANGLE  15   // degrees to turn for each robot movement

#define PI                3.14159265359

#define SERVER_IP         "127.0.0.1"
#define SERVER_PORT       6000

// Command codes sent from client to server
#define REGISTER           1
#define STOP               2
#define CHECK_COLLISION    3
#define STATUS_UPDATE      4

// Command codes sent from server to client
#define OK                 5
#define NOT_OK             6
#define NOT_OK_BOUNDARY    7
#define NOT_OK_COLLIDE     8
#define LOST_CONTACT       9

// Robot-related variables
typedef struct {
  float    x;
  float    y;
  int      direction;
} Robot;

typedef struct {
  Robot robots[MAX_ROBOTS];
  int   numRobots;     
  char  shutDown; // 1 if environment has been shut down, 0 otherwise
} Environment;


// These are the procedures being used for display
extern void initializeWindow();
extern void closeWindow();
extern void *redraw(void *);

 
