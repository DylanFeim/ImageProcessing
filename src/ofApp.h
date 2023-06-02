#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxImGui.h"

#include "Constants.h"

class ofApp : public ofBaseApp{
	public:
		ofVideoPlayer		m_videoPlayer;
		ofVideoGrabber		m_videoGrabber;			//for using a webcam. Looks at OF examples under the video folder for more detail.

		ofxCvContourFinder	m_contourFinder;
		bool				m_captureBg;

		//images needed
		ofxCvColorImage		m_colorImage;
		ofxCvGrayscaleImage	m_grayscaleImage;
		ofxCvGrayscaleImage m_grayscaleBg;
		ofxCvGrayscaleImage m_grayscaleDiff;

		//GUI
		ofxImGui::Gui	m_gui;
		int				m_threshold;				//expands "positives" but increases "false positives"
		int				m_numContoursConsidered;	//decide how many contours we want to spend time looking for
		float			m_minArea;					//high pass filter for blob radius
		float			m_maxArea;					//low pass filter for blob radius/size

		void setup();
		void update();
		void draw();

		//!!
		Constants::APP_MODE m_appMode;						//what is the current app state
		std::vector<std::string> m_appModes;				//string/lable version of appMode
		float m_trackedColor[4] = {0.0f, 0.0f, 0.0f, 0.0};	//imgui expects float[4] as color type
		float m_smoothfactor;
		float m_diameters[Constants::MAX_OBJECTS];
		ofVec2f m_movementTarget;
		float trueArea;
		float distance;
		int numObjectsOnScreen;
		int objDist;

		float minDistance;

		ofVec3f locArr[Constants::MAX_OBJECTS];

		char m_objectNames[Constants::MAX_OBJECTS][Constants::MAX_CHAR];

		ofSoundPlayer   detected;
		ofSoundPlayer   undetected;
	
		ofColor contourCol;


		void keyPressed(int key);
		void mousePressed(int x, int y, int button);

		//these will be our image processing functions
		//changing these functiosn to be more agnostic towards what data they manipulate so we
		//can use for processing both a video or love feed
		void processDifference(ofxCvColorImage & image);
		void processColor(ofxCvColorImage & image);
		float lerp(float start, float end, float percent);
};
