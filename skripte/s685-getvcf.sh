#!/bin/sh

HOST=192.168.1.3
PIN=xxxx
COOKIEFILE=s685-cookie

HEADSETS=( "el" "mo" )
ELEMENTS=${#HEADSETS[@]}

echo "logging in to $HOST"
curl -sX POST \
	http://$HOST/login.html \
		-d "language=1&password=$PIN" \
		-b@$COOKIEFILE \
		-c $COOKIEFILE >/dev/null

for (( HEADSETNUMBER=0;HEADSETNUMBER<$ELEMENTS;HEADSETNUMBER++)); do
    HEADSETNAME=${HEADSETS[${HEADSETNUMBER}]}
	curl -sX POST \
		http://$HOST/settings_telephony_tdt.html \
		-d "tdt_function=1&tdt_handset_port=$HEADSETNUMBER&tdt_file=&" \
		-b@$COOKIEFILE >/dev/null
	echo "loading telephone directory of handset $HEADSETNUMBER, $HEADSETNAME..."
	curl -X POST \
		http://$HOST/teledir.vcf?id=255 \
			-b@$COOKIEFILE > $HEADSETNAME.vcf
done

