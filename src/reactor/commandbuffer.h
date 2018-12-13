#pragma once
#include "reactor.h"

VkCommandBuffer beginSingleTimeCommands(rEngine& engine);

void endSingleTimeCommands(rEngine& engine, VkCommandBuffer commandBuffer); 
