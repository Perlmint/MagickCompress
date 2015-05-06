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

References
----------
  * [rg-etc1](https://code.google.com/p/rg-etc1/)
  * [pvrtccompressor](https://bitbucket.org/jthlim/pvrtccompressor)