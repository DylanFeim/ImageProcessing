#include "ofApp.h"
#include "Math.h"

//--------------------------------------------------------------
void ofApp::setup() {
	//set app params
	ofSetWindowShape(Constants::APP_WINDOW_WIDTH, Constants::APP_WINDOW_HEIGHT);
	ofSetFrameRate(Constants::APP_DESIRED_FRAMERATE);

	for (int i = 0; i < Constants::MAX_OBJECTS; i++) {
		m_diameters[i] = 0.0f;
	}
	
	char tempStr[32];
	
	for (int i = 0; i < Constants::MAX_OBJECTS; i++) {
		snprintf(tempStr, 32, "Object %u diameter in cm", i+1);
		strcpy(m_objectNames[i], tempStr);
	}

	 trueArea = 0;
	 distance = 0;
	 numObjectsOnScreen = 0;
	 minDistance = 0;

	 detected.load("Detected.mp3");
	 undetected.load("Undetected.mp3");

	 contourCol.set(54, 166, 205);


	 objDist = 0;

	//load video
	if (Constants::USE_LIVE_VIDEO) {
		//listing out devices
		vector<ofVideoDevice> devices = m_videoGrabber.listDevices();
		for (int i = 0; i < devices.size(); i++) {
			if (devices[i].bAvailable) {
				std::cout << devices[i].id << ": " << devices[i].deviceName << std::endl;
			}
			else {
				std::cout << devices[i].id << ": " << devices[i].deviceName << " - unavailable " << std::endl;
			}
		}

		//note that not all cameras will allow you change resolution (i.e. I have had trouble with built-in mac cameras)
		//is this is the case find another USB camera that does, or use video ;) Trying to loop through pixels on a camera
		//at more than a resolution of 320x240, 640x480 is going to cause problems
		m_videoGrabber.setDeviceID(0);
		m_videoGrabber.setDesiredFrameRate(30);
		m_videoGrabber.initGrabber(Constants::CAM_WIDTH, Constants::CAM_HEIGHT);
	}
	else {
		m_videoPlayer.load(Constants::VIDEO_PATH_COLOR_ONE);
		m_videoPlayer.setLoopState(OF_LOOP_NORMAL);
		m_videoPlayer.play();
	}

	//allocate CV images
	m_colorImage.allocate(Constants::VIDEO_WIDTH, Constants::VIDEO_HEIGHT);
	m_grayscaleImage.allocate(Constants::VIDEO_WIDTH, Constants::VIDEO_HEIGHT);
	m_grayscaleBg.allocate(Constants::VIDEO_WIDTH, Constants::VIDEO_HEIGHT);
	m_grayscaleDiff.allocate(Constants::VIDEO_WIDTH, Constants::VIDEO_HEIGHT);

	//init vars
	m_threshold				= 128;
	m_numContoursConsidered = 2;
	m_minArea				= 10.0f;
	m_maxArea				= 15000.0f;
	m_captureBg				= true;


	//init gui
	m_gui.setup();

	//
	m_appMode = Constants::APP_MODE::FIND_DIFFERENCE;
	m_appModes.push_back("Background Subtraction");
	m_appModes.push_back("Color Tracking");
}

//--------------------------------------------------------------
void ofApp::update(){
	if (Constants::USE_LIVE_VIDEO) {
		m_videoGrabber.update();

		if (m_videoGrabber.isFrameNew()) {
			m_colorImage.setFromPixels(m_videoGrabber.getPixels());
		}
	} 
	else {
		m_videoPlayer.update();

		if (m_videoPlayer.isFrameNew()) {
			m_colorImage.setFromPixels(m_videoPlayer.getPixels());
		}
	}

	if (m_videoGrabber.isFrameNew() || m_videoPlayer.isFrameNew()) {
		//Finite-State Machine (FSM) basic introduction ...
		switch (m_appMode) {
			case Constants::APP_MODE::FIND_DIFFERENCE: {
				processDifference(m_colorImage);
			}
			break;
			case Constants::APP_MODE::FIND_COLOR: {
				processColor(m_colorImage);
			}
			break;
		}
	}

	if (m_videoGrabber.isFrameNew() || m_videoPlayer.isFrameNew()) {

		if (m_contourFinder.nBlobs > numObjectsOnScreen) {
			detected.play();
			numObjectsOnScreen++;
		}
		else if (m_contourFinder.nBlobs < numObjectsOnScreen) {
			undetected.play();
			numObjectsOnScreen--;
		}

		//	minDistance
		if (minDistance > 0) {
			contourCol.set(54, 166, 205);
			for (int i = 0; i < m_contourFinder.nBlobs; i++) {
				for (int k = 0; i < m_contourFinder.nBlobs; i++) {
					if (k != i) {
						objDist = locArr[i].distance(locArr[k]);
						cout << objDist << endl;
						if(minDistance > objDist)
							contourCol.set(205, 54, 91);
							
					}
				}
			}
		}

	}


}

