#!/bin/bash

VERSION="$1"
sed -i "s/^key0=.*/key0=0^M--^MPosted from FQTerm ${VERSION}^M^WL^M/" res/userconf/fqterm.cfg.orig
sed -i "s/^set(FQTERM_VERSION.*/set(FQTERM_VERSION \"${VERSION}\")/" CMakeLists.txt
