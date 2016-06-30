#pragma once
#include "ofMain.h"
#include "shaders/importanceSampling.h"

class ofxPBRCubeMap{
private:
    ofCamera envCam[6];
    ofFbo envFbo[6];
    ofTexture envTexture;
    
    ofImage iEnv;
    ofImage iEnvMapImages[6];
    vector <ofImage> iFilteredImages[6];
    
    ofFloatImage fEnv;
    ofFloatImage fEnvMapImages[6];
    vector <ofFloatImage> fFilteredImages[6];
    
    ofShader shader;
    ImportanceSampling importanceSampling;
    ofMesh sphereMesh, envSphereMesh;
    unsigned int cubeMapID;
    unsigned int filteredCubeMapID;
    int textureUnit;
    int baseSize;
    
    int cacheWidth, cacheHeight;
    ofFbo cacheFbo;
    
    int textureFormat;
    ofImage iCacheImage;
    ofFloatImage fCacheImage;
    
    ofMesh skyboxFaces[6];
    
    int maxMipLevel;

	bool bIsAllocated = false;

	void loadShaders();
    void makeCubeMapTextures();
    void makeCubeMap();
    void makeFilteredCubeMap();
    void makeCache(string cachePath);
    void makeCube();

	float rotation = 0.0;
	float exposure = 1.0;
	float envLevel = 0.0;
    
public:
    ofxPBRCubeMap();
    
    void load(ofImage * sphereMapImage, int baseSize = 512);
    void load(ofFloatImage * sphereMapImage, int baseSize = 512);
    void load(string imagePath, int baseSize = 512, bool useCache = false, string cacheDirectry = "");
    void loadFromCache(string cachePath);
    void bind(int pos);
    void unbind();
    bool isHDR();
	bool isAllocated();
    int getNumMips();
	ofTexture* getPanoramaTexture();
	ofImage* getPanoramaImage();
	ofFloatImage* getFloatPanoramaImage();
	void fetchPanoramaImage(ofImage* img);
	void fetchPanoramaImage(ofFloatImage* img);
	ofFloatColor getColor(int x, int y);

	void setRotation(float rotation) { this->rotation = rotation; };
	float getRotation() { return rotation; };
	void setExposure(float exposure) { this->exposure = exposure; };
	float getExposure() { return exposure; };
	void setEnvLevel(float level) { this->envLevel = level; };
	float getEnvLevel() { return envLevel; };
};