//--------------------------------------------------------------
void ofApp::draw(){
	//draw videos
	ofSetColor(255, 255, 255);
	static int imageBorderSize = 10;
	m_grayscaleDiff.draw(imageBorderSize, imageBorderSize);
	m_colorImage.draw(Constants::VIDEO_WIDTH + imageBorderSize * 2.0f, imageBorderSize);
	m_grayscaleBg.draw(imageBorderSize, Constants::VIDEO_HEIGHT + imageBorderSize * 2.0f);
	m_grayscaleImage.draw(Constants::VIDEO_WIDTH + imageBorderSize * 2.0f, Constants::VIDEO_HEIGHT + imageBorderSize * 2.0f);

	//now draw contours (we are ignoring holes)
	static ofVec2f contourCenter;
	static float contourArea = 0;

	ofPushMatrix();
	{
		ofTranslate(imageBorderSize, imageBorderSize);

		for (int i = 0; i < m_contourFinder.nBlobs; i++) {
			contourCenter.set(m_contourFinder.blobs[i].boundingRect.getCenter().x, m_contourFinder.blobs[i].boundingRect.getCenter().y);
			contourArea = m_contourFinder.blobs[i].area;

		
			//draw contour
			m_contourFinder.blobs[i].draw(0.0f, 0.0f);

			//draw center
			ofSetColor(contourCol);
			ofDrawCircle(contourCenter.x, contourCenter.y, 10.0f);

			if (i == 0) {
				m_movementTarget.x = lerp(m_movementTarget.x, contourCenter.x, m_smoothfactor);
				m_movementTarget.y = lerp(m_movementTarget.y, contourCenter.y, m_smoothfactor);

				ofSetColor(ofColor::blueViolet);
				ofDrawCircle(m_movementTarget.x, m_movementTarget.y, 5.0f);
			}
			
			trueArea = M_PI * ((m_diameters[i] / 2) * (m_diameters[i] / 2));
			distance = (trueArea / contourArea*10000)/2+5;

			locArr[i].x = contourCenter.x;
			locArr[i].y = contourCenter.y;
			locArr[i].z = distance;

			//draw textual information
			ofSetColor(0, 255, 0);
			ofDrawBitmapString("Center: " + ofToString(contourCenter.x) + ", " + ofToString(contourCenter.y), m_contourFinder.blobs[i].boundingRect.getMaxX() + imageBorderSize, contourCenter.y);
			if (m_diameters[i]>0)
			ofDrawBitmapString("Distance: " + ofToString(distance)+" cm", m_contourFinder.blobs[i].boundingRect.getMaxX() + imageBorderSize, contourCenter.y + 20.0f);
			else
			ofDrawBitmapString("Area:   " + ofToString(contourArea), m_contourFinder.blobs[i].boundingRect.getMaxX() + imageBorderSize, contourCenter.y + 20.0f);
		}
	}
	ofPopMatrix();

	//draw guid (let's do it last)
	m_gui.begin();
	{
		ImGui::Text("OpenCV Lesson");
		ImGui::SliderInt("Threshold", &m_threshold, 0, 255);
		ImGui::SliderInt("Number of Objects to Consider", &m_numContoursConsidered, 1, Constants::MAX_OBJECTS);
		ImGui::SliderFloat("Min. Area", &m_minArea, 0.0f, Constants::VIDEO_WIDTH * Constants::VIDEO_HEIGHT);
		ImGui::SliderFloat("Max. Area", &m_maxArea, 0.0f, Constants::VIDEO_WIDTH * Constants::VIDEO_HEIGHT);
		ImGui::SliderFloat("Smooth Factor", &m_smoothfactor, 0.0f, 1.0f);
		
	

		ImGui::Separator(); 
		for (int i = 0; i < m_numContoursConsidered; i++) {
			ImGui::SliderFloat(m_objectNames[i], &m_diameters[i], 0.0f, 100);
			//if(m_appMode == Constants::APP_MODE::FIND_COLOR)
			//	ImGui::ColorEdit3("Selected Color", (float*)&m_trackedColor);

		}

		if (m_numContoursConsidered > 1)
			ImGui::SliderFloat("Min Distance", &minDistance, 0.0f, sqrt(Constants::VIDEO_WIDTH * Constants::VIDEO_WIDTH + Constants::VIDEO_HEIGHT * Constants::VIDEO_HEIGHT));

		ImGui::Separator(); 
		ImGui::Text("\n Please select app mode, thank you!");
		static int currentListBoxIndex = 0;
		if (ofxImGui::VectorCombo("App Mode", &currentListBoxIndex, m_appModes)) {	//imgui control conditionals tell us whether someone has interacted with it
			m_appMode = (Constants::APP_MODE)currentListBoxIndex;
		}
		switch (m_appMode) {
			case Constants::APP_MODE::FIND_DIFFERENCE: {
				if (ImGui::Button("Capture New Background")) {
					m_captureBg = true;
				}
			}
		   break;
			case Constants::APP_MODE::FIND_COLOR: {
				ImGui::ColorEdit3("Selected Color", (float*)&m_trackedColor);
				ImGui::Text("\nInstructions: \n-press spacebar to toggle pause on video \n-right-mouse click to select a color");
			}
			break;
		}
	}
	m_gui.end();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 32) {	//32 = spacebar
		if (!Constants::USE_LIVE_VIDEO) {
			m_videoPlayer.setPaused(!m_videoPlayer.isPaused());
		}
	}
	if (key == 't') {	//t = t

		for (int i = 0; i < Constants::MAX_OBJECTS; i++) {
			printf("Diameter %u %f\n", i, m_diameters[i]);
		}

	}
	if (key == 's') {	//s = s

		for (int i = 0; i < Constants::MAX_OBJECTS; i++) {
			printf(m_objectNames[i]);
		}

	}

	if (key == 'a') {	//a = a

		cout << "Number of blobs: " << m_contourFinder.nBlobs << endl;
		cout << "Number of blobs: " << numObjectsOnScreen << endl << endl;

	}

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if (button == OF_MOUSE_BUTTON_RIGHT && m_appMode == Constants::APP_MODE::FIND_COLOR) {
		ofRectangle videorect = ofRectangle(	Constants::VIDEO_WIDTH + Constants::VIDEO_BORDER_SIZE * 2, 
												Constants::VIDEO_BORDER_SIZE, 
												Constants::VIDEO_WIDTH,
												Constants::VIDEO_HEIGHT );

		//step 1 to mapping screenspace to image space
		int convertX = ofClamp(x, videorect.getMinX(), videorect.getMaxX());
		int convertY = ofClamp(y, videorect.getMinY(), videorect.getMaxY());

		//step 2 shifting over origin to 0,0
		convertX -= videorect.getMinX();
		convertY -= videorect.getMinY();

		//get that color yo! index = x + y * width;
		const int index = (convertX + convertY * m_colorImage.getWidth()) * m_colorImage.getPixels().getNumChannels();
		m_trackedColor[0] = m_colorImage.getPixels()[index + 0]/255.0f;	//r
		m_trackedColor[1] = m_colorImage.getPixels()[index + 1]/255.0f;	//g
		m_trackedColor[2] = m_colorImage.getPixels()[index + 2]/255.0f;	//b
	}
}

