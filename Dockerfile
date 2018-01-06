FROM ubuntu:xenial

COPY installer.sh /

ENV DEBIAN_FRONTEND non-interactive

RUN apt-get update && \
	apt-get -qq install \
		wget \
		git \
		astyle \
		cmake \
		build-essential \
		pkg-config && \
	./installer.sh

WORKDIR /WienerTakesAll/
