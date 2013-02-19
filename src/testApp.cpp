#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
    //Brings in image, draws lines around it, control over OSC

    camSize.x = 640; //keeping this small for examples, but change this if necessary
    camSize.y = 480;
    
    //Allocate for CV...shrinking if necessary
    if (camSize.x>=320 || camSize.y>=240) {
        
        colorImg.allocate(320, 240);
        grayImage.allocate(320, 240);
        grayBg.allocate(320, 240);
        grayDiff.allocate(320, 240);
        
    } else{
        //Or if it's smaller than that...shrink it down
        grayImage.allocate(camSize.x, camSize.y);
        grayBg.allocate(camSize.x, camSize.y);
        grayDiff.allocate(camSize.x, camSize.y);
        colorImg.allocate(camSize.x, camSize.y); 
        
    }
    
    //FBO 
    fboSyphonIn.allocate(camSize.x, camSize.y, GL_RGB);
    fboSyphonOut.allocate(camSize.x, camSize.y, GL_RGB);
    
    pix.allocate(camSize.x, camSize.y, 3);
    
    ofEnableAlphaBlending();
    
    fboSyphonIn.begin();
    ofClear(0,0,0,0);
    fboSyphonIn.end();
    
    fboSyphonOut.begin();
    ofClear(0,0,0,0);
    fboSyphonOut.end();

   
	bLearnBakground = true;
	threshold = 80;
    
    //Syphon setup
    ofSetFrameRate(30);
    
    syphonAppIn = "VDMX5";
    
    syphonInput.setup();
    syphonInput.setApplicationName(syphonAppIn);
    syphonInput.setServerName("To_CV_1");
    syphonOutput.setName("from_CV");
    
    //Info
    overView = false;
    
    //OSC
    vdmxOscIn.setup(3141); //VDMX OUTGOING OSC PORT
    vdmxOscOut.setup(HOST, PORT);
    
    //Initial values for CV
    colorize = false;
    
    mystery = .05;
    
    mysterySwitch = true;
    extraSketches = false;
    mystery2 = 0.0;
    lineThick = 0.1;
    
    
}

//--------------------------------------------------------------
void testApp::update(){
    

   // ofClear(0, 0, 0, 0 );
	ofBackground(100,100,100);
        
    if (camSize.x>=320 || camSize.y>=240) {
        pix.resize(320, 240);     //let's keep this small...no need to process much more than this
    }
    
    colorImg.setFromPixels(pix);
    colorImg.flagImageChanged();
    
    grayImage = colorImg;
    if (bLearnBakground == true){
        grayBg = grayImage;	
        bLearnBakground = false;
    }

    grayDiff.absDiff(grayBg, grayImage);
    grayDiff.threshold(threshold);

    contourFinder.findContours(grayDiff, 20, (320*240)/3, 10, true);	
    
    // check for waiting messages in OSC - add new OSC messages in here
	while(vdmxOscIn.hasWaitingMessages()){
        
		// get the next message
		ofxOscMessage m;
		vdmxOscIn.getNextMessage(&m);
        
		if(m.getAddress() == "/FromVDMX/threshold"){
			threshold = 255*m.getArgAsFloat(0);
		}
        if(m.getAddress() == "/FromVDMX/colorize"){
			colorize = m.getArgAsFloat(0); //eh..incoming as bool..
		}
        if(m.getAddress() == "/FromVDMX/lineThickness"){
			lineThick = 0.001+10*m.getArgAsFloat(0);
		}
        if(m.getAddress() == "/FromVDMX/mystery"){
			mystery = m.getArgAsFloat(0);
		}
        if(m.getAddress() == "/FromVDMX/mystery2"){
			mystery2 = m.getArgAsFloat(0);
		}
        if (m.getAddress() == "/FromVDMX/backgroundCapture") {
            bLearnBakground = m.getArgAsFloat(0);
        }
        if (m.getAddress() == "/FromVDMX/fillInContours") {
            fillInContours = m.getArgAsFloat(0);
        }
        if (m.getAddress() == "/FromVDMX/mysterySwitch") {
            mysterySwitch = m.getArgAsFloat(0);
        }
        
        if (m.getAddress() == "/FromVDMX/noiseAmount") {
            noiseAmount = 100*m.getArgAsFloat(0);
        }
        if (m.getAddress() == "/FromVDMX/extraSketches") {
            extraSketches = m.getArgAsFloat(0);
        }
        if (m.getAddress() == "/FromVDMX/extraSketches") {
            boundingBox = m.getArgAsFloat(0);
        }
    }

}

