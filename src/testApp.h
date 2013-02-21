#pragma once

#include "ofMain.h"

#include "ofxSyphon.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		
        void drawContours(ofVec2f pos, ofVec2f contourSize);
    
        void exit();
        void changeResolution();

    //Syphon
        ofxSyphonClient syphonInput;
        ofxSyphonServer syphonOutput;
        string syphonAppIn, syphonServerIn, syphonServerOut;
    
    //Copying operations
        ofPixels pix;
        ofVec2f camSize;
        ofTexture tex1;
        ofFbo fboSyphonIn;
        ofFbo fboSyphonOut;
    
        ofxCvColorImage			colorImg;
        ofxCvGrayscaleImage 	grayImage;
		ofxCvGrayscaleImage 	grayBg;
		ofxCvGrayscaleImage 	grayDiff;
        ofxCvContourFinder 	contourFinder;
    
		int 	threshold;
		bool	bLearnBakground;

        bool overView;
    
       //OSC
        ofxOscReceiver  vdmxOscIn;
        ofxOscSender  vdmxOscOut;
        bool colorize;
    
        //contour drawing options
    
        ofColor blobColor;
    
        ofImage colorHold;
    
        float mystery;
    
        ofPoint mapPt, mapCent;
 
        int cvWidth;
        int cvHeight;
    
        bool mysterySwitch;
        float mystery2;
        float lineThick;
        bool fillInContours;
        int noiseAmount;
        bool extraSketches;
        bool boundingBox;
    
    string oscHostOut;
    int oscPortOut;
    int oscPortIn;
    
    ofxXmlSettings xml;
    
    
    

};

