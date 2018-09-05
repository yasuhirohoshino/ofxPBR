#include "ofxPBRCubeMap.h"

ofxPBRCubeMap::ofxPBRCubeMap() {
    baseSize = 512;
}

void ofxPBRCubeMap::load(ofImage * sphereMapImage, int baseSize){
	loadShaders();
    this->baseSize = baseSize;
    maxMipLevel = log2(baseSize) + 1;
    
    for(int i = 0; i < 6; i++){
        iTex.filteredCubeMapFaces[i].assign(maxMipLevel, ofImage());
    }
    
    iTex.rawTexture = *sphereMapImage;
    envTexture = iTex.rawTexture.getTexture();
    textureFormat = GL_RGB;
    generate();
	bIsAllocated = true;
}

void ofxPBRCubeMap::load(ofFloatImage * sphereMapImage, int baseSize){
	loadShaders();
    this->baseSize = baseSize;
    maxMipLevel = log2(baseSize) + 1;
    
    for(int i = 0; i < 6; i++){
        fTex.filteredCubeMapFaces[i].assign(maxMipLevel, ofFloatImage());
    }
    
    fTex.rawTexture = *sphereMapImage;
    envTexture = fTex.rawTexture.getTexture();
    textureFormat = GL_RGB32F;
    generate();
	bIsAllocated = true;
}

void ofxPBRCubeMap::load(string imagePath, int baseSize, bool useCache, string cacheDirectry){
	loadShaders();
    this->baseSize = baseSize;
    maxMipLevel = log2(baseSize) + 1;
    
    if(isHDRImagePath(imagePath)){
        for(int i = 0; i < 6; i++){
            fTex.filteredCubeMapFaces[i].assign(maxMipLevel, ofFloatImage());
        }
    }else{
        for(int i = 0; i < 6; i++){
            iTex.filteredCubeMapFaces[i].assign(maxMipLevel, ofImage());
        }
    }
    
    if(useCache){
        bool hasCache = false;
        ofFilePath path;
        ofDisableArbTex();
        string cachePath = "";

        string fileName = path.getFileName(imagePath);
        string cacheName = "FCM_Cache_" + ofToString(baseSize) + "_" + path.getFileName(imagePath);
        ofDirectory dir;
        
        if(cacheDirectry == ""){
            dir.open(path.getEnclosingDirectory(imagePath));
        }else{
            dir.open(cacheDirectry);
        }
        
        for(int i=0; i<dir.getFiles().size(); i++){
            if(dir.getName(i) == cacheName){
                cachePath = dir.getPath(i);
                hasCache = true;
            }
        }
        
        if(hasCache){
            loadFromCache(cachePath);
        }else{
            loadImage(imagePath);
            generate();
            
            string directry = "";
            if(cacheDirectry != ""){
                directry = cacheDirectry + "/";
            }
            makeCache(directry + cacheName);
        }
    }else{
        loadImage(imagePath);
        generate();
    }
    
    ofEnableArbTex();
	bIsAllocated = true;
}

void ofxPBRCubeMap::loadImage(string imagePath){
    if(isHDRImagePath(imagePath)){
        fTex.rawTexture.load(imagePath);
        envTexture = fTex.rawTexture.getTexture();
        textureFormat = GL_RGB32F;
    } else {
        iTex.rawTexture.load(imagePath);
        envTexture = iTex.rawTexture.getTexture();
        textureFormat = GL_RGB;
    }
}

