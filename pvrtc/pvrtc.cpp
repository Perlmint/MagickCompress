// Ŭnicode please
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
#include <stdint.h>

#include "PvrTcDecoder.h"
#include "PvrTcEncoder.h"
#include "Bitmap.h"
#include "RgbaBitmap.h"

typedef struct PvrHeader_t
{
	static const unsigned char version[];

	enum Flag
	{
		NoFlag = 0,
		PreMultiplied = 0x02
	} flags;

	enum PixelFormat
	{
		PVRTC_2BPP_RGB = 0,
		PVRTC_2BPP_RGBA = 1,
		PVRTC_4BPP_RGB = 2,
		PVRTC_4BPP_RGBA = 3,
		PVRTC2_2BPP = 4,
		PVRTC2_4BPP = 5,
		ETC1 = 6,
		DXT1 = 7,
		DXT2 = 8,
		DXT3 = 9,
		DXT4 = 10,
		DXT5 = 11,
		BC1 = 7,
		BC2 = 9,
		BC3 = 11,
		BC4 = 12,
		BC5 = 13,
		BC6 = 14,
		BC7 = 15,
		UYVY = 16,
		YUY2 = 17,
		BW_1BPP = 18,
		R9G9B9E5 = 19,
		RGBG8888 = 20,
		GRGB8888 = 21,
		ETC2_RGB = 22,
		ETC2_RGBA = 23,
		ETC2_RGB_A1 = 24,
		EAC_R11U = 25,
		EAC_R11S = 26,
		EAC_RG11U = 27,
		EAC_RG11S = 28
	} pixelFormat;

	enum ColourSpace
	{
		LINEAR_RGB = 0,
		SRGB = 1
	} colourSpace;

	enum ChannelType
	{
		UNSIGNED_BYTE_NORMALISED = 0,
		SIGNED_BYTE_NORMALISED = 1,
		UNSIGNED_BYTE = 2,
		SIGNED_BYTE = 3,
		UNSIGNED_SHORT_NORMALISED = 4,
		SIGNED_SHORT_NORMALISED = 5,
		UNSIGNED_SHORT = 6,
		SIGNED_SHORT = 7,
		UNSIGNED_INTEGER_NORMALISED = 8,
		SIGNED_INTEGER_NORMALISED = 9,
		UNSIGNED_INTEGER = 10,
		SIGNED_INTEGER = 11,
		FLOAT = 12
	} channelType;

	uint32_t height;
	uint32_t width;
	uint32_t depth;
	uint32_t surfacesCount;
	uint32_t facesCount;
	uint32_t mipmapCount;
	uint32_t metaDataSize;
} PvrHeader;

const unsigned char PvrHeader::version[] = "PVR\x3";

static void PVRTCHeaderInit(Image *image, PvrHeader *header)
{
	header->width = image->columns;
	header->height = image->rows;
	header->flags = PvrHeader::NoFlag;
	header->colourSpace = PvrHeader::LINEAR_RGB;
	header->channelType = PvrHeader::UNSIGNED_BYTE_NORMALISED;
	header->depth = 1;
	header->surfacesCount = 1;
	header->facesCount = 1;
	header->mipmapCount = 1;
	header->metaDataSize = 0;
}

ssize_t WriteBlobLSBLongLong(Image *image,
	const MagickSizeType value)
{
	unsigned char
		buffer[8];

	assert(image != (Image *)NULL);
	assert(image->signature == MagickSignature);
	buffer[0] = (unsigned char)(value >> 0);
	buffer[1] = (unsigned char)(value >> 8);
	buffer[2] = (unsigned char)(value >> 16);
	buffer[3] = (unsigned char)(value >> 24);
	buffer[4] = (unsigned char)(value >> 32);
	buffer[5] = (unsigned char)(value >> 40);
	buffer[6] = (unsigned char)(value >> 48);
	buffer[7] = (unsigned char)(value >> 56);
	return(WriteBlob(image, 8, buffer));
}

