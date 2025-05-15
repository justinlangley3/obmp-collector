# Collector: openbmp/collector
#
# Copyright (c) 2021-2022 Cisco Systems, Inc. and others.  All rights reserved.
#
# Original Author: Tim Evens <tim@openbmp.org>
#
# BUILD:
#
#   Use buildkit - either via export DOCKER_BUILDKIT=1 or by using buildx cli
#
#   Run the following from the obmp-collector project root:
#
#   docker buildx build \
#     --platform linux/amd64 \
#     --progress=plain \
#     --build-arg VERSION=2.2.3 \
#     -f docker/Dockerfile \
#     -t openbmp/collector:2.2.3 \
#     .
#
# -----------------------------------------------
# stage: Build collector
# -----------------------------------------------
FROM alpine:3.21 AS build


# Install build dependencies
RUN apk update \
    && apk add --no-cache \
         bash \
         git \
         # c++ toolchain
         binutils \
         cmake \
         g++ \
         gcc \
         libstdc++ \
         make \
         musl \
         # libraries
         boost-dev \
         cyrus-sasl-dev \
         librdkafka-dev \
         lz4-dev \
         openssl-dev \
         yaml-cpp-dev \
         zlib-dev \
         zstd-dev

# Add source files
WORKDIR /ws

COPY Server ./Server
COPY CMakeLists.txt .
COPY openbmpd_version.h.in .

# Build the collector
RUN rm -rf build \
    && mkdir -p build \
    && cd build \
    && cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        .. \
    && make \
    && make install

# Remove build libs to keep image small
RUN apk del \
      boost-dev \
      cyrus-sasl-dev \
      librdkafka-dev \
      lz4-dev \
      openssl-dev \
      yaml-cpp-dev \
      zlib-dev \
      zstd-dev \
    # clean package cache
    && rm -rf /var/cache/apk/*

# -----------------------------------------------
# stage: Final image
# -----------------------------------------------
FROM alpine:3.21

# Proxy servers
#ENV http_proxy=http://proxy:80
#ENV https_proxy=http://proxy:80
#ENV no_proxy "domain.com"

# Install runtime dependencies
RUN apk update \
    && apk add --no-cache \
         bash \
         iproute2 \
         cyrus-sasl \
         libgcc \
         librdkafka \
         libstdc++ \
         openssl \
         yaml-cpp \
         yq-go \
         zlib \
    # clean package cache
    && rm -rf /var/cache/apk/*

# Set version and default configuration
ARG VERSION=0.0.0
RUN touch /usr/local/version-${VERSION}

# Add required files
COPY --from=build /usr/bin/openbmpd /usr/bin/
COPY --chmod=755 docker/scripts/configure/01_generate_config /usr/bin/openbmpd-configure
COPY --chmod=755 docker/scripts/docker-entrypoint.sh /docker-entrypoint.sh

WORKDIR /tmp

VOLUME ["/config"]
VOLUME ["/var/log"]

ENTRYPOINT ["/docker-entrypoint.sh"]

CMD ["/usr/bin/openbmpd", "-f", "-c", "/config/openbmpd.conf", "-l", "/var/log/openbmpd.log"]

EXPOSE 5000

