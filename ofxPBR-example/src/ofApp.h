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
    ofxPBRLight directionalLight, pointLight, spotLight;
    ofxPBR pbr;
    
    ofEasyCam cam;

	ofImage groundBaseColor;
	ofImage groundMetallic;
	ofImage groundRoughness;
	ofImage groundNormal;
	ofxPBRMaterial groundMaterial;

	ofxPanel gui;
	ofParameter<ofFloatColor> materialColor;

	ofParameter<bool> enableDerectionalLight;
	ofParameter<ofFloatColor> directionalLightColor;
	
	ofParameter<bool> enablePointLight;
	ofParameter<ofFloatColor> pointLightColor;

	ofParameter<bool> enableSpotLight;
	ofParameter<ofFloatColor> spotLightColor;
};
