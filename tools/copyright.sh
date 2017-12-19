#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
HEADER="$DIR/header.txt"

if ((BASH_VERSINFO[0] < 4))
then
    echo "Sorry, you need at least bash 4.0 to run this script."
    exit 1
fi

shopt -s globstar

pushd $DIR/.. > /dev/null

for f in **/*.h; do
    if [[ $f == *"scripts/"* ]] || [[ $f == *"include/config"* ]] || [[ $f == *"include/generated"* || $f == *"include/arch"* ]]; then
        echo "Skipping non-kernel code $f..."
    else
        if grep -q "Copyright (C) 2017" "$f"; then
            echo "Skipping $f..."
        else
            cat $HEADER $f > $f.new
            mv $f.new $f
            echo "Added header for $f..."
        fi
    fi
done

for f in **/*.c; do
    if [[ $f == *"scripts/"* ]] || [[ $f == *"include/config"* ]] || [[ $f == *"include/generated"* || $f == *"include/arch"* ]]; then
        echo "Skipping non-kernel code $f..."
    else
        if grep -q "Copyright (C) 2017" "$f"; then
            echo "Skipping $f..."
        else
            cat $HEADER $f > $f.new
            mv $f.new $f
            echo "Added header for $f..."
        fi
    fi
done

popd > /dev/null
