#pragma once

typedef unsigned int uint;
typedef long unsigned int luint;

struct DirStat{
	luint size;
	luint realSize;
	uint fileCount;
	uint dirCount;
	luint innerSize;
};