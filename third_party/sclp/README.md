# A Tree Search-Based Heuristic for the Three-Dimensional Single Container Loading Problem

This project was developed by Lucas Guesser during his Master's degree, under the supervision of Professor [Flávio Keidi Miyazawa, PhD](https://ic.unicamp.br/~fkm/) and [Professor Thiago Alves de Queiroz, PhD](https://sites.google.com/ufcat.edu.br/taq/). The thesis is available in the [Repository of Intellectual Production of Unicamp](https://www.repositorio.unicamp.br/acervo/detalhe/1499023), you can [download it from this permanent link](https://doi.org/10.47749/T/UNICAMP.2025.1499023).

## Compile and run

This project uses CMake, but there is a `Makefile` to make everything easier. Once the dependencies are installed, just run:

```sh
make
```

It will compile the code into the directory `build`. By default, it compiles in Debug mode. You can switch to Release by running `make BUILD_TYPE=Release` (it creates a separate directory for each one in `build`).

Then you can run the app that comes with this code by executing:

```sh
./build/Debug/app/allocate --help
```

I have added the `--help` flag so that when you execute the first time, it tells you all the options available.

There are two example files in [`doc`](./doc/): [`instance 1 of set 1`](./doc/set_1_instance_1.json) and [`instance 1 of set 15`](./doc/set_15_instance_1.json) (from the instances of Bischoff and Ratcliff 1995). You can solve it by running

```sh
./build/Debug/app/allocate doc/set_1_instance_1.json --output-file=output.json
```

Its output is saved into the file `output.json`.

## Dependencies

* gcc 14
* make 4.4
* cmake 3.30
* boost 1.83

Install on Fedora 41: `sudo dnf install gcc g++ make cmake boost-devel`.

Install on Ubuntu 22: `sudo apt install -y make cmake gcc-14 g++-14 libboost-timer1.83-dev libboost-container1.83-dev`.

## Using with Docker

1. install container manager:
   1. [Docker](https://docs.docker.com/engine/install/);
   2. [Podman](https://podman.io/);
2. build:
   1. there is a script dedicated to that `./build_image`;
   2. for more info: `./build_image --help`;
3. run:

 ```sh
    INPUT_FILE="YOUR_FILE_HERE"
    OUTPUT_FILE="YOUR_FILE_HERE"
    OPTIONS="YOUR_CLI_OPTIONS_HERE"
    COMMIT=$(git rev-parse --short HEAD)
    INPUT_DIR=$(realpath $(dirname ${INPUT_FILE}))
    OUTPUT_DIR=$(realpath $(dirname ${OUTPUT_FILE}))
    INPUT_FILE_NAME=$(basename ${INPUT_FILE})
    OUTPUT_FILE_NAME=$(basename ${OUTPUT_FILE})
    CONTAINER_INPUT_DIR="/tmp/input"
    CONTAINER_OUTPUT_DIR="/tmp/output"

    mkdir -p ${OUTPUT_DIR}

    docker container run --rm -it \
     -v ${INPUT_DIR}:${CONTAINER_INPUT_DIR}:z \
     -v ${OUTPUT_DIR}:${CONTAINER_OUTPUT_DIR}:z \
     ${DOCKER_REGISTRY}:sclp-app-${COMMIT} \
     --output-file="${CONTAINER_OUTPUT_DIR}/${OUTPUT_FILE_NAME}" \
     ${OPTIONS} \
     ${CONTAINER_INPUT_DIR}/${INPUT_FILE_NAME}
 ```

## Install Docker

Follow the instructions to install Docker:

* [Docker official install instructions](https://docs.docker.com/engine/install/);
  * [Docker install Fedora](https://docs.docker.com/engine/install/fedora/);
  * [Docker install Ubuntu](https://docs.docker.com/engine/install/ubuntu/);
* post-install:

 ```sh
    sudo groupadd docker
    sudo usermod -aG docker $USER # add user to docker group
    # log out and log in again
    docker run hello-world # check that it worked
    sudo systemctl enable docker.service # enable docker to start with system
    sudo systemctl enable containerd.service
```

## Licenses

* [Boost](https://www.boost.org/) - Boost Software License 1.0
* [Catch 2](https://github.com/catchorg/Catch2) - Boost Software License 1.0
* [cxxopts](https://github.com/jarro2783/cxxopts) - 2014 Jarryd Beck - MIT License
* [{fmt}](https://github.com/fmtlib/fmt) - 2012 - present, Victor Zverovich and {fmt} contributors - MIT License
* [JSON for Modern C++](https://github.com/nlohmann/json) - 2013-2022 Niels Lohmann - MIT License
* [cpbook-code](https://github.com/stevenhalim/cpbook-code/tree/master) - 2024 Steven Halim, Felix Halim, Suhendry Effendy - The Universal Permissive License (UPL) 1.0
* [Algorithms for Competitive Programming](https://cp-algorithms.com/) - 2014 - 2025 cp-algorithms contributors - CC BY-SA 4.0
