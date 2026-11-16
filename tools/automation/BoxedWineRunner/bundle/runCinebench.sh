#!/usr/bin/env bash
set -e
automation_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
boxedwine_exe="${1:-$automation_dir/bin/boxedwine}"
java -cp "$automation_dir/bin/BoxedWineRunner.jar" boxedwine.org.PrepareFilesystem "$automation_dir/filesystem.properties" "$automation_dir/../automation-filesystems" "$automation_dir/fs"
exec java -jar "$automation_dir/bin/BoxedWineRunner.jar" -user-reg "$automation_dir/fs/user.reg" -name Cinebench "$automation_dir/fs/fs.zip" "$automation_dir/perfScripts/cinebench" "$boxedwine_exe" -nosound -novideo
