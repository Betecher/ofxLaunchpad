#include "ofxLaunchpad.h"

// if you click off the window, there will be too many messages
// and the midi will fill up and have trouble
// need some kind of rate limiting buffer?

const int numeratorMin = 1, numeratorMax = 16;
const int denominatorMin = 3, denominatorMax = 18;
const int bufferMask = 1;
const int automapBegin = 104;
const int rowMask = 7;
const int colMask = 15;
const int cols = 9;
const int automapRow = 8;
const int totalButtons = 80;
	
void ofxLaunchpad::setup(int port, ofxLaunchpadListener* listener) {
	midiOut.listPorts();
	midiOut.openPort(port);
	
	setMappingMode();
	setAll();
	
	midiIn.listPorts();
	midiIn.openPort(port);
	midiIn.addListener(this);
	
	if(listener != NULL) {
		addListener(listener);
	}
	
	fbo.allocate(256, 256);

#ifdef LAUNCHPADMK2
	/*
	----------
	| (8, 0) | <= (row, col)
	|   91   | <=    key
	----------

	Launchpad MK2
	---------------------------------------------------------------------------------- sendControlChange
	| (8, 0) | (8, 1) | (8, 2) | (8, 3) | (8, 4) | (8, 5) | (8, 6) | (8, 7) |        |
	|  104   |  105   |  106   |  107   |  108   |  109   |  110   |  111   |        |
	---------------------------------------------------------------------------------- sendNoteOn
	| (0, 0) | (0, 1) | (0, 2) | (0, 3) | (0, 4) | (0, 5) | (0, 6) | (0, 7) | (0, 8) |
	|   81   |   82   |   83   |   84   |   85   |   86   |   87   |   88   |   89   |
	---------------------------------------------------------------------------------- sendNoteOn
	| (1, 0) | (1, 1) | (1, 2) | (1, 3) | (1, 4) | (1, 5) | (1, 6) | (1, 7) | (1, 8) |
	|   71   |   72   |   73   |   74   |   75   |   76   |   77   |   78   |   79   |
	---------------------------------------------------------------------------------- sendNoteOn
	| (2, 0) | (2, 1) | (2, 2) | (2, 3) | (2, 4) | (2, 5) | (2, 6) | (2, 7) | (2, 7) |
	|   61   |   62   |   63   |   64   |   65   |   66   |   67   |   68   |   69   |
	---------------------------------------------------------------------------------- sendNoteOn
	| (3, 0) | (3, 1) | (3, 2) | (3, 3) | (3, 4) | (3, 5) | (3, 6) | (3, 7) | (3, 7) |
	|   51   |   52   |   53   |   54   |   55   |   56   |   57   |   58   |   59   |
	---------------------------------------------------------------------------------- sendNoteOn
	| (4, 0) | (4, 1) | (4, 2) | (4, 3) | (4, 4) | (4, 5) | (4, 6) | (4, 7) | (4, 7) |
	|   41   |   42   |   43   |   44   |   45   |   46   |   47   |   48   |   49   |
	---------------------------------------------------------------------------------- sendNoteOn
	| (5, 0) | (5, 1) | (5, 2) | (5, 3) | (5, 4) | (5, 5) | (5, 6) | (5, 7) | (5, 7) |
	|   31   |   32   |   33   |   34   |   35   |   36   |   37   |   38   |   39   |
	---------------------------------------------------------------------------------- sendNoteOn
	| (6, 0) | (6, 1) | (6, 2) | (6, 3) | (6, 4) | (6, 5) | (6, 6) | (6, 7) | (6, 7) |
	|   21   |   22   |   23   |   24   |   25   |   26   |   27   |   28   |   29   |
	---------------------------------------------------------------------------------- sendNoteOn
	| (7, 0) | (7, 1) | (7, 2) | (7, 3) | (7, 4) | (7, 5) | (7, 6) | (7, 7) | (7, 7) |
	|   11   |   12   |   13   |   14   |   15   |   16   |   17   |   18   |   19   |
	---------------------------------------------------------------------------------- sendNoteOn
	*/
	for (int y = 0; y < 9; y++) {
		if (y == 8) {
			for (int x = 0; x < 8; x++) {
				launchPadMK2Keys[y][x] = 104 + x;
			}
			launchPadMK2Keys[y][8] = -1; // empty position, will skip this position
		}
		else {
			for (int x = 0; x < 9; x++) {
				launchPadMK2Keys[y][x] = (8 - y) * 10 + 1 + x;
			}
		}
	}

	ctrlArea = "ALL";
#endif
}

