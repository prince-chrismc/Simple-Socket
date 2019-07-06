#!/bin/sh

usage()
{
   echo "Usage: $0 <branch name>\n"
   echo "  Attempts to commit all tracked changes and commit them."
   echo "  Followed by creating a pull request for that branch\n"
}

if [ "$#" -ne 1 ]; then
  usage
  exit 1
fi

if [ ! -x $(which "git") ]; then
  >&2 echo "ERROR: git does not exist or is not executable!"
  exit 127
fi

git add -u
git diff-index --quiet HEAD
if [ $? -eq 0 ]; then
  echo " No changes to commit."
  exit 0
fi

git commit -m "cmake-formatted by ci"
git remote add origin-ci https://${GH_TOKEN}@github.com/prince-chrismc/Simple-Socket.git
git push --set-upstream origin-ci

curl -X POST \
-H "Content-Type: application/json" \
-H "Authorization: token ${GH_TOKEN}"
-d '{
  "title": "cmake-formatted by ci",
  "head": "$1",
  "base": "master"
  "body": "changes performed by ci linting job",
}' \
https://api.github.com/repos/prince-chrismc/Simple-Socket/pulls