static MagickBooleanType WritePVRTCHeader(Image *image, PvrHeader *header)
{
	WriteBlob(image, 4, PvrHeader::version);
	WriteBlobLSBLong(image, header->flags);
	WriteBlobLSBLongLong(image, header->pixelFormat);
	WriteBlobLSBLong(image, header->colourSpace);
	WriteBlobLSBLong(image, header->channelType);
	WriteBlobLSBLong(image, header->height);
	WriteBlobLSBLong(image, header->width);
	WriteBlobLSBLong(image, header->depth);
	WriteBlobLSBLong(image, header->surfacesCount);
	WriteBlobLSBLong(image, header->facesCount);
	WriteBlobLSBLong(image, header->mipmapCount);
	WriteBlobLSBLong(image, header->metaDataSize);

	return MagickTrue;
}

static void ReadPVRTCHeader(Image *image, PvrHeader *header)
{
	header->flags = (PvrHeader::Flag)ReadBlobLSBLong(image);
	header->pixelFormat = (PvrHeader::PixelFormat)ReadBlobLongLong(image);
	header->colourSpace = (PvrHeader::ColourSpace)ReadBlobLSBLong(image);
	header->channelType = (PvrHeader::ChannelType)ReadBlobLSBLong(image);
	header->height = ReadBlobLSBLong(image);
	header->width = ReadBlobLSBLong(image);
	header->depth = ReadBlobLSBLong(image);
	header->surfacesCount = ReadBlobLSBLong(image);
	header->facesCount = ReadBlobLSBLong(image);
	header->mipmapCount = ReadBlobLSBLong(image);
	header->metaDataSize = ReadBlobLSBLong(image);
	if (header->metaDataSize > 0)
	{
		DiscardBlobBytes(image, (MagickSizeType)header->metaDataSize);
	}
}

static MagickBooleanType IsPVRTC(const unsigned char *magick, const size_t length)
{
	if (length < 16)
	{
		return(MagickFalse);
	}
	if (LocaleNCompare((char *)magick, "PVR\x3", 4) == 0)
	{
		return(MagickTrue);
	}
	return(MagickFalse);
}

static Image *ReadPVRTCImage(
	const ImageInfo *image_info, ExceptionInfo *exception)
{
	unsigned char buf[8];

	Image *image;

	MagickBooleanType status;

	PixelPacket *q;

	Quantum maxQ = ScaleCharToQuantum(255);

	unsigned char *dataPtr;
	Javelin::Point2<int> image_size(0, 0);

	assert(image_info != (const ImageInfo *)NULL);
	assert(image_info->signature == MagickSignature);
	if (image_info->debug != MagickFalse)
		(void)LogMagickEvent(TraceEvent, GetMagickModule(), "%s",
		image_info->filename);
	assert(exception != (ExceptionInfo *)NULL);
	assert(exception->signature == MagickSignature);
	image = AcquireImage(image_info);
	status = OpenBlob(image_info, image, ReadBinaryBlobMode, exception);
	image->endian = LSBEndian;
	if (status == MagickFalse)
	{
		goto ErrorExit;
	}

	ReadBlob(image, 4, buf);
	if (memcmp(buf, "PVR\x3", 4) != 0)
	{
		ThrowReaderException(CorruptImageFatalError, "Wrong header");
		goto ErrorExit;
	}

	PvrHeader header;

	ReadPVRTCHeader(image, &header);

	image_size.x = header.width;
	image_size.y = header.height;
	image->columns = header.width;
	image->rows = header.height;

	q = QueueAuthenticPixels(image, 0, 0, header.width, header.height, exception);
	if (header.pixelFormat == PvrHeader::PVRTC_4BPP_RGBA)
	{
		strncpy(image->magick, "PVRTC4BPPRGBA", 13);
		size_t dataSize = sizeof(unsigned char) * image->columns / 2 * image->rows;
		dataPtr = (unsigned char *)malloc(dataSize);
		ReadBlob(image, dataSize, dataPtr);
		  Javelin::ColorRgba<unsigned char > *result = (Javelin::ColorRgba<unsigned char> *)malloc(sizeof(Javelin::ColorRgba<unsigned char>) * (image->columns * image->rows));
		Javelin::PvrTcDecoder::DecodeRgba4Bpp(result, image_size, dataPtr);
		Javelin::ColorRgba<unsigned char > *ptr = result;
		for (size_t i = 0, s = header.width * header.height; i < s; ++i)
		{
			q->red = ScaleCharToQuantum(ptr->r);
			q->green = ScaleCharToQuantum(ptr->g);
			q->opacity = ScaleCharToQuantum(ptr->a);
			++ptr;
			++q;
		}
	}
	else if (header.pixelFormat == PvrHeader::PVRTC_4BPP_RGB)
	{
		strncpy(image->magick, "PVRTC4BPPRGB", 12);
		size_t dataSize = sizeof(unsigned char) * image->columns / 2 * image->rows;
		dataPtr = (unsigned char *)malloc(dataSize);
		ReadBlob(image, dataSize, dataPtr);
		  Javelin::ColorRgb<unsigned char> *result = (Javelin::ColorRgb<unsigned char> *)malloc(sizeof(Javelin::ColorRgba<unsigned char>) * image->columns * image->rows);
		Javelin::PvrTcDecoder::DecodeRgb4Bpp(result, image_size, dataPtr);
		Javelin::ColorRgb<unsigned char> *ptr = result;
		for (size_t i = 0, s = header.width * header.height; i < s; ++i)
		{
			q->red = ScaleCharToQuantum(ptr->r);
			q->green = ScaleCharToQuantum(ptr->g);
			q->blue = ScaleCharToQuantum(ptr->b);
			q->opacity = maxQ;
			++ptr;
			++q;
		}
		  free(result);
	}
	else if (header.pixelFormat == PvrHeader::PVRTC_2BPP_RGB)
	{
		strncpy(image->magick, "PVRTC2BPPA", 11);
	}

	CloseBlob(image);
	return image;

ErrorExit:
	image = DestroyImageList(image);
	return((Image *)NULL);
}

