#!/bin/sh

usage()
{
   echo "Usage: $0 <dir>"
   echo ""
   echo "  Used to call the cmake-format on the various 'CMakeLists.txt' located under 'dir'"
   echo ""
}

if [ "$#" -ne 1 ]; then
  usage
  exit 1
fi

if [ ! -d "$1" ]; then
  >&2 echo "'$1' is not a directory"
  usage
  exit 1
fi

if [ ! -x $(which "cmake-format") ]; then
  >&2 echo "ERROR: cmake-format does not exist or is not executable!"
  echo "Try: pip install cmake-format (requires python3)"
  exit 127
fi

for file in $(find $1 -name "*CMakeLists.txt")
do
   echo "Formatting $file..."
   cmake-format "$file" > "$file.format"
   rm "$file"
   mv "$file.format" "$file"
done

exit 0