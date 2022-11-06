#!/bin/bash

if [ -f "main" ]; then
    echo ">run.sh : Running ElektronGL..."
    ./main
else 
    echo ">run.sh : ElektronGL not built. Building now..."
    ./scripts/build.sh
    echo ">run.sh : Running ElektronGL..."
    ./main
fi