ofColor boostBrightness(ofColor color) {
	return color / 2 + ofColor(128);
}

void ofxLaunchpad::draw(float x, float y, float width, float height) {
	ofPushStyle();
	ofPushMatrix();
	ofSetCircleResolution(12);
	ofSetLineWidth(MIN(width, height) / (cols * 10));
	ofTranslate(x, y);
	ofScale(width / cols, height / cols);
	
	ofColor outlineColor(64);
	
	ofFill();
	ofSetColor(0);
	ofRect(0, 0, 9, 9);
	ofNoFill();
	ofSetColor(outlineColor);
	ofRect(0, 0, 9, 9);
	
	ofPushMatrix();
	ofTranslate(.5, .5);
	for(int col = 0; col < 8; col++) {
		ofFill();
		ofSetColor(boostBrightness(getLedGrid(col, automapRow)));
		ofCircle(col, 0, .3);
		ofNoFill();
		ofSetColor(outlineColor);
		ofCircle(col, 0, .3);
	}
	ofPopMatrix();
	
	ofTranslate(0, 1);
	for(int row = 0; row < 8; row++) {
		for(int col = 0; col < 8; col++) {
			ofPushMatrix();
			ofTranslate(col, row);
			ofFill();
			ofSetColor(boostBrightness(getLedGrid(col, row)));
			ofRect(.1, .1, .8, .8);
			ofNoFill();
			ofSetColor(outlineColor);
			ofRect(.1, .1, .8, .8);
			ofPopMatrix();
		}
	}
	
	ofPushMatrix();
	ofTranslate(4, 4);
	ofRotate(45);
	ofFill();
	ofSetColor(0);
	ofRect(-.25, -.25, .5, .5);
	ofPopMatrix();
	
	ofTranslate(8, 0);
	ofTranslate(.5, .5);
	for(int row = 0; row < 8; row++) {
		ofFill();
		ofSetColor(boostBrightness(getLedGrid(8, row)));
		ofCircle(0, row, .3);
		ofNoFill();
		ofSetColor(outlineColor);
		ofCircle(0, row, .3);
	}
	
	ofPopMatrix();
	ofPopStyle();
}

void ofxLaunchpad::draw(float x, float y) {
	draw(x, y, getWidth(), getHeight());
}

float ofxLaunchpad::getWidth() {
	return 32 * cols;
}

float ofxLaunchpad::getHeight() {
	return 32 * cols;
}

void ofxLaunchpad::begin() {
	fbo.begin();
	ofPushStyle();
	ofPushMatrix();	
}

void ofxLaunchpad::end() {
	ofPopMatrix();
	ofPopStyle();
	fbo.end();
	fbo.readToPixels(pix);
	pix.crop(0, 0, 8, 8);
	set(pix);
}

void ofxLaunchpad::addListener(ofxLaunchpadListener* listener) {
	ofAddListener(automapButtonEvent, listener, &ofxLaunchpadListener::automapButton);
	ofAddListener(gridButtonEvent, listener, &ofxLaunchpadListener::gridButton);
}

void ofxLaunchpad::removeListener(ofxLaunchpadListener* listener) {
	ofRemoveListener(automapButtonEvent, listener, &ofxLaunchpadListener::automapButton);
	ofRemoveListener(gridButtonEvent, listener, &ofxLaunchpadListener::gridButton);
}

void ofxLaunchpad::setBrightness(float brightness) {
	float dutyCycle = brightness / 2;
	float bestDistance;
	int bestDenominator, bestNumerator;
	for(int denominator = denominatorMin; denominator <= denominatorMax; denominator++) {
		for(int numerator = numeratorMin; numerator <= denominator; numerator++) {
			float curDutyCycle = (float) numerator / (float) denominator;
			float curDistance = abs(dutyCycle - curDutyCycle);
			if((curDistance < bestDistance) || (denominator == denominatorMin && numerator == numeratorMin)) {
				bestDenominator = denominator;
				bestNumerator = numerator;
				bestDistance = curDistance;
			}
		}
	}
	setDutyCycle(bestNumerator, bestDenominator);
}

void ofxLaunchpad::setMappingMode(MappingMode mappingMode) {
	midiOut.sendControlChange(1, 0, mappingMode == XY_MAPPING_MODE ? 1 : 2);
}

void ofxLaunchpad::setLedAutomap(int col, ofxLaunchpadColor color) {
	int key = automapBegin + col;
	int i = automapRow * cols + col;
	if(buffer[i] != color) {
		midiOut.sendControlChange(1, key, color.getMidi());
		buffer[i] = color;
	}
}