void ofxPBRCubeMap::loadFromCache(string cachePath){
	loadShaders();
    ofDisableArbTex();
    if(isHDRImagePath(cachePath)){
        fTex.cacheTexture.load(cachePath);
        textureFormat = GL_RGB32F;
        baseSize = fTex.cacheTexture.getWidth() / 3;
    } else {
        iTex.cacheTexture.load(cachePath);
        textureFormat = GL_RGB;
        baseSize = iTex.cacheTexture.getWidth() / 3;
    }
    
    maxMipLevel = log2(baseSize) + 1;
    for(int i = 0; i < 6; i++){
        iTex.filteredCubeMapFaces[i].assign(maxMipLevel, ofImage());
        fTex.filteredCubeMapFaces[i].assign(maxMipLevel, ofFloatImage());
    }
        
    int texWidth = baseSize;
    int texHeight = baseSize;
    
    int offsetX = 0;
    int offsetY = 0;
    
    for(int i=0; i<maxMipLevel; i++){
        if(i != 0){
            offsetY = baseSize * 2;
        }
        for(int j=0;j<6;j++){
            envFbo[j].allocate(texWidth, texHeight, textureFormat);
            if(textureFormat == GL_RGB32F) {
                envFbo[j].begin();
                ofClear(0);
                fTex.cacheTexture.draw(-((j % 3) * texWidth + offsetX), -(floor(j / 3) * texHeight + offsetY));
                envFbo[j].end();
                ofFloatPixels _pix;
                envFbo[j].readToPixels(_pix);
                fTex.filteredCubeMapFaces[j][i].setFromPixels(_pix);
            }else{
                envFbo[j].begin();
                ofClear(0);
                iTex.cacheTexture.draw(-((j % 3) * texWidth + offsetX), -(floor(j / 3) * texHeight + offsetY));
                envFbo[j].end();
                ofPixels _pix;
                envFbo[j].readToPixels(_pix);
                iTex.filteredCubeMapFaces[j][i].setFromPixels(_pix);
            }
        }
        if(i > 0){
            offsetX += texWidth * 3;
        }
        texWidth /= 2;
        texHeight /= 2;
    }
    makeFilteredCubeMap();
    cacheEnvFbo.allocate(baseSize, baseSize * 0.5, textureFormat);
    if (textureFormat == GL_RGB32F) {
        cacheEnvFbo.begin();
        ofClear(0);
        fTex.cacheTexture.draw(-baseSize * 1.5, -baseSize * 2.5);
        cacheEnvFbo.end();
        ofFloatPixels _pix;
        cacheEnvFbo.readToPixels(_pix);
        fTex.rawTexture.setFromPixels(_pix);
    }else{
        cacheEnvFbo.begin();
        ofClear(0);
        iTex.cacheTexture.draw(-baseSize * 1.5, -baseSize * 2.5);
        cacheEnvFbo.end();
        ofPixels _pix;
        cacheEnvFbo.readToPixels(_pix);
        iTex.rawTexture.setFromPixels(_pix);
    }
    ofEnableArbTex();
    
	bIsAllocated = true;
}

void ofxPBRCubeMap::loadShaders()
{
	shader.setupShaderFromSource(GL_VERTEX_SHADER, importanceSampling.gl3VertShader);
	shader.setupShaderFromSource(GL_FRAGMENT_SHADER, importanceSampling.gl3FragShader);
	shader.bindDefaults();
	shader.linkProgram();
}

