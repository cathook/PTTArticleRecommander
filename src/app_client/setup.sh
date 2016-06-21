#! /usr/bin/env bash

me="${0}"
server_url="${1}"
pattern='<TEMPLATE_SERVER_ADDR>'

find -type f | while read fname; do
  sed -i "s/${pattern}/${server_url}/g" "$fname"
done

rm "${me}"
