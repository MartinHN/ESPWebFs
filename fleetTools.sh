#! /bin/bash
# set -x -e

RAW_DIR="$(dirname -- "$0")"
SCRIPT_DIR=$(cd -- $RAW_DIR &>/dev/null && pwd)

function get_allIps() {
    OIFS=$IFS
    IFS='\n'
    records=$(dns-sd -t 1 -B _WebFs._tcp | grep -E "_WebFs._tcp.\s*(.*)")
    names=$(echo $records | awk '{ print $7 }')
    IFS=$OIFS
    echo $names | tr '\n' ' '
}

function upgradeESP() {
    espHostName=$1
    binFile=$2
    publicDir=$3
    echo "upgrading $espHostName"
    bash ${SCRIPT_DIR}/cliTools.sh $espHostName $binFile $publicDir &
}

function upgradeAllESP() {
    binFile=$1
    publicDir=$2
    ips=$(get_allIps)
    echo "ips are $ips"
    flagF="/tmp/espUpgraded.txt"
    if [[ -f $flagF ]]; then
        rm $flagF
    fi
    touch $flagF
    for ip in $(echo $ips); do
        # echo $ip.local $binFile $publicDir
        upgradeESP "$ip.local" $binFile $publicDir
    done
    wait

}

if [[ $1 != "" ]]; then
    echo "executing"
    upgradeAllESP $1 $2
fi
