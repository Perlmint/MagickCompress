// Ŭnicode please
#pragma once

#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/colorspace.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include <math.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC ModuleExport MagickBooleanType WritePKMImage_inner(const ImageInfo *image_info, Image *image, MagickBooleanType alphaMode);
EXTERNC ModuleExport Image *ReadPKMImage_inner(const ImageInfo *image_info, ExceptionInfo *exception, MagickBooleanType alphaMode);