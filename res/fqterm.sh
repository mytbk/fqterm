#!/bin/sh

this_name="$0"

current_directory=`/bin/pwd`

cd `dirname "$this_name"`
base_name=`basename "$this_name"`
bin_directory=`/bin/pwd`

# get the real file name in case it's a symbolic link.
while [ -h "$this_name" ]; do
  this_name=`/bin/ls -l "$base_name" | sed -e 's/^.* -> //' `
  cd `dirname "$this_name"`
  base_name=`basename "$this_name"`
  bin_directory=`/bin/pwd`
done

# setup environment variables and then run the real executable.
if [ -x "$base_name".bin ]; then
  cd "$current_directory"
  export FQTERM_PREFIX=`dirname "$bin_directory"`
  export FQTERM_RESOURCE="$FQTERM_PREFIX/share/FQTerm"
  "$bin_directory/$base_name.bin" "$@"
  exit $?
else
  cd "$current_directory"
  echo "Error, cannot find $base_name."
  exit 2
fi
