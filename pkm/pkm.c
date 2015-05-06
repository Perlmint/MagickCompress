// Ŭnicode please
#include "pkm_inner.h"

static MagickBooleanType IsPKM(const unsigned char *magick, const size_t length)
{
	if (length < 16)
	{
		return(MagickFalse);
	}
	if (LocaleNCompare((char *)magick, "PKM ", 4) == 0)
	{
		return(MagickTrue);
	}
	return(MagickFalse);
}

static Image *ReadPKMImage(
  const ImageInfo *image_info, ExceptionInfo *exception)
{
	return ReadPKMImage_inner(image_info, exception, MagickFalse);
}

static MagickBooleanType
  WritePKMImage(const ImageInfo *image_info, Image *image)
{
	return WritePKMImage_inner(image_info, image, MagickFalse);
}

static Image *ReadPKMAImage(
	const ImageInfo *image_info, ExceptionInfo *exception)
{
	return ReadPKMImage_inner(image_info, exception, MagickTrue);
}

static MagickBooleanType
WritePKMAImage(const ImageInfo *image_info, Image *image)
{
	return WritePKMImage_inner(image_info, image, MagickTrue);
}

ModuleExport unsigned long RegisterPKMImage(void)
{
	MagickInfo *entry;

	entry = SetMagickInfo("PKM");
	entry->encoder = (EncodeImageHandler *)WritePKMImage;
	entry->decoder = (DecodeImageHandler *)ReadPKMImage;
	entry->magick = (IsImageFormatHandler *)IsPKM;
	entry->description = ConstantString("PKM");
	entry->module = ConstantString("PKM");

	(void)RegisterMagickInfo(entry);

	entry = SetMagickInfo("PKMA");
	entry->encoder = (EncodeImageHandler *)WritePKMAImage;
	entry->decoder = (DecodeImageHandler *)ReadPKMAImage;
	entry->magick = (IsImageFormatHandler *)IsPKM;
	entry->description = ConstantString("PKM_alpha");
	entry->module = ConstantString("PKM");

	(void)RegisterMagickInfo(entry);
	return (MagickImageCoderSignature);
}

ModuleExport void UnregisterPKMImage(void)
{
	(void)UnregisterMagickInfo("PKM");
}