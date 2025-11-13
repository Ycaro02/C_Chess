FROM eclipse-temurin:17-jdk

RUN apt-get update && apt-get install -y unzip bash make g++ zip wget curl which file findutils
RUN echo 1 | apt install -y google-android-ndk-r20-installer

ENV ANDROID_HOME=/Android-tools
ENV ANDROID_CMDLINE_TOOLS=/Android-tools/cmdline-tools/latest

RUN rm -rf /Android-tools && mkdir -p /Android-tools

RUN mkdir -p /compile_utils

COPY . /compile_utils

RUN wget -q https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip -O /tmp/cmdline-tools.zip

RUN mkdir -p /Android-tools/cmdline-tools/latest

RUN unzip -q /tmp/cmdline-tools.zip -d /Android-tools/cmdline-tools/latest

RUN mv /Android-tools/cmdline-tools/latest/cmdline-tools/* /Android-tools/cmdline-tools/latest/

RUN rm -rf /tmp/cmdline-tools.zip

ENV PATH="${PATH}:/Android-tools/cmdline-tools/latest/bin:/Android-tools/platform-tools"

RUN yes | sdkmanager --licenses

RUN sdkmanager "platform-tools" "platforms;android-33" "build-tools;33.0.2"

ENV ANDROID_WORK_DIR=/app/chess/C_Chess/android

ENV ANDROID_NDK_ROOT=/usr/lib/android-sdk/ndk/20.0.5594570

# Openssl compile
RUN /compile_utils/compile_openssl_android.sh
# RUN mkdir /openssl
# RUN cd /openssl && wget https://github.com/openssl/openssl/releases/download/openssl-3.6.0/openssl-3.6.0.tar.gz && tar xvf openssl-3.6.0.tar.gz
# RUN export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$ANDROID_NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin:$PATH \
#     && cd /openssl/openssl-3.6.0 && ./Configure android-arm64 -D__ANDROID_API__=29 && make