FROM debian:stable-slim AS base
RUN apt-get update && apt-get install -y curl cmake unzip clang build-essential python3 libbz2-dev libz-dev libicu-dev libboost-all-dev git

FROM ghcr.io/medkhabt/ogdf-build:latest AS ogdf-build 

FROM base AS project-build
WORKDIR /app
COPY --from=ogdf-build /src/ogdf /app/ogdf
COPY include ./include 
COPY src ./src 
COPY run ./run
COPY resources ./resources
COPY CMakeLists.txt CMakeLists.txt
RUN mkdir -p build && cmake -DOGDF_BUILD_DIR="/app/ogdf" -S . -B build 
WORKDIR /app/build
RUN make -j8 

