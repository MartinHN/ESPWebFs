# set -x
espIp="phatou.local" #"phatou.local"
hasResolvedIp=false
function resolveEspIp() {
    if [[ $hasResolvedIp == "1" ]]; then
        return
    fi
    #use ip instead of domain name to avoid resolving at each curl call
    espIp=$(ping -c1 -n $espIp | head -n1 | sed "s/.*(\([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\)).*/\1/g")
    hasResolvedIp="1"
}

function ota_time_diff() {
    resolveEspIp
    remoteDate=$(curl --get -s http://$espIp/m/compDate)
    remoteTime=$(date -jf "%b %d %Y %H:%M:%S" "$remoteDate" +%s)
    # echo remote $remoteDate

    localDate=$(date -r $1)
    localTime=$(date -r $1 +%s)
    # echo local $localDate $localTime
    diffT=$(expr $localTime - $remoteTime)
    echo $diffT
}

function ota_update() {
    resolveEspIp
    curl -F update=@$1 http://$espIp/m/update --progress-bar
}

function ota_try_update() {
    resolveEspIp
    d=$(ota_time_diff $1)
    if ((${d#-} > 60)); then
        echo "need ota update ($d s)"
        ota_update $1
    else
        echo "ota up to date ($d s)"
    fi
}

function upload_file {
    resolveEspIp
    curl -F upload_data=@$1 http://$espIp/m/upload
}

function download_file {
    resolveEspIp
    curl --get --data-urlencode "file_path=$1" http://$espIp/m/download --output $1
}

function delete_file() {
    resolveEspIp
    curl -s --get --data-urlencode "file_path=$1" http://$espIp/m/delete
}

function get_hash() {
    resolveEspIp
    curl -s --get --data-urlencode "file_path=$1" http://$espIp/m/hash
}

function ls_remote() {
    resolveEspIp
    curl -s --get --data-urlencode "file_path=$1" http://$espIp/m/ls
}

function getLocalHash() {
    resolveEspIp
    md5sum $1 | awk '{ print $1 }'
}

function update_file() {
    echo "checking "$1

    RemoteH=$(get_hash $1)
    LocalH=$(getLocalHash $1)
    echo "remote : " $RemoteH "local : " $LocalH

    if [[ $RemoteH == $LocalH ]]; then
        echo "up to date"
    else
        echo "uploading "$1
        upload_file $1
    fi

}

function check_connected() {
    resolveEspIp
    if [[ $espIp == "" ]]; then
        echo "ip not resolved"
        exit 4
    fi
    # echo "checking conection $espIp"
    # ping $espIp -o -q -t3
    # conStatus=$?
    # if [[ $conStatus == 0 ]]; then
    #     echo "connected"
    # else
    #     echo "esp $espIp is not connected"
    #     exit $conStatus
    # fi
}

function remove_erased() {
    remFs=$(ls_remote)
    echo "recvd" $remFs

    for f in $(echo $remFs | sed "s/,/ /g"); do
        ex=$(find $1 -name $f)
        if ((${#ex})); then
            echo $f "found" $ex
        else
            echo $f "not found" $ex
            delete_file $f
        fi
    done
}

function update_new_files() {
    check_connected
    cd $1
    for file in ./*; do
        if [[ -f $file ]]; then
            update_file $file
        fi
    done
    cd -

}

function sync_folder() {
    update_new_files $1
    remove_erased $1
}

if [[ $1 != "" ]]; then
    espIp=$1
    check_connected
    ota_try_update $2
    if [[ $3 != "" ]]; then
        update_new_files $3
    fi
    echo $espIp >>"/tmp/espUpgraded.txt"

fi
