#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TARGET_FILE="$DIR/../include/kernel/version.h"
TMP_FILE=".tmpversion"

VERSION=$1
ARCH=$2
COMPILER=$3

NAME="Quark"
USER=`whoami`
HOSTNAME=`hostname`
DATE=`date`

COMPILER_VERSION=`$COMPILER -v 2>&1 | grep -i version`

function cdefine() {
    echo \#define $1 \"$2\"
}

(
    echo \#pragma once
    echo
    cdefine "OS_NAME" "$NAME"
    cdefine "OS_VERSION" "$VERSION"
    cdefine "OS_COMPILER" "$COMPILER_VERSION"
    cdefine "OS_HOST" "$USER@$HOSTNAME"
    cdefine "OS_ARCH" "$ARCH"
    cdefine "OS_DATE" "$DATE"
    echo
    echo \#define "OS_STRING" 'OS_NAME " version " OS_VERSION " " OS_ARCH " (" OS_HOST ") " OS_COMPILER " " OS_DATE'
) > "$TMP_FILE"

cp -f "$TMP_FILE" "$TARGET_FILE"
rm "$TMP_FILE"
