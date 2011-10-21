#include "testApp.h"
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>

//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);

	if(settings.loadFile("settings.xml")){
		ofLogVerbose() << "settings loaded ok";
	}else{
		ofLogError() << "error loading settings";
	}

	int quality = settings.getValue("settings:quality",48);
	int keyframe_freq = settings.getValue("settings:keyframe-freq",64);
	bool keyframe_auto = settings.getValue("settings:keyframe-auto",true);
	string server = settings.getValue("settings:server","91.121.134.23");
	int port = settings.getValue("settings:port",8002);
	string mount = settings.getValue("settings:mount","kinect.ogg");
	string passwd = settings.getValue("settings:passwd","myd9v");
	int width = settings.getValue("settings:width",320);
	int height = settings.getValue("settings:height",240);

	sendBrightness=settings.getValue("settings:sendBrightness",false);;

	string appsrc;
	string videorate;
	string videoscale;
	if(sendBrightness){
		appsrc = "appsrc  name=video_src is-live=true do-timestamp=true ! "
				"video/x-raw-rgb,width=640,height=480,depth=24,bpp=24,framerate=30/1,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! queue ! ";
		videorate  = "videorate ! video/x-raw-rgb,depth=24,bpp=24,framerate=25/2,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! ";
		videoscale = "videoscale ! video/x-raw-rgb,width=" + ofToString(width) + ",height=" + ofToString(height) + ",depth=24,bpp=24,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! ";
	}else{
		appsrc = "appsrc  name=video_src is-live=true do-timestamp=true ! "
						"video/x-raw-gray,width=640,height=480,depth=8,bpp=8,framerate=30/1 ! queue ! ";
		videorate  = "videorate ! video/x-raw-gray,depth=8,bpp=8,framerate=25/2 ! ";
		videoscale = "videoscale ! video/x-raw-gray,width=" + ofToString(width) + ",height=" + ofToString(height) + ",depth=8,bpp=8 ! ";
	}


	string colorspace = " ffmpegcolorspace ! video/x-raw-yuv,width=" + ofToString(width) + ",height=" + ofToString(height) + " ! ";
	string theoraenc = ofVAArgsToString("theoraenc quality=%d keyframe-auto=%s keyframe-freq=%d ! oggmux ! ",quality,keyframe_auto?"true":"false",keyframe_freq);
	string shoutcast = ofVAArgsToString("shout2send name=shoutsink ip=%s port=%d mount=%s password=%s streamname= description= genre= url=",server.c_str(),port,mount.c_str(),passwd.c_str());


	string pipeline = appsrc + videorate + videoscale + colorspace + theoraenc + shoutcast;


			/*ofVAArgsToString("appsrc  name=video_src is-live=true do-timestamp=true ! "
			"video/x-raw-rgb,width=640,height=480,depth=24,bpp=24,framerate=30/1,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! queue ! "
			"videorate ! video/x-raw-rgb,depth=24,bpp=24,framerate=25/2,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! videoscale ! "
			"video/x-raw-rgb,width=%d,height=%d,depth=24,bpp=24,endianness=4321,red_mask=16711680, green_mask=65280, blue_mask=255 ! ffmpegcolorspace ! "
			"video/x-raw-yuv,width=%d,height=%d ! theoraenc quality=%d keyframe-auto=%s keyframe-freq=%d ! "
			"oggmux ! shout2send name=shoutsink ip=%s port=%d mount=%s password=%s streamname= description= genre= url="
			,width,height,width,height,quality,keyframe_auto?"true":"false",keyframe_freq,server.c_str(),port,mount.c_str(),passwd.c_str());*/


	gst.setPipelineWithSink(pipeline,"shoutsink",false);
	gstSrc = (GstAppSrc*)gst_bin_get_by_name(GST_BIN(gst.getPipeline()),"video_src");
	if(gstSrc){
		gst_app_src_set_stream_type (gstSrc,GST_APP_STREAM_TYPE_STREAM);
		g_object_set (G_OBJECT(gstSrc), "format", GST_FORMAT_TIME, NULL);
	}

	pixels.allocate(640,480,OF_IMAGE_COLOR);
	if(sendBrightness){
		tex.allocate(640,480,GL_RGB);
	}


	kinect.init();
	kinect.setUseRegistration(true);
	kinect.setClippingInMillimeters(500, 1500);
	kinect.open();
	gst.play();
}

//--------------------------------------------------------------
void testApp::update(){
	kinect.update();
	if(kinect.isFrameNew()){
		GstBuffer * buffer;
		if(sendBrightness){
			unsigned char * lpBits24 = kinect.getPixels();
			for(int i=0;i<640*480;i++){
				int nPos24 = i*3;
				pixels[nPos24] = kinect.getDepthPixels()[i];
				#ifdef DO_RGB565

				unsigned char Red24   = lpBits24[nPos24+2]; // 8-bit red
				unsigned char Green24 = lpBits24[nPos24+1]; // 8-bit green
				unsigned char Blue24  = lpBits24[nPos24+0]; // 8-bit blue

				unsigned char Red16   = Red24   >> 3;  // 5-bit red
				unsigned char Green16 = Green24 >> 2;  // 6-bit green
				unsigned char Blue16  = Blue24  >> 3;  // 5-bit blue

				unsigned short RGB2Bytes = Blue16 + (Green16<<5) + (Red16<<(5+6));
				pixels[i*3+1] = *((unsigned char*)(&RGB2Bytes));
				pixels[i*3+2] =  *((unsigned char*)((&RGB2Bytes)+1));
				#endif

				pixels[i*3+1] = (lpBits24[i*3]+lpBits24[i*3+1]+lpBits24[i*3+2])*.3333;
				pixels[i*3+2] =  0;
			}
			buffer = gst_app_buffer_new (pixels.getPixels(), 640*480*3, NULL, pixels.getPixels());
		}else{
			buffer = gst_app_buffer_new (kinect.getDepthPixels(), 640*480, NULL, kinect.getDepthPixels());
		}

		GstFlowReturn flow_return = gst_app_src_push_buffer(gstSrc, buffer);
		if (flow_return != GST_FLOW_OK) {
			ofLog(OF_LOG_WARNING,"error pushing buffer");
		}
		gst.update();
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	if(sendBrightness){
		tex.loadData(pixels);
		tex.draw(0,0);
	}else{
		kinect.drawDepth(0,0);
	}
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
