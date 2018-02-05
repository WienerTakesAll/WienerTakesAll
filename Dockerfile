FROM ubuntu:xenial

COPY installer.sh /WienerTakesAll/

ENV DEBIAN_FRONTEND non-interactive

WORKDIR /WienerTakesAll/

RUN apt-get update && \
	apt-get -qq install \
		wget \
		git \
		astyle \
		cmake \
		build-essential \
		pkg-config && \
	./installer.sh

