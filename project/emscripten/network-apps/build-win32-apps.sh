#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

copy_to_release() {
  local artifact="$1"
  local release_root="${script_dir}/../Build/Release"
  if [[ -d "${release_root}" ]]; then
    local release_apps_dir
    release_apps_dir="$(cd "${release_root}" && pwd)/network-apps"
    mkdir -p "${release_apps_dir}"
    cp "${script_dir}/${artifact}" "${release_apps_dir}/${artifact}"
    echo "Copied ${artifact} to ${release_apps_dir}"
  fi
}

clang_bin="${PE_CLANG:-clang}"
lld_bin="${PE_LLD:-/Users/ai/Desktop/projects/emsdk/upstream/bin/lld}"
strip_bin="${PE_STRIP:-/Users/ai/Desktop/projects/emsdk/upstream/bin/llvm-strip}"

build_pe() {
  local name="$1"
  local obj="/tmp/${name}.win32.obj"
  local exe="${name}.exe"
  local zip_name="${name}-win32.zip"

  echo "Building ${exe}"
  "${clang_bin}" -target i386-pc-windows-msvc -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -c \
    -o "${obj}" \
    "${script_dir}/${name}.c"
  "${lld_bin}" -flavor link /entry:start /subsystem:console /nodefaultlib \
    /fixed /dynamicbase:no /nxcompat:no /stack:1048576,1048576 \
    "/out:${script_dir}/${exe}" \
    "${obj}"
  "${strip_bin}" "${script_dir}/${exe}" || true
  zip -j -q -o "${script_dir}/${zip_name}" "${script_dir}/${exe}"
  copy_to_release "${zip_name}"
}

build_pe network-share-host
build_pe network-share-join
build_pe network-share-agent

echo "Win32 network share apps built in ${script_dir}"
