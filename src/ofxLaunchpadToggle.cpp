#include "ofxLaunchpadToggle.h"

ofxLaunchpadToggle::ofxLaunchpadToggle()
:toggleMode(HOLD_MODE) {
}

void ofxLaunchpadToggle::setup(int outPort, int inPort, ofxLaunchpadListener* listener) {
	ofxLaunchpad::setup(outPort, inPort, listener);
	addListener(this);
}

void ofxLaunchpadToggle::setToggleMode(ToggleMode toggleMode) {
	this->toggleMode = toggleMode;
}

void ofxLaunchpadToggle::launchpadEvent(ButtonEvent buttonEvent) {
	int row = buttonEvent.row;
	int col = buttonEvent.col;
	bool pressed = buttonEvent.pressed;
	float duration = buttonEvent.duration;
	ofxLaunchpadColor cur = getLedGrid(col, row);
	switch(toggleMode) {
		case MOMENTARY_MODE:
			setLedGrid(col, row, ofxLaunchpadColor(!cur.isOn()));
			break;
		case TOGGLE_MODE:
			if(pressed) {
				setLedGrid(col, row, ofxLaunchpadColor(!cur.isOn()));
			}
			break;
		case CYCLE_MODE:
			if(pressed) {
				if(cur.isRed()) {
					setLedGrid(col, row, ofColor::green);
				} else if(cur.isGreen()) {
					setLedGrid(col, row, ofColor::black);
				} else {
					setLedGrid(col, row, ofColor::red);
				}
			}
			break;
		case HOLD_MODE:
			if(pressed) {
				if(cur.isOn()) {
					setLedGrid(col, row, false);
				} else {
					setLedGrid(col, row, ofColor::green);
				}
			} else {
				if(cur.isOn() && duration > .2) {
					setLedGrid(col, row, ofColor::red);
				}
			}
			break;
	}
}