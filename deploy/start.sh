#!/bin/bash
# Use docker openjdk:24 to run this script with `pwd` mounted to /app 
cd /app
cp ./test.db* /tmp
java -jar SimpleET-0.0.1-SNAPSHOT-exec.jar
