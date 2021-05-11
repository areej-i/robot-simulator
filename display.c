#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <pthread.h>
#include <math.h>

#include "simulator.h"

// Display-related variables
Display *display;
Window   win;
GC       gc;


// Initialize and open the simulator window with size WIN_SIZE x WIN_SIZE.
void initializeWindow() {
  // Open connection to X server
  display = XOpenDisplay(NULL);

  // Create a simple window, set the title and get the graphics context then
  // make is visible and get ready to draw
  win = XCreateSimpleWindow(display,  RootWindow(display, 0), 0, 0,
			    ENV_SIZE, ENV_SIZE, 0, 0x000000, 0xFFFFFF);
  XStoreName(display, win, "Robot Simulator");
  gc = XCreateGC(display, win, 0, NULL);
  XMapWindow(display, win);
  XFlush(display);
  usleep(20000);  // sleep for 20 milliseconds.
}

// Close the display window
void closeWindow() {
  XFreeGC(display, gc);
  XUnmapWindow(display, win);
  XDestroyWindow(display, win);
  XCloseDisplay(display);
}


// Redraw all the environment.  This includes all robots that belong to the
// environment.  This code should run in an infinite loop continuoously
// drawing the environment.   Robot's are drawn as black circles with radius
// ROBOT_RADIUS and a white line from their center to their radius to indicate
// the direction that the robot is facing.
void *redraw(void *environment) {
  Environment        *env = environment;

  // Open the window
  initializeWindow();

  // Now keep redrawing until shutdown time
  while(env->shutDown == 0) {

    // Erase the background 
    XSetForeground(display, gc, 0xFFFFFF);
    XFillRectangle(display, win, gc, 0, 0, ENV_SIZE, ENV_SIZE);

    // Draw all the balls
    XSetForeground(display, gc, 0x000000); // black
    
    for (int i=0; i<env->numRobots; i++) {
      //printf("Direction: %d\n", direction);
      XSetForeground(display, gc, 0x000000); // black
      XFillArc(display, win, gc,
	       env->robots[i].x-ROBOT_RADIUS, ENV_SIZE-env->robots[i].y-ROBOT_RADIUS,
	       2*ROBOT_RADIUS, 2*ROBOT_RADIUS, 0,
	       360*64);
      XSetForeground(display, gc, 0xFFFFFF); // white
      XDrawLine(display, win, gc,
		env->robots[i].x, ENV_SIZE-env->robots[i].y,
		(int)(env->robots[i].x + (ROBOT_RADIUS*cos((double)env->robots[i].direction/180.0*PI))),
		(int)(ENV_SIZE-env->robots[i].y + (ROBOT_RADIUS*sin((double)-env->robots[i].direction/180.0*PI))));
    }

    XFlush(display);
    usleep(2000);

  }
  closeWindow();

  pthread_exit(NULL);
}