void ofxPBRCubeMap::generate(){
    cacheWidth = baseSize * 3;
    cacheHeight = baseSize * 3;
    cacheFbo.allocate(cacheWidth, cacheHeight, textureFormat);
    
    ofPushStyle();
    ofVec3f target[6];
    target[0] = ofVec3f(90,0,0); // pos x
    target[1] = ofVec3f(-90,0,0); //neg x
    target[2] = ofVec3f(0,90,180); //pos y
    target[3] = ofVec3f(0,-90,180); //neg y
    target[4] = ofVec3f(-180,0,0); //pos z
    target[5] = ofVec3f(0,0,0); //neg z
    
    ofEnableDepthTest();
    ofDisableArbTex();
    
    sphereMesh = ofSpherePrimitive(2048, 100).getMesh();
    for(int i=0;i<sphereMesh.getNormals().size();i++){
        sphereMesh.setNormal(i, ofVec3f(-1.0, -1.0, 1.0) * glm::normalize(sphereMesh.getVertex(i)));
    }
    envSphereMesh = ofSpherePrimitive(2048, 100).getMesh();
    for(int i=0;i<envSphereMesh.getTexCoords().size();i++){
        envSphereMesh.setTexCoord(i, ofVec2f(1.0 - envSphereMesh.getTexCoord(i).x, 1.0 - envSphereMesh.getTexCoord(i).y));
    }
    
    // render raw cubemap faces
    for(int i=0;i<6;i++){
		envCam[i] = ofCamera();
        envCam[i].setFov(90.0);
        envCam[i].setNearClip(0.1);
        envCam[i].setFarClip(6000);
        envCam[i].setPosition(0, 0, 0);
        envFbo[i].allocate(baseSize, baseSize, textureFormat);
        envFbo[i].begin();
        ofClear(255);
        envCam[i].pan(target[i].x);
        envCam[i].tilt(target[i].y);
        envCam[i].roll(target[i].z);
        envCam[i].begin();
        envTexture.bind();
        envSphereMesh.draw();
        envTexture.unbind();
        envCam[i].end();
        envFbo[i].end();
        if(textureFormat == GL_RGB32F){
            ofFloatPixels pix;
            envFbo[i].readToPixels(pix);
            fTex.cubeMapFaces[i].setFromPixels(pix);
        }else{
            ofPixels pix;
            envFbo[i].readToPixels(pix);
            iTex.cubeMapFaces[i].setFromPixels(pix);
        }
    }
    
    ofPopStyle();
    ofDisableDepthTest();
    
    makeRawCubeMap();
    
    ofPushStyle();
    ofEnableDepthTest();
    
    // render filtered cubemap faces
    for(int i=0; i<6; i++){
        int width = baseSize;
        int height = baseSize;
        for(int j=0; j<maxMipLevel; j++){
            envFbo[i].allocate(width, height, textureFormat);
            envFbo[i].begin();
            ofClear(255);
            
            envCam[i].begin();
            
            glActiveTexture( GL_TEXTURE0 + 1 );
            glEnable( GL_TEXTURE_CUBE_MAP );
            glBindTexture( GL_TEXTURE_CUBE_MAP, cubeMapID );
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            
            shader.begin();
            shader.setUniform1i("envMap", 1);
			shader.setUniform1f("faceResolution", baseSize);
            shader.setUniform1f("Roughness", ofMap(j, 0, maxMipLevel-1, 0.0, 1.0 ));
            sphereMesh.draw();
            shader.end();
            
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0 );
            glDisable( GL_TEXTURE_CUBE_MAP );
            glActiveTexture( GL_TEXTURE0 );
            
            envCam[i].end();
            
            envFbo[i].end();
            
            if(textureFormat == GL_RGB32F){
                ofFloatPixels pix;
                envFbo[i].readToPixels(pix);
                fTex.filteredCubeMapFaces[i][j].setFromPixels(pix);
            }else{
                ofPixels pix;
                envFbo[i].readToPixels(pix);
                iTex.filteredCubeMapFaces[i][j].setFromPixels(pix);
            }
            width /= 2;
            height /= 2;
        }
    }
    ofPopStyle();
    
    makeFilteredCubeMap();

	ofEnableArbTex();
}

void ofxPBRCubeMap::makeRawCubeMap(){
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    glGenTextures(1, &cubeMapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapID);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    if(textureFormat == GL_RGB32F){
        int width = fTex.cubeMapFaces[0].getWidth();
        int height = fTex.cubeMapFaces[0].getHeight();
        
        makeCubeMapFaces(width, height,
                         fTex.cubeMapFaces[0].getPixels(),
                         fTex.cubeMapFaces[2].getPixels(),
                         fTex.cubeMapFaces[4].getPixels(),
                         fTex.cubeMapFaces[1].getPixels(),
                         fTex.cubeMapFaces[3].getPixels(),
                         fTex.cubeMapFaces[5].getPixels());
        
		for (int i = 0; i < 6; i++) {
			fTex.cubeMapFaces[i].clear();
		}
		fTex.rawTexture.resize(baseSize, baseSize * 0.5);
	} else {
		int width = iTex.cubeMapFaces[0].getWidth();
        int height = iTex.cubeMapFaces[0].getHeight();

        makeCubeMapFaces(width, height,
                         iTex.cubeMapFaces[0].getPixels(),
                         iTex.cubeMapFaces[2].getPixels(),
                         iTex.cubeMapFaces[4].getPixels(),
                         iTex.cubeMapFaces[1].getPixels(),
                         iTex.cubeMapFaces[3].getPixels(),
                         iTex.cubeMapFaces[5].getPixels());
        
		for (int i = 0; i < 6; i++) {
			iTex.cubeMapFaces[i].clear();
		}
		iTex.rawTexture.resize(baseSize, baseSize * 0.5);
	}
    
    glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0);
}