//--------------------------------------------------------------
void testApp::draw(){
    
    ofBackground(0, 0, 0);
    
    //1. Draw the syphon output image to an FBO...this seems to work better if in draw instead of update
    ofSetColor(255);
    fboSyphonIn.begin();    //start capturing your FBO
        ofClear(0,0,0,0);       //clear it
        syphonInput.draw(0,0);  //draw the incompatible syphon texture to an FBO. Syphon texture cannot be used with read to pixels because it is a different texture type than ofTexture's internal type
    fboSyphonIn.end();                      
    
    //2. Take that FBO reference image (ie the image input) and copy it to some pixels
    //We have to use pixels here because CV operations cannot be performed on textures
    fboSyphonIn.getTextureReference().readToPixels(pix);//these will be read in later in update
    
    
    
    ofSetColor(255);
    fboSyphonOut.begin(); 
        ofClear(0,0,0,0);
        drawContours(ofVec2f(0,0), ofVec2f(ofGetWidth(),ofGetHeight())); //go to our drawing contours function where we'll do drawing operations on data derived from the CV blob detection.
    fboSyphonOut.end();

    
    fboSyphonOut.draw(0, 0); //4. draw the texture to the OF screen - this is optional since we have our texture now...it can just be sent back out

    syphonOutput.publishTexture(&fboSyphonOut.getTextureReference()); //send the texture out via Syphon

    //Debug view to show different stages of CV and also shows your background image
    if(overView){
        ofSetColor(255);
        ofDrawBitmapString("Press 1 for 640 480, and 2 for 1280 720", 20,20);
        char reportStr[1024];
        sprintf(reportStr, "bg subtraction and blob detection\npress ' ' to capture bg\nthreshold %i (press: +/-)\nnum blobs found %i, fps: %f", threshold, contourFinder.nBlobs, ofGetFrameRate());
        ofDrawBitmapString(reportStr, 20, 600);
        ofDrawBitmapString("Syphon In size: " + ofToString(syphonInput.getWidth()) + " " + ofToString(syphonInput.getHeight()), 20, 660);
        
        
        ofSetColor(255);
        colorImg.draw(20,20);
        grayImage.draw(360,20);
        grayBg.draw(20,280);
        grayDiff.draw(360,280);

        for (int i = 0; i < contourFinder.nBlobs; i++){
            contourFinder.blobs[i].draw(360,540);
        }
    }


}

