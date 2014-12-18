objdump -t /usr/local/lib/libuv.a | grep -v -e UND -e " file format" | grep uv_ | awk '{printf "%s ", $6; sub(/^uv_/, "_nuv_", $6); print $6}' | sort -u > rewrites
objcopy --redefine-syms=rewrites /usr/local/lib/libuv.a libnuv.a
objcopy --redefine-syms=rewrites /usr/local/lib/libhelium.a libhelium.a
