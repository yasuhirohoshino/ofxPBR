#pragma once

#include "ofMain.h"
#include "ofxPBR.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void renderScene();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
    function<void()> scene;
    
    ofxPBRCubeMap cubeMap;
    ofxPBRMaterial material;
    ofxPBRLight light;
    ofxPBR pbr;
    
    ofShader renderShader;
    ofEasyCam cam;
};
