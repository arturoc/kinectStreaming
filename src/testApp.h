#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxXmlSettings.h"
#include <gst/app/gstappsrc.h>

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		ofxKinect kinect;
		ofGstVideoUtils gst;
		GstAppSrc * gstSrc;

		ofxXmlSettings settings;
		ofPixels pixels;
		ofPixels depthPixels;
		ofTexture tex;

		bool sendBrightness;
		bool sendRaw;

};
