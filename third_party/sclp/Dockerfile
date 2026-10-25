# build image

FROM docker.io/library/ubuntu@sha256:80dd3c3b9c6cecb9f1667e9290b3bc61b78c2678c02cbdae5f0fea92cc6734ab AS builder

WORKDIR /app

RUN apt update && apt install -y make cmake gcc-14 g++-14 libboost-timer1.83-dev libboost-container1.83-dev

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 10 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 20 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-14 30 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-14 30

COPY CMakeLists.txt CMakeLists.txt
COPY Makefile Makefile
COPY dependency dependency
COPY include include
COPY src src
COPY test test
COPY benchmark benchmark
COPY app app

RUN make BUILD_TYPE=Release test

# working image

FROM docker.io/library/ubuntu@sha256:80dd3c3b9c6cecb9f1667e9290b3bc61b78c2678c02cbdae5f0fea92cc6734ab

WORKDIR /app

COPY dependency/Catch2/LICENSE.txt licenses/catch2_license
COPY dependency/cxxopts/LICENSE licenses/cxxopts_license
COPY dependency/fmt/LICENSE licenses/fmt_license
COPY dependency/json/LICENSE.MIT licenses/json_license
COPY --from=builder /app/build/Release/app/allocate /app

ENTRYPOINT [ "/app/allocate" ]

CMD [ "--help" ]
