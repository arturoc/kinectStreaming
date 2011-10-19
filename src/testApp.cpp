#include "testApp.h"
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>

//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);

	settings.loadFile("settings.xml");
	int quality = settings.getValue("settings:quality",48);
	int keyframe_freq = settings.getValue("settings:keyframe-freq",64);
	bool keyframe_auto = settings.getValue("settings:keyframe-auto",false);
	string server = settings.getValue("settings:server","giss.tv");
	int port = settings.getValue("settings:port",8000);
	string mount = settings.getValue("settings:mount","kinect.ogg");
	string passwd = settings.getValue("settings:passwd","myd9v");
	int width = settings.getValue("settings:width",320);
	int height = settings.getValue("settings:port",240);


	string pipeline = ofVAArgsToString("appsrc  name=video_src is-live=true do-timestamp=true ! "
			"video/x-raw-gray,width=640,height=480,depth=8,bpp=8,framerate=30/1 ! queue ! "
			"videorate ! video/x-raw-gray,depth=8,bpp=8,framerate=30/1 ! videoscale ! "
			"video/x-raw-gray,width=%d,height=%d,depth=8,bpp=8 ! ffmpegcolorspace ! "
			"video/x-raw-yuv,width=%d,height=%d ! theoraenc quality=%d keyframe-auto=%s keyframe-freq=%d ! "
			"oggmux ! shout2send name=shoutsink ip=%s port=%d mount=%s password=%s streamname= description= genre= url="
			,width,height,width,height,quality,keyframe_auto?"true":"false",keyframe_freq,server.c_str(),port,mount.c_str(),passwd.c_str());


	gst.setPipelineWithSink(pipeline,"shoutsink",false);
	gstSrc = (GstAppSrc*)gst_bin_get_by_name(GST_BIN(gst.getPipeline()),"video_src");
	if(gstSrc){
		gst_app_src_set_stream_type (gstSrc,GST_APP_STREAM_TYPE_STREAM);
		g_object_set (G_OBJECT(gstSrc), "format", GST_FORMAT_TIME, NULL);
	}
	kinect.init();
	kinect.open();
	gst.play();
}

//--------------------------------------------------------------
void testApp::update(){
	kinect.update();
	if(kinect.isFrameNew()){
		GstBuffer * buffer;
		buffer = gst_app_buffer_new (kinect.getDepthPixels(), 640*480, NULL, kinect.getDepthPixels());

		GstFlowReturn flow_return = gst_app_src_push_buffer(gstSrc, buffer);
		if (flow_return != GST_FLOW_OK) {
			ofLog(OF_LOG_WARNING,"error pushing buffer");
		}
		gst.update();
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	kinect.drawDepth(0,0);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
