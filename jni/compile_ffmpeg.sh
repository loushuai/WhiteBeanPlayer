git clone https://github.com/loushuai/FFmpeg-n3.1.3.git

cd FFmpeg-n3.1.3-master

NDK_ROOT=/Users/brain/Softwares/android-ndk-r9d
PLATFORM=arm-linux-androideabi-4.6
OS=darwin-x86_64

./configure \
    --arch=arm \
    --cpu=cortex-a8 \
    --target-os=linux \
    --enable-cross-compile \
    --enable-gpl \
    --enable-version3 \
    --enable-nonfree \
    --arch=armv7-a \
    --enable-pic \
    --enable-asm \
    --enable-neon \
    --disable-shared \
    --enable-static \
    --cross-prefix=$NDK_ROOT/toolchains/$PLATFORM/prebuilt/$OS/bin/arm-linux-androideabi- \
    --sysroot=$NDK_ROOT/platforms/android-9/arch-arm \
    --disable-programs \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-ffserver \
    --disable-doc \
    --disable-avdevice 

make

target_path=../libffmpeg
mkdir $target_path
cp libavcodec/libavcodec.a $target_path/libavcodec.a
cp libavutil/libavutil.a $target_path/libavutil.a
cp libavfilter/libavfilter.a $target_path/libavfilter.a
cp libswresample/libswresample.a $target_path/libswresample.a
cp libavformat/libavformat.a $target_path/libavformat.a
cp libswscale/libswscale.a $target_path/libswscale.a
cp libpostproc/libpostproc.a $target_path/libpostproc.a
