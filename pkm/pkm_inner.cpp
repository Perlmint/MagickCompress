// Ŭnicode please
#include "pkm_inner.h"
#include "etc1_comp.h"

template <typename T1, typename T2>
inline T1 MIN(const T1 &a, const T2 &b)
{
    if (a < b)
    {
      return a;
    }
    return b;
}

static unsigned int make4(unsigned int val)
{
	unsigned int remain = val % 4;
	if (remain == 0)
	{
		return val;
	}
	return val + 4 - remain;
}

MagickBooleanType WritePKMImage_inner(const ImageInfo *image_info, Image *image, MagickBooleanType alphaMode)
{
	MagickBooleanType
		status;

	register const PixelPacket
		*p;

	register size_t
		x;

	size_t
		y;

	rg_etc1::etc1_pack_params param;
	param.m_dithering = false;
	param.m_quality = rg_etc1::cLowQuality;

	rg_etc1::pack_etc1_block_init();

	/*
	Open output image file.
	*/
	assert(image_info != (const ImageInfo *)NULL);
	assert(image_info->signature == MagickSignature);
	assert(image != (Image *)NULL);
	assert(image->signature == MagickSignature);
	image->endian = MSBEndian;
	if (image->debug != MagickFalse)
		(void)LogMagickEvent(TraceEvent, GetMagickModule(), "%s", image->filename);
	status = OpenBlob(image_info, image, WriteBinaryBlobMode, &image->exception);
	if (status == MagickFalse)
		return(status);

	// Write Header
	// MAGIC - 4 bytes
	WriteBlobString(image, "PKM ");
	// Version - 2 bytes
	WriteBlobString(image, "10");
	// data type - 2 bytes
	WriteBlobByte(image, 0);
	WriteBlobByte(image, 0);
	// Extended Width - 16bit, big endian
	WriteBlobShort(image, make4(image->columns));
	// Extended Height - 16bit, big endian
	WriteBlobShort(image, make4(image->rows));
	// Image Width - 16bit, big endian
	WriteBlobShort(image, image->columns);
	// Image Height - 16bit, big endian
	WriteBlobShort(image, image->rows);

	unsigned char original_pixels[64];
	unsigned char *out_pixels = (unsigned char *)AcquireQuantumMemory(sizeof(unsigned char), 8);
	for (y = 0; y < (size_t)image->rows; y += 4)
	{
		for (x = 0; x < (size_t)image->columns; x += 4)
		{
			unsigned int w = MIN(image->columns - x, 4),
				h = MIN(image->rows - y, 4);
			p = GetVirtualPixels(image, x, y, w, h, &image->exception);
			memset(original_pixels, 0xFF, 64);
			if (p == (const PixelPacket *)NULL)
			{
				break;
			}

			for (unsigned i = 0; i < h; ++i)
			{
				for (unsigned j = 0; j < w; ++j)
				{
					unsigned int base = (i * 4 + j);
					if (alphaMode == MagickTrue)
					{
						original_pixels[base * 4 + 0] = 
						original_pixels[base * 4 + 1] = 
						original_pixels[base * 4 + 2] = 255 - ScaleQuantumToChar(p->opacity);
					}
					else
					{
						original_pixels[base * 4 + 0] = ScaleQuantumToChar(p->red);
						original_pixels[base * 4 + 1] = ScaleQuantumToChar(p->green);
						original_pixels[base * 4 + 2] = ScaleQuantumToChar(p->blue);
					}
					++p;
				}
			}

			rg_etc1::pack_etc1_block(out_pixels, reinterpret_cast<unsigned int *>(original_pixels), param);

			WriteBlob(image, 8, out_pixels);
			SetImageProgress(image, SaveImageTag, (y + h) * image->rows + (x + w) * 4, image->rows * image->columns);
		}
	}

	out_pixels = (unsigned char *)RelinquishMagickMemory(out_pixels);
	CloseBlob(image);
	return(MagickTrue);
}

union rgba {
	unsigned char arr[4];
	struct 
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	} e;
};

