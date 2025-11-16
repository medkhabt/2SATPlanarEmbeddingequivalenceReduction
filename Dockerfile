FROM debian:stable-slim AS base
RUN apt-get update && apt-get install -y curl cmake unzip clang build-essential python3 libbz2-dev libz-dev libicu-dev libboost-all-dev git

FROM base AS ogdf-build 
WORKDIR /src
COPY ogdf ./ogdf
RUN rm ogdf/CMakeCache.txt
WORKDIR /src/ogdf
RUN cmake .  && make -j8

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

