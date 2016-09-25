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
    ofxPBRLight light, light2;
    ofxPBR pbr;
    
    ofShader renderShader;
    ofEasyCam cam;
    
    ofShader render;
    ofShader render2;

    GLuint depthMap, depthMapFbo;
    ofMatrix4x4 shadowProjMatrix;
    int shadowMapRes = 1024;
    
    
    
    int numShadows;
    vector<ofCamera> camera;
    vector<ofVec3f> lightPos;
    vector<ofMatrix4x4> viewMat;
    vector<ofMatrix4x4> shadowMatrix;
    const ofMatrix4x4 biasMatrix = ofMatrix4x4(
                                               0.5, 0.0, 0.0, 0.0,
                                               0.0, 0.5, 0.0, 0.0,
                                               0.0, 0.0, 0.5, 0.0,
                                               0.5, 0.5, 0.5, 1.0
                                               );
};
