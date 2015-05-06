ImageMagickTextureCompressModules
=================================

Texture compress modules for ImageMagick

Supported compression format
----------------------------
  * etc1
  * pvrtc-4bpp-rgb
  * pvrtc-4bpp-rgba

ImageMagick format names
------------------------
  * pkm
    * compress with etc1
	* save as pkm format
  * pkma
    * compress alpha as grayscale etc1
	* save as pkm format
  * pvrtc
    * read only alias
	* can identify pvrtc-4bpp-rgba / pvrtc-4bpp-rgb
  * pvrtc4bpprgba
  	* compress RGBA with pvrtc 4bits-per-pixel
	* save as pvr format
  * pvrtc4bpprgb
    * compress RGB with pvrtc 4bits-per-pixel
	* save as pvr format
	* discard alpha channel

How to build
------------
### Windows module
1. download [ImageMagick-windows.zip](http://www.imagemagick.org/script/install-source.php#windows) and unzip into ext as `ImageMagick`
2. run `configure.exe` in `ext/ImageMagick/VisualMagick/configure`
    * At this time, you can select 64-bit / 32-bit
3. build visual studio solution `ImageMagickTextureCompressModules.sln`

### OSX module
1. prerequisite
    * glibtool
2. copy magick headers into ext
3. copy libMagickCore-6.Q16.2.dylib into ext
4. build with `Makefile`

References
----------
  * [rg-etc1](https://code.google.com/p/rg-etc1/)
  * [pvrtccompressor](https://bitbucket.org/jthlim/pvrtccompressor)
