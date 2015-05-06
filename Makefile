MAGICK_CORE_NAME=MagickCore-6.Q16
IGNORE_WARN=-Wdeprecated-register
CL_FLAGS=-DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16 -DHAVE_CONFIG_H -I./ext -O2 $(IGNORE_WARN) -g
LD_FLAGS=-O2 -L./ext -no-undefined -module -avoid-version -rpath /coder -l$(MAGICK_CORE_NAME) -lstdc++

PKM_OBJS=pkm/pkm.c.lo pkm/pkma.c.lo pkm/etc1_comp.cpp.lo pkm/pkm_inner.cpp.lo
PVRTC_OBJS=pvrtc/pvrtc.cpp.lo pvrtc/BitScale.cpp.lo pvrtc/MortonTable.cpp.lo pvrtc/PvrTcDecoder.cpp.lo pvrtc/PvrTcEncoder.cpp.lo pvrt/PvrTcPacket.cpp.lo

all: pkm pvrtc

pkm/pkm.c.lo: pkm/pkm.c
pkm/pkma.c.lo: pkm/pkma.c
pkm/etc1_comp.cpp.lo: pkm/etc1_comp.cpp
pkm/pkm_inner.cpp.lo: pkm/pkm_inner.cpp

pkm: pkm.so
pkm.so: $(PKM_OBJS)

pvrtc/pvrtc.cpp.lo: pvrtc/pvrtc.cpp
pvrtc/BitScale.cpp.lo: pvrtc/BitScale.cpp
pvrtc/MortonTable.cpp.lo: pvrtc/MortonTable.cpp
pvrtc/PvrTcDecoder.cpp.lo: pvrtc/PvrTcDecoder.cpp
pvrtc/PvrTcEncoder.cpp.lo: pvrtc/PvrTcEncoder.cpp
pvrt/PvrTcPacket.cpp.lo: pvrtc/PvrTcPacket.cpp

pvrtc: pvrtc.so
pvrtc.so: $(PVRTC_OBJS)

%.cpp.lo:
	glibtool --tag=CC --mode=compile gcc $(CL_FLAGS) -MT $@ -MD -MP -c -o $@ $<
%.c.lo:
	glibtool --tag=CC --mode=compile gcc $(CL_FLAGS) -MT $@ -MD -MP -c -o $@ $<
%.so:
	glibtool --tag=CC --mode=link gcc $(LD_FLAGS) -o $*.la $^
	install_name_tool -change /usr/local/lib/lib$(MAGICK_CORE_NAME).2.dylib ./lib$(MAGICK_CORE_NAME).dylib .libs/$@


clean:
	rm -rf .libs
	rm -rf $(PKM_OBJS) $(PVRTC_OBJS)