Image *ReadPKMImage_inner(const ImageInfo *image_info, ExceptionInfo *exception, MagickBooleanType alphaMode)
{
	unsigned char buf[8];

	Image *image;

	MagickBooleanType status;

	short width, height, extWidth, extHeight;

	rgba decoded[16];
	rgba *decoded_ptr;

	unsigned short x_block, y_block, x, y, x_target, y_target;

	PixelPacket *q;

	Quantum maxQ = ScaleCharToQuantum(255);

	assert(image_info != (const ImageInfo *)NULL);
	assert(image_info->signature == MagickSignature);
	if (image_info->debug != MagickFalse)
		(void)LogMagickEvent(TraceEvent, GetMagickModule(), "%s",
		image_info->filename);
	assert(exception != (ExceptionInfo *)NULL);
	assert(exception->signature == MagickSignature);
	image = AcquireImage(image_info);
	status = OpenBlob(image_info, image, ReadBinaryBlobMode, exception);
	image->endian = MSBEndian;
	image->depth = 8;
	if (status == MagickFalse)
	{
		goto ErrorExit;
	}

	// MAGIC - 4 bytes
	// Version - 2 bytes
	ReadBlob(image, 6, buf);
	if (memcmp(buf, "PKM 10", 6) != 0)
	{
		ThrowReaderException(CorruptImageFatalError, "Wrong header");
		goto ErrorExit;
	}
	// data type - 2 bytes
	DiscardBlobBytes(image, 2);
	// Extended Width - 16bit, big endian
	if ((extWidth = ReadBlobShort(image)) % 4 != 0)
	{
		ThrowReaderException(CorruptImageError, "Invalid extended width");
	}
	// Extended Height - 16bit, big endian
	if ((extHeight = ReadBlobShort(image)) % 4 != 0)
	{
		ThrowReaderException(CorruptImageError, "Invalid extended height");
	}
	// Image Width - 16bit, big endian
	width = ReadBlobShort(image);
	if (extWidth < width)
	{
		ThrowReaderException(CorruptImageError, "Invalid width. extended width is larger than content width");
		goto ErrorExit;
	}
	// Image Height - 16bit, big endian
	height = ReadBlobShort(image);
	if (extHeight < height)
	{
		ThrowReaderException(CorruptImageError, "Invalid height. extended height is larger than content height");
		goto ErrorExit;
	}

	image->depth = 8;
	if (SetImageExtent(image, width, height) == MagickFalse)
	{
		InheritException(exception, &image->exception);
		goto ErrorExit;
	}

	for (y_block = 0; y_block < extHeight; y_block += 4)
	{
		for (x_block = 0; x_block < extWidth; x_block += 4)
		{
			if (ReadBlob(image, 8, buf) != 8)
			{
				char buf[256];
				sprintf(buf, "Read Failed, at %lld\n%s", TellBlob(image), image->exception.description);
				ThrowReaderException(BlobFatalError, buf);
				goto ErrorExit;
			}
			if (!rg_etc1::unpack_etc1_block(buf, reinterpret_cast<unsigned int *>(decoded->arr), false))
			{
				char buf[256];
				sprintf(buf, "Unpack Failed, %u, %u, %u, %u", x_block, y_block, MIN(4, width - x_block), MIN(4, height - y_block));
				ThrowReaderException(CorruptImageError, buf);
				goto ErrorExit;
			}
			decoded_ptr = decoded;
			y_target = MIN(height, y_block + 4);
			x_target = MIN(width, x_block + 4);
			q = GetAuthenticPixels(image, x_block, y_block, x_target - x_block, y_target - y_block, exception);
			if (q == (PixelPacket *)NULL)
			{
				char buf[2048];
				sprintf(buf, "Failed to retrieve pixels, %u, %u, %u, %u\n%d - %s\n%s", x_block, y_block, x_target - x_block, y_target - y_block, exception->error_number, exception->reason, exception->description);
				ThrowReaderException(CorruptImageError, buf);
				goto ErrorExit;
			}
			for (y = y_block; y < y_target; ++y)
			{
				for (x = x_block; x < x_target; ++x)
				{
					if (alphaMode == MagickFalse)
					{
						q->red = ScaleCharToQuantum(decoded_ptr->e.r);
						q->green = ScaleCharToQuantum(decoded_ptr->e.g);
						q->blue = ScaleCharToQuantum(decoded_ptr->e.b);
						q->opacity = maxQ;
					}
					else
					{
						q->red = 
						q->green = 
						q->blue = maxQ;
						q->opacity = ScaleCharToQuantum(255 - decoded_ptr->e.r);
					}
					++decoded_ptr;
					++q;
				}
			}
			if (SyncAuthenticPixels(image, exception) == MagickFalse)
			{
				ThrowReaderException(CorruptImageError, "Pixel sync failed");
				goto ErrorExit;
			}
			q = NULL;
		}
	}

	CloseBlob(image);
	return image;

ErrorExit:
	image = DestroyImageList(image);
	return((Image *)NULL);
}
