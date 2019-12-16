#!/bin/bash

cd robot_agent
make
cd ..
./sync.sh

cd mission_control
./mission_control