void ofxPBRCubeMap::makeFilteredCubeMap(){
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    glGenTextures(1, &filteredCubeMapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, filteredCubeMapID);
    
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, maxMipLevel - 1);
    
    for(int i=0; i<maxMipLevel; i++){
        if(textureFormat == GL_RGB32F){
            int width = fTex.filteredCubeMapFaces[0][i].getWidth();
            int height = fTex.filteredCubeMapFaces[0][i].getHeight();
            makeCubeMapFaces(width, height,
                             fTex.filteredCubeMapFaces[0][i].getPixels(),
                             fTex.filteredCubeMapFaces[2][i].getPixels(),
                             fTex.filteredCubeMapFaces[4][i].getPixels(),
                             fTex.filteredCubeMapFaces[1][i].getPixels(),
                             fTex.filteredCubeMapFaces[3][i].getPixels(),
                             fTex.filteredCubeMapFaces[5][i].getPixels(),
                             i);
        } else {
            int width = iTex.filteredCubeMapFaces[0][i].getWidth();
            int height = iTex.filteredCubeMapFaces[0][i].getHeight();
            makeCubeMapFaces(width, height,
                             iTex.filteredCubeMapFaces[0][i].getPixels(),
                             iTex.filteredCubeMapFaces[2][i].getPixels(),
                             iTex.filteredCubeMapFaces[4][i].getPixels(),
                             iTex.filteredCubeMapFaces[1][i].getPixels(),
                             iTex.filteredCubeMapFaces[3][i].getPixels(),
                             iTex.filteredCubeMapFaces[5][i].getPixels(),
                             i);
		}
    }
    
    glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0);
}

void ofxPBRCubeMap::makeCubeMapFaces(int width, int height,
                                     ofPixels& px, ofPixels& py, ofPixels& pz,
                                     ofPixels& nx, ofPixels& ny, ofPixels& nz,
                                     int index){
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, index, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, px.getData()); // positive x
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, index, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, py.getData()); // positive y
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, index, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pz.getData()); // positive z
    
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, index, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nx.getData()); // negative x
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, index, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, ny.getData()); // negative y
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, index, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nz.getData()); // negative z
}

void ofxPBRCubeMap::makeCubeMapFaces(int width, int height,
                                     ofFloatPixels& px, ofFloatPixels& py, ofFloatPixels& pz,
                                     ofFloatPixels& nx, ofFloatPixels& ny, ofFloatPixels& nz,
                                     int index){
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, index, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, px.getData()); // positive x
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, index, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, py.getData()); // positive y
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, index, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, pz.getData()); // positive z
    
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, index, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, nx.getData()); // negative x
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, index, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, ny.getData()); // negative y
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, index, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, nz.getData()); // negative z
}

