cat synthesize-text.txt | \
grep 'audioContent' | \
sed 's|audioContent| |' | \
tr -d '\n ":{},' > tmp.txt \
&& base64 tmp.txt --decode > synthesize-text-audio.mp3 && rm tmp.txt
