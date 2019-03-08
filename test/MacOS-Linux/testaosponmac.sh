#!/bin/bash
testfailed=0
projectdir=`pwd`

emulatorName="msix_android_emulator"
avdPackage=""

usage()
{
    echo "usage: ./testaosponmac [-avd <emulator name>] [-c <package>]"
    echo $'\t' "-avd <emulator name>. Name of avd. Default msix_android_emulator"
    echo $'\t' "-c <package>. Create avd with specified package with the name defined by -adv. If not present assume the emulator already exits"
}

while [ "$1" != "" ]; do
    case $1 in
        -avd ) shift
             emulatorName=$1
             ;;
        -c ) shift
             avdPackage=$1
             ;;
        -h ) usage
             exit
             ;;
        * )  usage
             exit 1
    esac
    shift
done

function ParseResult {
    local FILE="$1"
    if [ ! -f $FILE ]
    then
        echo $FILE" not found!"
        exit 1
    fi
    cat $FILE
    if grep -q "FAILED" $FILE
    then
        echo "FAILED"
        testfailed=1
    else
        echo "succeeded"
    fi
}

function TerminateEmulator {
    $ANDROID_HOME/platform-tools/adb emu kill &
}

# Create emulator if requested
if [ -n "$avdPackage" ]; then
    echo "Creating emulator" $emulatorName
    echo "no" | $ANDROID_HOME/tools/bin/avdmanager create avd -n $emulatorName -k '$avdPackage' --force
fi
echo "Listing emulators"
$ANDROID_HOME/emulator/emulator -list-avds
echo "Starting emulator" $emulatorName
nohup $ANDROID_HOME/emulator/emulator -avd $emulatorName -no-snapshot > /dev/null 2>&1 &
$ANDROID_HOME/platform-tools/adb wait-for-device shell 'while [[ -z $(getprop sys.boot_completed) ]]; do sleep 1; done; input keyevent 82'
$ANDROID_HOME/platform-tools/adb devices
echo "Emulator started"

# Clean up.
$ANDROID_HOME/platform-tools/adb shell rm -rf /data/data/com.microsoft.androidbvt/files
rm -f $projectdir/../mobile/androidbvt/testResults.txt
rm -f $projectdir/../mobile/androidbvt/testApiResults.txt

# Create App
cd $projectdir/../mobile/AndroidBVT
mkdir -p app/src/main/assets
cp -R $projectdir/../appx/* app/src/main/assets
cp $projectdir/../../.vs/test/api/input/apitest_test_1.txt app/src/main/assets
mkdir -p app/src/main/jniLibs/x86
cp $projectdir/../../.vs/lib/libmsix.so app/src/main/jniLibs/x86
mkdir -p app/src/main/libs
cp $projectdir/../../.vs/lib/msix-jni.jar app/src/main/libs
rm -r build app/build
sh ./gradlew assembleDebug

# Install app
$ANDROID_HOME/platform-tools/adb push app/build/outputs/apk/debug/app-debug.apk /data/local/tmp/com.microsoft.androidbvt
$ANDROID_HOME/platform-tools/adb shell pm install -t -r '/data/local/tmp/com.microsoft.androidbvt'

# Start app
$ANDROID_HOME/platform-tools/adb shell am start -n 'com.microsoft.androidbvt/com.microsoft.androidbvt.MainActivity' -a android.intent.action.MAIN -c android.intent.category.LAUNCHER

# The app terminates when is done
count=0
while ! $ANDROID_HOME/platform-tools/adb shell ps | grep -q "com.microsoft.androidbvt"
do
    echo "Waiting for test app to start..."
    ((count+=5))
    if [ $count -eq 120 ]; then
        echo "App never started"
        TerminateEmulator
        exit 1
    fi
    sleep 5
done
count=0
while adb shell ps | grep -q "com.microsoft.androidbvt"
do
    echo "Test is running..."
    ((count+=5))
    if [ $count -eq 240 ]; then
        echo "Test never completed"
        TerminateEmulator
        exit 1
    fi
    sleep 5
done

# Get Results
$ANDROID_HOME/platform-tools/adb pull /data/data/com.microsoft.androidbvt/files/testResults.txt
$ANDROID_HOME/platform-tools/adb pull /data/data/com.microsoft.androidbvt/files/testApiResults.txt

TerminateEmulator

ParseResult testResults.txt
ParseResult testApiResults.txt

echo "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
if [ $testfailed -ne 0 ]
then
    echo "                           FAILED                                 "
    exit $testfailed
else
    echo "                           passed                                 "
    exit 0
fi
