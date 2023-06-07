function get_allIps() {
    records=$(dns-sd -t 1 -B _WebFs._tcp | grep _WebFs._tcp.)
    names=$(echo $records | awk '{ print $7 }')
    echo $names | tr '\n' ' '

}

function upgradeESP() {
    espHostName=$1
    binFile=$2
    publicDir=$3
    echo "upgrading $espHostName"
    ./cliTools.sh $espHostName $binFile $publicDir &
}

function upgradeAllESP() {
    binFile=$1
    publicDir=$2
    ips=$(get_allIps)
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
