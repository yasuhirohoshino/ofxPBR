#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();
    
    cam.setupPerspective(false, 60, 1, 12000);
    renderShader.load("ofxPBRShaders/default");
    
    cubeMap.load("Barce_Rooftop_C_3k.jpg", 1024, true, "filteredMapCache");
    pbr.setup(1024);
    pbr.setCubeMap(&cubeMap);
    
    scene = bind(&ofApp::renderScene, this);
    
    light.setLightType(LightType_Directional);
    light.setPosition(-1500, 1000, 1500);
    light.lookAt(ofVec3f(0));
    light.setScale(1.5);
    light.setColor(ofFloatColor(1.0));
    light.setShadowType(ShadowType_Soft);
    pbr.addLight(&light);
    
    cubeMap.setEnvLevel(0.3);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    pbr.makeDepthMap(scene);
    
    cam.begin();
    pbr.drawEnvironment(&cam);
    scene();
    cam.end();
}

//--------------------------------------------------------------
void ofApp::renderScene(){
    ofEnableDepthTest();
    pbr.begin(&cam, &renderShader);
    
    material.roughness = 0.0;
    material.metallic = 0.0;
    material.begin(pbr.getShader());
    ofDrawBox(0, -40, 0, 2000, 10, 2000);
    material.end();
    
    for(int i=0;i<10;i++){
        material.roughness = float(i) / 9.0;
        for(int j=0;j<10;j++){
            material.metallic = float(j) / 9.0;
            material.begin(pbr.getShader());
            ofDrawSphere(i * 100 - 450, 0, j * 100 - 450, 35);
            material.end();
        }
    }
    
    pbr.end();
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