void testApp::drawContours(ofVec2f pos, ofVec2f contourSize){

    
    if(colorize){
        colorHold.setFromPixels(pix.getPixels(), camSize.x, camSize.y, OF_IMAGE_COLOR); //sample the color image for use later. Done here so it's not sampling 1000 times within the for loop
    }
    

    
    if (camSize.x>=320 || camSize.y>=240) {
        cvWidth=320;
        cvHeight = 240;
    }else{
        cvWidth = camSize.x;
        cvHeight = camSize.y;
    }

    ofPushMatrix();
    ofTranslate(pos.x, pos.y); //move the whole thing over
    
    
    //let's process these blobs...
    for(int i=0; i<(int)contourFinder.blobs.size(); i++ ) {
        
        //First, lets send the centroid of each blob over OSC
        if(i<10){ //but only the first 10 so we don't go too crazy...
            ofxOscMessage x;
            x.setAddress("/CV_OUT/blob/"+ofToString(i)+"/x");
            mapCent.x = ofMap(contourFinder.blobs[i].centroid.x, 0, cvWidth,0, 1.0); //lets scale these from 0-1.0 instead for ease....
            x.addFloatArg(mapCent.x);

            vdmxOscOut.sendMessage(x);
            ofxOscMessage y;
            y.setAddress("/CV_OUT/blob/"+ofToString(i)+"/y");
            mapCent.y = ofMap(contourFinder.blobs[i].centroid.y, 0, cvHeight,0, 1.0);
      
            y.addFloatArg(mapCent.y);
            vdmxOscOut.sendMessage(y);
        }
        
        if(fillInContours){
            ofFill();
        } else{
            ofNoFill();
        }
        
        ofSetLineWidth(lineThick);
        
        ofBeginShape();
        
        //For each blob, go through it's individual points and draw a shape
        for( int j=0; j<contourFinder.blobs[i].nPts; j=j+ofMap(mystery, 0, 1, 1, 70)) {
            mapPt.x=ofMap(contourFinder.blobs[i].pts[j].x, 0, cvWidth, 0, contourSize.x);
            mapPt.y=ofMap(contourFinder.blobs[i].pts[j].y, 0, cvHeight, 0, contourSize.y);
             
            float numPts = contourFinder.blobs[i].nPts;
            float positionNum = j; //cast to float was being a pain when dividing...doing it this way for now
            ofVertex(mapPt.x + noiseAmount*ofSignedNoise(10*(positionNum/numPts)+5*ofGetElapsedTimef()),
                     mapPt.y + noiseAmount*ofSignedNoise(10*(positionNum/numPts)+10*ofGetElapsedTimef()) ); //add extra vertices
            //Colorize the blob
            if(colorize){
                blobColor.set(colorHold.getColor(mapPt.x, mapPt.y));
                ofSetColor(blobColor);
            }else{
                ofSetColor(255);
            }
            
            //Draw lines from the centroid to the outside edges
            if(mysterySwitch){
                mapCent.x = ofMap(contourFinder.blobs[i].centroid.x, 0, cvWidth,0, contourSize.x);
                mapCent.y = ofMap(contourFinder.blobs[i].centroid.y, 0, cvHeight,0, contourSize.y);
                ofLine(mapPt.x, mapPt.y, mapCent.x, mapCent.y);
                //ofVertex( mapCent );
                //ofVertex( mapPt );
                
            }
        }
        
        //Draw random sketches over all the other lines
        if(extraSketches){
            //ExtraSketches - just draws extra points in at varying positions
            if (mystery2>0) {    
                for (int k = 1; k<ofMap(mystery2, 0.0, 1.0, 1, 30); k++) {
                    for( int j=0; j<contourFinder.blobs[i].nPts; j+=k*4 ) {
                        mapPt.x=ofMap(contourFinder.blobs[i].pts[j].x,0,cvWidth,0,contourSize.x);
                        mapPt.y=ofMap(contourFinder.blobs[i].pts[j].y,0,cvHeight,0,contourSize.y);
                        ofVertex( mapPt.x, mapPt.y );
                    }
                }
            }
        }
        

        ofEndShape();  
    }
    if(boundingBox){
        ofPushStyle();
        ofNoFill();
        for( int i=0; i<contourFinder.blobs.size(); i++ ) {
            ofRect( ofMap(contourFinder.blobs[i].boundingRect.x, 0,cvWidth,0,contourSize.x), 
                   ofMap(contourFinder.blobs[i].boundingRect.y, 0,cvHeight,0,contourSize.y),
                   ofMap(contourFinder.blobs[i].boundingRect.width, 0,cvWidth,0,contourSize.x), 
                   ofMap(contourFinder.blobs[i].boundingRect.height, 0,cvHeight,0,contourSize.y) );
        }
        ofPopStyle();
    }
    
    ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	switch (key){
		case ' ':
			bLearnBakground = true;
			break;
		case '+':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '-':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
        case 'd':
            overView = !overView;
            break;
        case '1':
            ofSetWindowShape(640, 480);
            break;
        case '2':
            ofSetWindowShape(1280, 720);
            break;
        case '3':
            ofSetWindowShape(1920, 1080); //eh...
            break;
	}
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
  
    //Re-allocate if window size changes. Linked to a key because it will cause issues if trying to drag to resize
    camSize.x = w;
    camSize.y = h;
    
    //Allocate for CV
    if (camSize.x>=320 || camSize.y>=240) {
        
        colorImg.allocate(320, 240);
        grayImage.allocate(320, 240);
        grayBg.allocate(320, 240);
        grayDiff.allocate(320, 240);
        
    } else{
        //Or if it's smaller than that...shrink it down
        grayImage.allocate(camSize.x, camSize.y);
        grayBg.allocate(camSize.x, camSize.y);
        grayDiff.allocate(camSize.x, camSize.y);
        colorImg.allocate(camSize.x, camSize.y); 
        
    }
    
    //FBO 
    fboSyphonIn.allocate(camSize.x, camSize.y, GL_RGB);
    fboSyphonOut.allocate(camSize.x, camSize.y, GL_RGB);
    
    pix.allocate(camSize.x, camSize.y, 3);
    
    ofEnableAlphaBlending();
    
    fboSyphonIn.begin();
    ofClear(0,0,0,0);
    fboSyphonIn.end();
    
    fboSyphonOut.begin();
    ofClear(0,0,0,0);
    fboSyphonOut.end();
    
    cvWidth = camSize.x;
    cvHeight = camSize.y;
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
//--------------------------------------------------------------
void testApp::exit(){ 
    
}

//--------------------------------------------------------------
void testApp::changeResolution(){ 

}
