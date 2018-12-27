#pragma once

#include "reactor.h"

struct rImage;
void rImageCreate(rImage& image, string location);

struct rImage {

	rEngine* engine;

	int width;
	int height;
	int depth;


	VkImage image;
	VkImageView imageView;
	VkSampler sampler;

	rImage() {}
	rImage(rEngine& inEngine, string path)
	{
		engine = &inEngine;
		rImageCreate(*this, path);
	}

};
