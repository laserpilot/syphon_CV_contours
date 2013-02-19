/*
 *  ofxSyphonServer.h
 *  syphonTest
 *
 *  Created by astellato,vade,bangnoise on 11/6/10.
 *
 *  http://syphon.v002.info/license.php
 */

#include "ofMain.h"

class ofxSyphonClient {
	public:
	ofxSyphonClient();
	~ofxSyphonClient();
	
    void setup ();
    
    void setApplicationName(string appName);
    void setServerName(string serverName);
  
    void bind();
    void unbind();
    
    void draw(float x, float y, float w, float h);
    void draw(float x, float y);
    
    ofTexture& getTexture(); //Does not return same texture type as used internally by OF...can't do pixel copying
    int getWidth(), getHeight();
    
	protected:
	void* mClient;
    void* latestImage;
	ofTexture mTex;
	int width, height;
	bool bSetup;
	string name;
};