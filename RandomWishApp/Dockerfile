# Use a stable Ubuntu-based image
FROM ubuntu:20.04

# Set working directory inside the container
WORKDIR /app

# Update package sources and install dependencies in a single layer
RUN apt-get update -y && \
    apt-get install -y --no-install-recommends \
    git \
    gcc \
    g++ \
    cmake \
    libboost-all-dev \
    libasio-dev \
    iputils-ping && \
    rm -rf /var/lib/apt/lists/*

# Check network connectivity by pinging an external server (for debugging)
RUN ping -c 4 google.com || echo "Network issue detected"

# Copy the source code and the json.hpp file into the container
COPY . .

# Create the include directory if not already present
RUN mkdir -p /opt/crow/include

# Copy the json.hpp file to the correct location in the container
COPY json.hpp /opt/crow/include/nlohmann/json.hpp

# Compile the C++ application
RUN g++ -o RandomWishApp RandomWishApp.cpp -I/opt/crow/include

# Set the entry point to run the compiled binary
ENTRYPOINT ["./RandomWishApp"]