static MagickBooleanType
WritePVRTCImage(const ImageInfo *image_info, Image *image)
{
	MagickBooleanType
		status;

	register const PixelPacket
		*p;

	register ssize_t
		x;

	ssize_t
		y;

	PvrHeader header;

	unsigned int i, j;

	  Javelin::RgbaBitmap *bitmap = (Javelin::RgbaBitmap *)NULL;
	unsigned char *dataPtr;

	/*
	Open output image file.
	*/
	assert(image_info != (const ImageInfo *)NULL);
	assert(image_info->signature == MagickSignature);
	assert(image != (Image *)NULL);
	assert(image->signature == MagickSignature);
	if (image->columns != image->rows)
	{
		ThrowWriterException(CorruptImageError, "image is not a square image.");
		return MagickFalse;
	}
	
	for (i = 1, j = 0; i <= image->columns; i = i << 1)
	{
		if ((image->columns & i) != 0)
		{
			++j;
		}
	}

	if (j != 1)
	{
		ThrowWriterException(CorruptImageError, "image is not a POT image.");
		return MagickFalse;
	}

	bitmap = new Javelin::RgbaBitmap(image->columns, image->rows);
	
	image->endian = LSBEndian;
	if (image->debug != MagickFalse)
	{
		(void)LogMagickEvent(TraceEvent, GetMagickModule(), "%s", image->filename);
	}
	status = OpenBlob(image_info, image, WriteBinaryBlobMode, &image->exception);
	if (status == MagickFalse)
	{
		ThrowWriterException(CorruptImageError, "Open blob failed");
		return(status);
	}

	Javelin::ColorRgba<unsigned char> *colorPtr = bitmap->GetData();
	for (y = 0; y < (ssize_t)image->rows; ++y)
	{
		p = GetVirtualPixels(image, 0, y, image->columns, 1, &image->exception);
		for (x = 0; x < (ssize_t)image->columns; ++x)
		{
			colorPtr->r = ScaleQuantumToChar(p->red);
			colorPtr->g = ScaleQuantumToChar(p->green);
			colorPtr->b = ScaleQuantumToChar(p->blue);
			colorPtr->a = 255 - ScaleQuantumToChar(p->opacity);
			++colorPtr;
			++p;
		}
	}

	PVRTCHeaderInit(image, &header);

	size_t dataSize = 0;

	if (strcmp(image_info->magick, "PVRTC4BPPRGBA") == 0)
	{
		header.pixelFormat = PvrHeader::PVRTC_4BPP_RGBA;
		dataSize = sizeof(unsigned char) * bitmap->GetArea() / 2;
		dataPtr = (unsigned char *)malloc(dataSize);
		memset(dataPtr, 0, dataSize);
		Javelin::PvrTcEncoder::EncodeRgba4Bpp(dataPtr, *bitmap);
	}
	else if (strcmp(image_info->magick, "PVRTC4BPPRGB") == 0)
	{
		header.pixelFormat = PvrHeader::PVRTC_4BPP_RGB;
		dataSize = sizeof(unsigned char) * bitmap->GetArea() / 2;
		dataPtr = (unsigned char *)malloc(dataSize);
		memset(dataPtr, 0, dataSize);
		Javelin::PvrTcEncoder::EncodeRgb4Bpp(dataPtr, *bitmap);
	}
	else if (strcmp(image_info->magick, "PVRTC2BPPA") == 0)
	{
		header.pixelFormat = PvrHeader::PVRTC_2BPP_RGB;
		dataSize = sizeof(unsigned char) * bitmap->GetArea() / 4;
		dataPtr = (unsigned char *)malloc(dataSize);
		memset(dataPtr, 0, dataSize);
		Javelin::PvrTcEncoder::EncodeAlpha2Bpp(dataPtr, *bitmap);
	}
	else if (strcmp(image_info->magick, "PVRTC4BPPA") == 0)
	{
		header.pixelFormat = PvrHeader::PVRTC_4BPP_RGB;
		dataSize = sizeof(unsigned char) * bitmap->GetArea() / 2;
		dataPtr = (unsigned char *)malloc(dataSize);
		memset(dataPtr, 0, dataSize);
		Javelin::PvrTcEncoder::EncodeAlpha4Bpp(dataPtr, *bitmap);
	}

	WritePVRTCHeader(image, &header);

	WriteBlob(image, dataSize, dataPtr);
	free(dataPtr);
	CloseBlob(image);

	  free(bitmap);
	return MagickTrue;
}