void ofApp::processDifference(ofxCvColorImage & image) {
	m_grayscaleImage = image;
	if (m_captureBg) {
		m_grayscaleBg = m_grayscaleImage;
		m_captureBg = false; //only want to capture once until we ask to do so again
	}

	//get the difference between images
	m_grayscaleDiff.absDiff(m_grayscaleBg, m_grayscaleImage);

	//maybe try to processing
	m_grayscaleDiff.blurGaussian(3);
	//m_grayscaleDiff.dilate_3x3();
	//m_grayscaleDiff.erode_3x3();

	//adjust threshold
	m_grayscaleDiff.threshold(m_threshold);

	//now look for contours/blobs
	m_contourFinder.findContours(m_grayscaleDiff, m_minArea, m_maxArea, m_numContoursConsidered, false, true);
}

void ofApp::processColor(ofxCvColorImage & image) {
	//I'll see youuu later ... okay I see you now.
	const int numChannelsPerPixel = image.getPixels().getNumChannels();	//3 = r,g,b
	const int numChannels = Constants::VIDEO_WIDTH * Constants::VIDEO_HEIGHT * numChannelsPerPixel;
	float pixel[3] = {0.0f, 0.0f, 0.0f};

	for (int i = 0; i < numChannels; i+= numChannelsPerPixel) {

		//get color
		pixel[0] = image.getPixels()[i + 0];	//r
		pixel[1] = image.getPixels()[i + 1];	//g
		pixel[2] = image.getPixels()[i + 2];	//b

		//now we want to compare to it the tracked color and see if it is the same
		if (	(abs(pixel[0] - m_trackedColor[0] * 255.0f) < m_threshold) &&	//r
				(abs(pixel[1] - m_trackedColor[1] * 255.0f) < m_threshold) &&	//g
				(abs(pixel[2] - m_trackedColor[2] * 255.0f) < m_threshold) 		//b
			) {	
			m_grayscaleDiff.getPixels()[i/numChannelsPerPixel] = 255;	///divide index by numChannelsPerPixel to map from rgb channels / pixel to 1 channale per pixel
		}
		else {
			m_grayscaleDiff.getPixels()[i/numChannelsPerPixel] = 0;
		}
	}

	m_grayscaleDiff.flagImageChanged();	//ofImage.update()
	m_contourFinder.findContours(m_grayscaleDiff, m_minArea, m_maxArea, m_numContoursConsidered, false, true);
}

float ofApp::lerp(float start, float end, float percent) {
	return (start + percent * (end - start));
}

