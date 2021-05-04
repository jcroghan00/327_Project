#!/usr/bin/bash

last_name="tullis"
first_name="ryan"
#path to the directory your code is located in relative to submit.sh
code_dir="gamecode"
#path to README, if same as code_dir set to ""
README_path="README"
#path to CHANGELOG, if same as code_dir or README set to ""
CHANGELOG_path="CHANGELOG"

submit_title="$last_name-$first_name.assignment-"
submit_version=$1
submit_dir="$submit_title$submit_version"

while true; do
read -rp "Did you update the README? " yn
  case $yn in
    [Yy]* ) break;;
    [Nn]* ) exit;;
  esac
done
while true; do
read -rp "Is this the correct file name? '$submit_title$submit_version.tar.gz' " title
  case $title in
    [Yy]* ) break;;
    [Nn]* ) exit;;
  esac
done
mkdir -p "$submit_dir"
cd $code_dir || { echo "Failure"; exit 1; }
make clobber
cd ..
touch $CHANGELOG_path
# copies the git log to CHANGELOG
# comment this line out if you don't use git. just need to make sure your CHANGELOG is updated
git log --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit > $CHANGELOG_path
#copy CHANGELOG, README and the contents of code_dir to submit_dir
cp -a "$code_dir"/. $CHANGELOG_path $README_path "$submit_dir"
touch "$submit_dir".tar.gz
#create the tarball excluding hidden files
tar cvfz "$submit_dir".tar.gz --exclude=".*" "$submit_dir"
# remove temp folder when done
sudo rm -rf "$submit_dir"