extern "C" ModuleExport void UnregisterPVRTCImage(void)
{
	(void)UnregisterMagickInfo("PVRTC");
}

extern "C" ModuleExport unsigned long RegisterPVRTCImage(void)
{
	MagickInfo *entry;

	entry = SetMagickInfo("PVRTC4BPPRGBA");
	entry->encoder = (EncodeImageHandler *)WritePVRTCImage;
	entry->decoder = (DecodeImageHandler *)ReadPVRTCImage;
	entry->magick = (IsImageFormatHandler *)IsPVRTC;
	entry->description = ConstantString("PVRTC 4bpp rgba");
	entry->module = ConstantString("PVRTC");
	(void)RegisterMagickInfo(entry);

	entry = SetMagickInfo("PVRTC4BPPRGB");
	entry->encoder = (EncodeImageHandler *)WritePVRTCImage;
	entry->decoder = (DecodeImageHandler *)ReadPVRTCImage;
	entry->magick = (IsImageFormatHandler *)IsPVRTC;
	entry->description = ConstantString("PVRTC 4bpp rgb");
	entry->module = ConstantString("PVRTC");
	(void)RegisterMagickInfo(entry);

	entry = SetMagickInfo("PVRTC2BPPA");
	entry->encoder = (EncodeImageHandler *)WritePVRTCImage;
	entry->decoder = (DecodeImageHandler *)ReadPVRTCImage;
	entry->magick = (IsImageFormatHandler *)IsPVRTC;
	entry->description = ConstantString("PVRTC 2bpp a");
	entry->module = ConstantString("PVRTC");
	(void)RegisterMagickInfo(entry);

	entry = SetMagickInfo("PVRTC4BPPA");
	entry->encoder = (EncodeImageHandler *)WritePVRTCImage;
	entry->decoder = (DecodeImageHandler *)ReadPVRTCImage;
	entry->magick = (IsImageFormatHandler *)IsPVRTC;
	entry->description = ConstantString("PVRTC 4bpp a");
	entry->module = ConstantString("PVRTC");
	(void)RegisterMagickInfo(entry);

	entry = SetMagickInfo("PVR");
	entry->decoder = (DecodeImageHandler *)ReadPVRTCImage;
	entry->magick = (IsImageFormatHandler *)IsPVRTC;
	entry->description = ConstantString("PVRTC");
	entry->module = ConstantString("PVRTC");

	(void)RegisterMagickInfo(entry);
	return (MagickImageCoderSignature);
}
