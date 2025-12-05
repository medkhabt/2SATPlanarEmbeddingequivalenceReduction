FROM debian:stable-slim AS base
RUN apt-get update && apt-get install -y curl cmake unzip clang build-essential python3 libbz2-dev libz-dev libicu-dev libboost-all-dev git

FROM ghcr.io/medkhabt/ogdf-build:latest AS ogdf-build 

FROM base AS project-build
WORKDIR /app/Core
COPY --from=ogdf-build /src/ogdf /app/ogdf
COPY Core/include ./include 
COPY Core/src ./src 
COPY Core/run ./run
COPY Core/resources ./resources
COPY Core/CMakeLists.txt CMakeLists.txt
RUN mkdir -p build && cmake -DOGDF_BUILD_DIR="/app/ogdf" -S . -B build 
WORKDIR /app/Core/build
RUN make -j8 

FROM golang:alpine AS api-build

WORKDIR /app/API

COPY API/main.go ./main.go 
COPY API/script.sh ./script.sh 
COPY API/go.mod ./go.mod
COPY --from=project-build /app/Core/build /app/Core/build

RUN chmod +x script.sh

RUN go mod download

RUN go build -o /api

FROM debian:stable-slim
COPY --from=api-build /app/Core/ /app/Core/
COPY --from=api-build /app/API/ /app/API/
COPY --from=api-build /api /api

EXPOSE 8087

WORKDIR /app/API

CMD ["/api"]


