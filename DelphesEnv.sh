#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
export PYTHONPATH="$DIR/python:${PYTHONPATH}"
export LD_LIBRARY_PATH="$DIR:${LD_LIBARARY_PATH}"
