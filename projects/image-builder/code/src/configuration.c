#include "image-builder/configuration.h"

// TODO: Move default LBA size to 4096 , seems better for performone on disks in
// this day and age
// And have it be dependent on or set it manually
// static U64 physicalBlockBoundary = 512;
// static U64 optimalTransferLengthGranularity = 512;
Configuration configuration = {.imageName = "new-image.hdd", .LBASize = 1024};