void ofxLaunchpad::setLedGrid(int col, int row, ofxLaunchpadColor color) {
#ifdef LAUNCHPADMK2
	if (ctrlArea != "ALL") {
		if (row == 8 || col == 8) return;
	}

	int key = launchPadMK2Keys[row][col];
	if (key == -1) return;
	if (row == 8) {
		midiOut.sendControlChange(1, key, color.getMidi());
	}
	else {
		midiOut.sendNoteOn(1, key, color.getMidi());
	}
	
	int i = row * cols + col;
	if (i >= buffer.size()) return;
	buffer[i] = color;
#elif
	if(row == automapRow) {
		setLedAutomap(col, color);
	} else {
		int a = ((row & rowMask) << 4);
		int b = ((col & colMask) << 0);
		int key = ((row & rowMask) << 4) | ((col & colMask) << 0);
		int i = row * cols + col;
		if(buffer[i] != color) {
			midiOut.sendNoteOn(1, key, color.getMidi());
			buffer[i] = color;
		}
	}

#endif
	

}

ofxLaunchpadColor ofxLaunchpad::getLedGrid(int col, int row) const {
	int ind = row * cols + col;
	if (ind >= buffer.size()) {
		return 0;
	} else {
		return buffer[row * cols + col];
	}
}

void ofxLaunchpad::set(ofPixels& pix, bool clear, bool copy) {
	int i = 0;
	for(int y = 0; y < 8; y++) {
		for(int x = 0; x < 8; x += 2) {
			ofxLaunchpadColor first(pix.getColor(x, y));
			ofxLaunchpadColor second(pix.getColor(x + 1, y));
			midiOut.sendNoteOn(3, first.getMidi(), second.getMidi());
			buffer[i++] = first;
			buffer[i++] = second;
		}
		i++;
	}
	 // any note on signifies that we're done with rapid update
	midiOut.sendNoteOn(1, 127, 0);
}

void ofxLaunchpad::setBufferingMode(bool copy, bool flash, int update, int display) {
	int bufferingMode =
		(0 << 6) |
		(1 << 5) |
		((copy ? 1 : 0) << 4) |
		((flash ? 1 : 0) << 3) |
		((update & bufferMask) << 2) |
		(0 << 1) |
		((display & bufferMask) << 0);
	midiOut.sendControlChange(1, 0, bufferingMode);
}

void ofxLaunchpad::setAll(ofxLaunchpadColor::BrightnessMode brightnessMode) {
	int mode;
	switch(brightnessMode) {
		case ofxLaunchpadColor::OFF_BRIGHTNESS_MODE: mode = 0; break;
		case ofxLaunchpadColor::LOW_BRIGHTNESS_MODE: mode = 125; break;
		case ofxLaunchpadColor::MEDIUM_BRIGHTNESS_MODE: mode = 126; break;
		case ofxLaunchpadColor::FULL_BRIGHTNESS_MODE: mode = 127; break;
	}
	
	buffer.clear();
	buffer.resize(totalButtons, ofxLaunchpadColor(mode));
	
	lastEvent.clear();
	lastEvent.resize(totalButtons);
	
	midiOut.sendControlChange(1, 0, mode);
}

void ofxLaunchpad::setDutyCycle(int numerator, int denominator) {
	numerator = ofClamp(numerator, numeratorMin, numeratorMax);
	denominator = ofClamp(denominator, denominatorMin, denominatorMax);
	int data = (denominator - 3) << 0;
	if(numerator < 9) {
		data |= (numerator - 1) << 4;
		midiOut.sendControlChange(1, 30, data);
	} else {
		data |= (numerator - 9) << 4;
		midiOut.sendControlChange(1, 31, data);
	}
}

void ofxLaunchpad::newMidiMessage(ofxMidiEventArgs& args) {
	int pressed = args.byteTwo > 0;
	int grid = args.status == MIDI_NOTE_ON;
	if(grid) {
		int row = (args.byteOne >> 4) & rowMask;
		int col = (args.byteOne >> 0) & colMask;
		int i = row * cols + col;
		ButtonEvent event(col, row, pressed, &lastEvent[i]);
		ofNotifyEvent(gridButtonEvent, event);
		lastEvent[i] = event;
	} else {
		int row = automapRow;
		int col = (args.byteOne - automapBegin) & colMask;
		int i = row * cols + col;
		ButtonEvent event(col, row, pressed, &lastEvent[i]);
		ofNotifyEvent(automapButtonEvent, event);
		lastEvent[i] = event;
	}
}