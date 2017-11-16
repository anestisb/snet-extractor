#!/usr/bin/env bash
#
# snet-extractor
# -----------------------------------------
#
# Anestis Bechtsoudis <anestis@census-labs.com>
# Copyright 2017 by CENSUS S.A. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e # fail on unhandled error
set -u # fail on undefined variable
#set -x # debug

readonly SNET_FLAGS="https://www.gstatic.com/android/snet/snet.flags"
readonly TMP_WORK_DIR=$(mktemp -d /tmp/snet-extractor.XXXXXX) || exit 1
readonly TOOL_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# JSON beautifier (https://stedolan.github.io/jq/)
readonly JBEA_MAC="$TOOL_ROOT/bin/jq-osx-amd64"
readonly JBEA_LINUX="$TOOL_ROOT/bin/jq-linux64"

readonly kMetadataFile="metadata_flags.txt"
readonly kPayloadFile="payload.snet"

readonly FLAGS_FILE="snet_flags.json"
readonly JAR_FILE="snet.jar"

abort() {
  # If debug enabled, keep work directory for further investigation
  if [[ "$-" == *x* ]]; then
    echo "[!] Workspace available at '$TMP_WORK_DIR' - delete manually when done"
  else
    rm -rf "$TMP_WORK_DIR"
  fi
  exit "$1"
}

isMacOS() {
  if [[ "$(uname -s)" == "Darwin" ]]; then
    return 0
  fi
  return 1
}

# Main
trap "abort 1" SIGHUP SIGINT SIGTERM

if isMacOS; then
  JBEA="$JBEA_MAC"
else
  JBEA="$JBEA_LINUX"
fi

echo "[*] Downloading SNET flags file"
curl "$SNET_FLAGS" --output "$TMP_WORK_DIR/snet.flags" || {
  echo "[-] Failed to download snet flags data"
  abort 1
}

gcc -Wall -Wextra -Werror -o "$TOOL_ROOT/snet-bin-parser" "$TOOL_ROOT/snet-bin-parser.c" || {
  echo "[-] Failed to compile snet binary parser tool"
  abort 1
}

"$TOOL_ROOT/snet-bin-parser" "$TMP_WORK_DIR/snet.flags" "$TMP_WORK_DIR" || {
  echo "[-] Failed to parse snet.flags"
  abort 1
}

version=$(grep "^VERSION:" "$TMP_WORK_DIR/$kMetadataFile" | cut -d ":" -f2)
echo "[*] Detected snet version '$version'"

outputDir="snet-$version"
mkdir -p "$outputDir"
cp "$TMP_WORK_DIR/$kPayloadFile" "$outputDir/$FLAGS_FILE"
cp "$TMP_WORK_DIR/$kMetadataFile" "$outputDir"
cp "$TMP_WORK_DIR/snet.flags" "$outputDir"

downloadUrl=$($JBEA .url "$outputDir/$FLAGS_FILE" | tr -d '"')
fileName="${downloadUrl##*/}"

echo "[*] Downloading SNET Jar file"
curl "$downloadUrl" --output "$outputDir/$fileName" || {
  echo "[-] Failed to download snet jar data"
  abort 1
}

"$TOOL_ROOT/snet-bin-parser" "$outputDir/$fileName" "$TMP_WORK_DIR" || {
  echo "[-] Failed to parse $fileName"
  abort 1
}
cp "$TMP_WORK_DIR/$kPayloadFile" "$outputDir/$JAR_FILE"

echo "[*] All files successfully extract at '$outputDir'"
abort 0
