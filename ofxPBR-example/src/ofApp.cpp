#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();
    
    //cam.setupPerspective(false, 60, 1, 12000);
    //
    //cubeMap.load("Barce_Rooftop_C_3k.jpg", 1024, true, "filteredMapCache");
    //pbr.setup(1024);
    //pbr.setCubeMap(&cubeMap);
    
    scene = bind(&ofApp::renderScene, this);
    
    //light.setLightType(LightType_Directional);
    //light.setPosition(-1500, 1000, 1500);
    //light.lookAt(ofVec3f(0));
    //light.setScale(1.5);
    //light.setColor(ofFloatColor(1.0));
    //light.setShadowType(ShadowType_Hard);
    //pbr.addLight(&light);
    
    cubeMap.setEnvLevel(0.3);
    
    numShadows = 8;
    
    render.setGeometryInputType(GL_TRIANGLES);
    render.setGeometryOutputType(GL_TRIANGLE_STRIP);
    render.setGeometryOutputCount(int(fmaxf(18, numShadows * 3)));
    render.load("ofxPBRShaders/default.vert", "ofxPBRShaders/default.frag", "ofxPBRShaders/default.geom");
    
    render2.load("shaders/render");
    
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMap);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, shadowMapRes, shadowMapRes, numShadows, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    
    glGenFramebuffers(1, &depthMapFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    camera.assign(numShadows, ofCamera());
    lightPos.assign(numShadows, ofVec3f());
    viewMat.assign(numShadows, ofMatrix4x4());
    shadowMatrix.assign(numShadows, ofMatrix4x4());
    
    for(int i=0;i<numShadows;i++){
        lightPos[i] = ofVec3f(1000 * sin(TWO_PI * float(i) / float(numShadows)), 1000, 1000 * cos(TWO_PI * float(i) / float(numShadows)));
        camera[i].setNearClip(1.0);
        camera[i].setFarClip(3000.0);
        camera[i].setFov(90);
        camera[i].setForceAspectRatio(1.0);
        camera[i].lookAt(ofVec3f(0.0));
        camera[i].setupPerspective();
    }
    
    cam.setNearClip(0.0);
    cam.setFarClip(10000);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    for(int i=0;i<numShadows;i++){
        camera[i].setGlobalPosition(1000 * sin(TWO_PI * float(i)/float(numShadows)), 1000, 1000 * cos(TWO_PI * float(i)/float(numShadows)));
        camera[i].lookAt(ofVec3f(0.0));
        lightPos[i] = camera[i].getPosition();
        viewMat[i] = camera[i].getModelViewMatrix() * camera[i].getProjectionMatrix();
        shadowMatrix[i] = ofMatrix4x4::getInverseOf(cam.getModelViewMatrix()) * camera[i].getModelViewMatrix() * camera[i].getProjectionMatrix() * biasMatrix;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
    glViewport(0, 0, shadowMapRes, shadowMapRes);
    ofClear(0);
    render.begin();
    render.setUniform1i("envMap", 1);
    render.setUniform1i("numLights", numShadows);
	GLuint shadowIndexVert = glGetSubroutineIndex(render.getProgram(), GL_VERTEX_SHADER, "shadow");
	GLuint renderIndexVert = glGetSubroutineIndex(render.getProgram(), GL_VERTEX_SHADER, "render");
	GLuint shadowIndexGeom = glGetSubroutineIndex(render.getProgram(), GL_GEOMETRY_SHADER, "shadow");
	GLuint renderIndexGeom = glGetSubroutineIndex(render.getProgram(), GL_GEOMETRY_SHADER, "render");
    GLuint shadowIndexFrag = glGetSubroutineIndex(render.getProgram(), GL_FRAGMENT_SHADER, "shadow");
    GLuint renderIndexFrag = glGetSubroutineIndex(render.getProgram(), GL_FRAGMENT_SHADER, "render");
    
    GLuint indexArrayVert[] = { shadowIndexFrag };
	GLuint indexArrayGeom[] = { shadowIndexGeom };
	GLuint indexArrayFrag[] = { shadowIndexFrag }; 
    
    render.setUniform1i("renderForDepthMap", true);
    glUniform3fv(glGetUniformLocation(render.getProgram(), "light"), numShadows, &lightPos[0].x);
    glUniformMatrix4fv(glGetUniformLocation(render.getProgram(), "viewMat"), numShadows, GL_FALSE, viewMat[0].getPtr());
	glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, indexArrayVert);
	glUniformSubroutinesuiv(GL_GEOMETRY_SHADER, 1, indexArrayGeom);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, indexArrayFrag);
    renderScene();
    render.end();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    
    
    glActiveTexture( GL_TEXTURE0 + 1 );
    glEnable( GL_TEXTURE_2D_ARRAY );
    glBindTexture( GL_TEXTURE_2D_ARRAY, depthMap );
    
    cam.begin();
    render2.begin();
    render2.setUniform1i("depthMap", 1);
    render2.setUniform1i("numLights", numShadows);
    glUniform3fv(glGetUniformLocation(render2.getProgram(), "lightPos"), numShadows, &lightPos[0].x);
//    glUniform4fv(glGetUniformLocation(render2.getProgram(), "colors"), 8, &color[0].r);
    glUniformMatrix4fv(glGetUniformLocation(render2.getProgram(), "shadowMatrix"), numShadows, GL_FALSE, shadowMatrix[0].getPtr());
    renderScene();
    render2.end();
    cam.end();
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0 );
    glDisable( GL_TEXTURE_2D_ARRAY );
    glActiveTexture( GL_TEXTURE0 );
    
//    pbr.makeDepthMap(scene);
//    
//    cam.begin();
//    pbr.drawEnvironment(&cam);
//    scene();
//    cam.end();
}

//--------------------------------------------------------------
void ofApp::renderScene(){
    ofEnableDepthTest();
    ofPushMatrix();
//    pbr.begin(&cam);
    {
//        material.roughness = 0.0;
//        material.metallic = 0.0;
//        material.begin(&pbr);
        ofDrawBox(0, -40, 0, 2000, 10, 2000);
//        material.end();
        
        ofSetColor(255,0,0);
        for(int i=0;i<10;i++){
//            material.roughness = float(i) / 9.0;
            for(int j=0;j<10;j++){
//                material.metallic = float(j) / 9.0;
//                material.begin(&pbr);
                ofDrawSphere(i * 100 - 450, 0, j * 100 - 450, 35);
//                material.end();
            }
        }
    }
//    pbr.end();
    ofPopMatrix();
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
