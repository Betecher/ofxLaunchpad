#pragma once

#include "ofMain.h"
#include "ofxLaunchpad.h"
#include "ofxLaunchpadToggle.h"
class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);
	
	ofxLaunchpad launchpad;
	ofVideoGrabber camera;
	ofImage pix;
	bool useColor;
};
