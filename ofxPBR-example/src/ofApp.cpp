#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();
    
    cam.setupPerspective(false, 60, 1, 5000);

	scene = bind(&ofApp::renderScene, this);

    cubeMap.load("Barce_Rooftop_C_3k.jpg", 1024, true, "filteredMapCache");
    pbr.setup(scene, &cam, 2048);
	pbr.setUsingCameraFrustomForShadow(false);
	pbr.setDirectionalShadowBBox(0, 0, 0, 1000, 1000, 1000);
    pbr.setCubeMap(&cubeMap);
	pbr.setDrawEnvironment(true);
    
	light1.setup();
	light1.setLightType(LightType_Spot);
	light1.setShadowType(ShadowType_Soft);
	light1.setSpotLightGradient(10.0);
	light1.setSpotLightDistance(3000);
	light1.setSpotLightCutoff(30);
	pbr.addLight(&light1);

	light2.setup();
	light2.setLightType(LightType_Directional);
	light2.setShadowType(ShadowType_Soft);
	pbr.addLight(&light2);

    cubeMap.setEnvLevel(0.3);
}

//--------------------------------------------------------------
void ofApp::update(){
	light1.setPosition(500 * sin(ofGetElapsedTimef()), 500, 500 * cos(ofGetElapsedTimef()));
	light1.lookAt(ofVec3f(0));

	light2.setPosition(-sin(ofGetElapsedTimef()), 1, -cos(ofGetElapsedTimef()));
	light2.lookAt(ofVec3f(0));
}

//--------------------------------------------------------------
void ofApp::draw(){
	pbr.updateDepthMaps();
	cam.begin();
	pbr.renderScene();
	cam.end();
}

//--------------------------------------------------------------
void ofApp::renderScene(){
	ofEnableDepthTest();
	glEnable(GL_CULL_FACE);
	pbr.beginDefaultRenderer();
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
	pbr.endDefaultRenderer();
	glDisable(GL_CULL_FACE);
	ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

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
