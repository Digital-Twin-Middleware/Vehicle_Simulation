# Use an official C++ build image with Ubuntu 22.04
FROM ubuntu:22.04

# Set the environment variable for non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \ 
    libmosquitto-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory in the container
WORKDIR /app

# Copy the entire source code into the container
COPY . .

# Create a build directory
RUN mkdir build && cd build && \
    cmake .. && \
    make

# Set the entry point for the container
ENTRYPOINT ["./build/VehicleSimulation"]
