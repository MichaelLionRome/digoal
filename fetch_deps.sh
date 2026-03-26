#!/bin/bash


exit_on_error() {
    exit_code=$1
    last_command=${@:2}
    if [ $exit_code -ne 0 ]; then
        >&2 echo "\"${last_command}\" command failed with exit code ${exit_code}."
        exit $exit_code
    fi
}

# enable !! command completion
set -o history -o histexpand

mkdir -p lib
cd lib

# Begin c-bitvector
if [[ -d "c-bitvector" ]]; then
    cd c-bitvector
    git fetch
    git pull
else
    if ! (git clone https://github.com/CristobalM/c-bitvector) then
        echo "Couldn't retrieve c-bitvector repository.. exiting"
        exit 1
    fi
    cd c-bitvector
fi

make build-very-light
cd ..
# End c-bitvector


# Begin c-vector
if [[ -d "c-vector" ]]; then
    cd c-vector
    git fetch
    git pull
else
    if ! (git clone https://github.com/CristobalM/c-vector) then
        echo "Couldn't retrieve c-vector repository.. exiting"
        exit 1
    fi
    cd c-vector
fi

cd ..
# End c-vector
