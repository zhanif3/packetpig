#!/bin/bash
make clean
git pull
rm lib/packetpig.jar
rm lib/packetpig/target/packetpig-1.0-SNAPSHOT.jar
make clean
make

