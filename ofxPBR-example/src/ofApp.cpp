#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(0);
	ofSetVerticalSync(false);

    ofDisableArbTex();
    
    cam.setupPerspective(false, 60, 1, 5000);

	scene = bind(&ofApp::renderScene, this);

    cubeMap.load("Barce_Rooftop_C_3k.jpg", 1024, true, "filteredMapCache");
    pbr.setup(scene, &cam, 2048);
    pbr.setCubeMap(&cubeMap);
	pbr.setDrawEnvironment(true);
    
    render.load("ofxPBRShaders/default.vert", "ofxPBRShaders/default.frag");

	light2.setup();
	light2.setLightType(LightType_Directional);
	//light2.setPosition(1500, 1000, 1500);
	//light2.lookAt(ofVec3f(0));
	//light2.setScale(1.5);
	//light2.setColor(ofFloatColor(1.0));
	light2.setShadowType(ShadowType_Soft);
	pbr.addLight(&light2);
    
	light.setup();
	//light.setEnable(true);
	light.setLightType(LightType_Spot);
	light.setShadowType(ShadowType_Soft);
	light.setSpotLightGradient(10.0);
	light.setSpotLightDistance(3000);
	light.setSpotLightCutoff(30);
	//   light.setPosition(-500, 1000, 500);
	//   light.lookAt(ofVec3f(0));
	//light.setScale(1.0);
	//light.setColor(ofFloatColor(1.0));
	//	//light.setSpotLightDistance(5000);
	//	//light.setSpotLightCutoff(45);
	////    light.setPointLightRadius(5000);
	//light.setNearClip(1.0);
	//light.setFarClip(5000);
	pbr.addLight(&light);

    cubeMap.setEnvLevel(0.3);
}

//--------------------------------------------------------------
void ofApp::update(){
	light.setPosition(500 * sin(ofGetElapsedTimef()), 500, 500 * cos(ofGetElapsedTimef()));
	light.lookAt(ofVec3f(0));

	light2.setPosition(-sin(ofGetElapsedTimef()), 1, -cos(ofGetElapsedTimef()));
	light2.lookAt(ofVec3f(0));
}

//--------------------------------------------------------------
void ofApp::draw(){
	prevTime = ofGetElapsedTimef();

	pbr.updateDepthMaps();

	cam.begin();
	pbr.renderScene();
	cam.end();

	pbr.getDirectionalDepthMap(0)->draw(0, 0, 256, 256);

    ofSetWindowTitle(ofToString(ofGetFrameRate()));
	float t = ofGetElapsedTimef() - prevTime;
	ofDrawBitmapString(ofToString(t), 20, 20);
}

//--------------------------------------------------------------
void ofApp::renderScene(){
	ofEnableDepthTest();
	glEnable(GL_CULL_FACE);
    pbr.beginCustomRenderer(&render);
    {
        material.roughness = 0.25;
        material.metallic = 0.0;
        material.begin(&pbr);
		glCullFace(GL_BACK);
        ofDrawBox(0, -40, 0, 2000, 10, 2000);
        material.end();
        
		glCullFace(GL_FRONT);
        for(int i=0;i<10;i++){
            material.roughness = float(i) / 9.0;
            for(int j=0;j<10;j++){
                material.metallic = float(j) / 9.0;
                material.begin(&pbr);
                ofDrawSphere(i * 100 - 450, 0, j * 100 - 450, 35);
                material.end();
            }
        }
	}
	pbr.endCustomRenderer();
	glDisable(GL_CULL_FACE);
	ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	render.load("ofxPBRShaders/default.vert", "ofxPBRShaders/default.frag");
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
