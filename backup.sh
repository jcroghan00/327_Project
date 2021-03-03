#!/usr/bin/bash
mkdir -p backups

backup_file="../327_Project"
dest="backups"
archive_file="$(date +'%Y-%m-%d_%H-%M-%S'.tgz)"
touch $dest/$archive_file
echo "Backing up $backup_files to $dest/$archive_file"

tar czvf $dest/$archive_file -X .exclude.txt $backup_file