void ofxPBRCubeMap::makeCache(string chachePath){
    int offsetX = 0;
    cacheFbo.begin();
    ofClear(0);
    if(textureFormat == GL_RGB32F){
        for(int i=0; i<maxMipLevel; i++){
            int texWidth = fTex.filteredCubeMapFaces[0][i].getWidth();
            int texHeight = fTex.filteredCubeMapFaces[0][i].getHeight();
            ofPushMatrix();
            if(i > 0){
                ofTranslate(offsetX, baseSize * 2);
                offsetX += texWidth * 3;
            }
            for(int j=0; j<6; j++){
                fTex.filteredCubeMapFaces[j][i].draw((j % 3) * texWidth, floor(j / 3) * texHeight, texWidth, texHeight);
            }
            ofPopMatrix();
        }
		fTex.rawTexture.draw(fTex.filteredCubeMapFaces[0][0].getWidth() * 1.5, fTex.filteredCubeMapFaces[0][0].getHeight() * 2.5, fTex.filteredCubeMapFaces[0][0].getWidth(), fTex.filteredCubeMapFaces[0][0].getHeight() * 0.5);
    }else{
        for(int i=0; i<maxMipLevel; i++){
            int texWidth = iTex.filteredCubeMapFaces[0][i].getWidth();
            int texHeight = iTex.filteredCubeMapFaces[0][i].getHeight();
            ofPushMatrix();
            if(i > 0){
                ofTranslate(offsetX, baseSize * 2);
                offsetX += texWidth * 3;
            }
            for(int j=0; j<6; j++){
                iTex.filteredCubeMapFaces[j][i].draw((j % 3) * texWidth, floor(j / 3) * texHeight, texWidth, texHeight);
            }
            ofPopMatrix();
        }
		iTex.rawTexture.draw(iTex.filteredCubeMapFaces[0][0].getWidth() * 1.5, iTex.filteredCubeMapFaces[0][0].getHeight() * 2.5, iTex.filteredCubeMapFaces[0][0].getWidth(), iTex.filteredCubeMapFaces[0][0].getHeight() * 0.5);
    }
    
    cacheFbo.end();
    if(textureFormat == GL_RGB32F){
        ofFloatImage img;
        ofFloatPixels pix;
        cacheFbo.getTexture().readToPixels(pix);
        img.setFromPixels(pix);
        img.save(chachePath);
    }else{
        ofImage img;
        ofPixels pix;
        cacheFbo.getTexture().readToPixels(pix);
        img.setFromPixels(pix);
        img.save(chachePath);
    }

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < maxMipLevel; j++) {
			if (textureFormat == GL_RGB32F) {
				fTex.filteredCubeMapFaces[i][j].clear();
			} else {
				iTex.filteredCubeMapFaces[i][j].clear();
			}
		}
	}
}

bool ofxPBRCubeMap::isHDRImagePath(string path){
    if(path.find(".hdr", 0) != string::npos ||
       path.find(".exr", 0) != string::npos){
        return true;
    } else {
        return false;
    }
}

void ofxPBRCubeMap::bind(int pos){
    textureUnit = pos;
    glActiveTexture( GL_TEXTURE0 + pos );
    glEnable( GL_TEXTURE_CUBE_MAP );
    glBindTexture( GL_TEXTURE_CUBE_MAP, filteredCubeMapID );
}

void ofxPBRCubeMap::unbind(){
    glActiveTexture( GL_TEXTURE0 + textureUnit );
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0 );
    glDisable( GL_TEXTURE_CUBE_MAP );
    glActiveTexture( GL_TEXTURE0 );
}

bool ofxPBRCubeMap::isHDR(){
    if(textureFormat == GL_RGB32F){
        return true;
    } else {
        return false;
    }
}

bool ofxPBRCubeMap::isAllocated()
{
	return bIsAllocated;
}

int ofxPBRCubeMap::getNumMips(){
    return maxMipLevel - 1;
}

ofTexture * ofxPBRCubeMap::getPanoramaTexture()
{
	if (isHDR()) {
		return &fTex.rawTexture.getTexture();
	} else {
		return &iTex.rawTexture.getTexture();
	}
}

template<>
ofImage* ofxPBRCubeMap::getPanorama(){
    return &iTex.rawTexture;
}

template<>
ofFloatImage* ofxPBRCubeMap::getPanorama(){
    return &fTex.rawTexture;
}

ofFloatColor ofxPBRCubeMap::getColor(int x, int y)
{
	if (isHDR()) {
		return fTex.rawTexture.getColor(x, y);
    } else {
		return iTex.rawTexture.getColor(x, y);
	}
}