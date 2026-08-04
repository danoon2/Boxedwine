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

clang_bin="${CLANG:-clang}"
lld_bin="${LLD:-/Users/ai/Desktop/projects/emsdk/upstream/bin/lld}"
strip_bin="${STRIP:-/Users/ai/Desktop/projects/emsdk/upstream/bin/llvm-strip}"

build_elf() {
  local name="$1"
  local strip="${2:-false}"
  local obj="/tmp/${name}.o"
  echo "Building ${name}"
  "${clang_bin}" -target i386-unknown-linux-elf -c \
    -o "${obj}" \
    "${script_dir}/${name}.S"
  "${lld_bin}" -flavor gnu -m elf_i386 -static -e _start \
    -o "${script_dir}/${name}" \
    "${obj}"
  if [[ "${strip}" == "true" ]]; then
    "${strip_bin}" "${script_dir}/${name}"
  fi
  zip -j -q -o "${script_dir}/${name}.zip" "${script_dir}/${name}"
  copy_to_release "${name}.zip"
}

build_c_elf() {
  local name="$1"
  local strip="${2:-false}"
  local obj="/tmp/${name}.o"
  echo "Building ${name}"
  "${clang_bin}" -target i386-unknown-linux-elf -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -c \
    -o "${obj}" \
    "${script_dir}/${name}.c"
  "${lld_bin}" -flavor gnu -m elf_i386 -static -e _start \
    -o "${script_dir}/${name}" \
    "${obj}"
  if [[ "${strip}" == "true" ]]; then
    "${strip_bin}" "${script_dir}/${name}"
  fi
  zip -j -q -o "${script_dir}/${name}.zip" "${script_dir}/${name}"
  copy_to_release "${name}.zip"
}

build_elf network-echo-test
build_elf network-udp-test
build_elf network-mdns-options-test
build_elf network-mdns-recv-test
build_c_elf network-mdns-nabu-responder-test true
build_c_elf network-mdns-nabu-query-test true
build_elf network-dns-test
build_elf network-listen-test
build_elf network-iface-test
build_c_elf network-netlink-iface-test true
build_elf network-room-tcp-server true
build_elf network-room-tcp-client true
build_c_elf network-share-host true
build_c_elf network-share-join true
build_c_elf network-share-agent true

echo "Linux network app built in ${script_dir}"
