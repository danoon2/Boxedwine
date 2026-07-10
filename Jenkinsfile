// Notes:
// Windows build: put wget, msbuild and build tools in path (C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\bin\Hostx64\x64)
void gitCheckout() {
    def retryAttempt = 0
    retry(3) {
        if (retryAttempt > 0) {
            sleep(120  * retryAttempt)
        }
        def checkoutVars = checkout scm
        if (checkoutVars.GIT_COMMIT) {
            env.BOXEDWINE_GIT_COMMIT = checkoutVars.GIT_COMMIT
        }
    }
}

void publishGithubBuildStatus(String state, String description) {
    def commit = env.GIT_COMMIT ?: env.BOXEDWINE_GIT_COMMIT
    if (!commit) {
        echo 'GIT_COMMIT and BOXEDWINE_GIT_COMMIT are not set; skipping GitHub build status update.'
        return
    }

    catchError(buildResult: 'SUCCESS', stageResult: 'UNSTABLE') {
        withCredentials([usernamePassword(credentialsId: '2f2698c7-8fb4-4eb7-9cde-d048228a04ae', usernameVariable: 'GITHUB_USER', passwordVariable: 'GITHUB_TOKEN')]) {
            withEnv([
                "GITHUB_STATUS_COMMIT=${commit}",
                "GITHUB_STATUS_STATE=${state}",
                "GITHUB_STATUS_DESCRIPTION=${description}",
                "GITHUB_STATUS_CONTEXT=${env.GITHUB_STATUS_CONTEXT ?: 'jenkins/automation'}",
                "GITHUB_STATUS_TARGET_URL=${env.GITHUB_STATUS_TARGET_URL ?: 'https://boxedwine.org/builds/'}"
            ]) {
                sh '''#!/bin/bash
                    python3 - <<'PY'
import json
import os
import urllib.request

token = os.environ["GITHUB_TOKEN"]
commit = os.environ["GITHUB_STATUS_COMMIT"]
payload = json.dumps({
    "state": os.environ["GITHUB_STATUS_STATE"],
    "target_url": os.environ["GITHUB_STATUS_TARGET_URL"],
    "description": os.environ["GITHUB_STATUS_DESCRIPTION"],
    "context": os.environ["GITHUB_STATUS_CONTEXT"],
}).encode("utf-8")

request = urllib.request.Request(
    f"https://api.github.com/repos/danoon2/Boxedwine/statuses/{commit}",
    data=payload,
    headers={
        "Accept": "application/vnd.github+json",
        "Authorization": f"Bearer {token}",
        "Content-Type": "application/json",
        "User-Agent": "boxedwine-jenkins",
        "X-GitHub-Api-Version": "2022-11-28",
    },
    method="POST",
)

with urllib.request.urlopen(request, timeout=30) as response:
    print(f"Updated GitHub status: {response.status}")
PY
                '''
            }
        }
    }
}

pipeline {
    agent none
    options { 
        skipDefaultCheckout(true)
        // emscripten unit tests require this
        disableConcurrentBuilds()
    }
    stages {
        stage ('GitHub Status') {
            agent {
                label "linux64"
            }
            steps {
                script {
                    gitCheckout()
                    publishGithubBuildStatus('pending', 'Boxedwine build running')
                }
            }
        }
        stage ('Test') {
            parallel {
                stage ('Test Emscripten') {
                    agent {
                        label "emscripten"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        sh '''#!/bin/bash
                            source ~/emsdk/emsdk_env.sh
                            cd project/emscripten
                            make clean
                            make test
                            killall -9 python3
                            cd Build/Test
                            emrun --kill_start --kill_exit --browser="/usr/bin/firefox" --browser_args="--headless" boxedwine.html
                        '''
                        sh '''#!/bin/bash
                            source ~/emsdk/emsdk_env.sh
                            cd project/emscripten
                            make clean
                            make testJit
                            killall -9 python3
                            cd Build/TestJit
                            emrun --kill_start --kill_exit --browser="/usr/bin/firefox" --browser_args="--headless" boxedwine.html
                        '''
                    }
                }
                stage ('Test Linux (x64)') {
                    agent {
                        label "linux64"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                make clean
                                make test || exit
                                ./Build/Test/boxedwine || exit
                            '''
                        }
                    }
                }
                stage ('Test Mac (ARMv8)') {
                    agent {
                        label "macArmv8"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        sh '''#!/bin/bash
                            cd project/mac-xcode
                            /bin/bash buildTest.sh || exit
                            bin/BoxedwineTest.app/Contents/MacOS/BoxedwineTest
                        '''
                    }
                }
                stage ('Test Linux (ARMv8)') {
                    agent {
                        label "linuxArm64"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                make clean
                                make test || exit
                                ./Build/Test/boxedwine || exit
                            '''
                        }
                    }
                }
                stage ('Test Windows') {
                    agent {
                        label "windows"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        bat "\"C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\msbuild\" \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Test;Platform=win32"
                        bat "project\\msvc\\BoxedWine\\Test\\BoxedWine.exe"
                        bat "\"C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\msbuild\" \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Test;Platform=x64"
                        bat "project\\msvc\\BoxedWine\\x64\\Test\\BoxedWine.exe"
                    }
                }
                stage ('Test Windows ARM64') {
                    agent {
                        label "windowsARM64"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        bat "msbuild \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Test;Platform=ARM64"
                        bat "project\\msvc\\BoxedWine\\ARM64\\Test\\BoxedWine.exe"
                    }
                }  
            }
        }
        stage ('Build') {
            parallel {
                stage ('Build Emscripten') {
                    agent {
                        label "emscripten"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/emscripten") {
                            sh '''#!/bin/bash
                                source ~/emsdk/emsdk_env.sh
                                set -e
                                set -x
                                rm -rf Deploy
                                make clean

                                build_web_target() {
                                    local make_target="$1"
                                    local build_dir="$2"
                                    local deploy_name="$3"
                                    local deploy_dir="Deploy/Web/${deploy_name}"

                                    make "${make_target}"
                                    if [ ! -f "${build_dir}/boxedwine.wasm" ]
                                    then
                                        echo "${build_dir}/boxedwine.wasm DOES NOT exist."
                                        exit 999
                                    fi

                                    mkdir -p "${deploy_dir}"
                                    cp "${build_dir}/boxedwine.html" "${deploy_dir}"
                                    cp boxedwine.css "${deploy_dir}"
                                    cp boxedwine-shell.js "${deploy_dir}"
                                    cp "${build_dir}/boxedwine.js" "${deploy_dir}"
                                    cp "${build_dir}/boxedwine.wasm" "${deploy_dir}"
                                    cp /var/www/buildfiles/* "${deploy_dir}"
                                }

                                build_web_target release Build/Release SingleThreaded
                                build_web_target multiThreaded Build/MultiThreaded MultiThreaded
                                build_web_target jit Build/Jit SingleThreadedJit
                                build_web_target multiThreadedJit Build/MultiThreadedJit MultiThreadedJit
                            ''' 
                        }
                        dir("project/emscripten") {
                            stash includes: 'Deploy/Web/**', name: 'web'                            
                        }
                    }
                } 
                stage ('Build Linux (x64)') {
                    agent {
                        label "linux64"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                rm Build/Release/boxedwine
                                rm Build/Deploy/Linux64/boxedwine
                                make clean
                                make release
                                mkdir -p Build/Deploy/Linux64
                                if [ ! -f "Build/Release/boxedwine" ] 
                                then
                                    echo "Build/Release/boxedwine DOES NOT exists."
                                    exit 999
                                fi
                                cp Build/Release/boxedwine Build/Deploy/Linux64/
                            '''
                        }
                        
                        dir("project/linux/Build") {
                            stash includes: 'Deploy/Linux64/boxedwine', name: 'linux64'
                        }
                    }
                }
                stage ('Build Mac (ARMv8)') {
                    agent {
                        label "macArmv8"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/mac-xcode") {
                            sh '''#!/bin/bash
                                rm -rf bin/Boxedwine.app
                                rm -rf Deploy/Mac/Boxedwine.app
                                mkdir -p Deploy/Mac
                                /bin/bash buildRelease.sh
                                if [ ! -d "bin/Boxedwine.app" ] 
                                then
                                    echo "bin/Boxedwine.app DOES NOT exists."
                                    exit 999
                                fi
                                codesign --force --deep --verify --verbose --timestamp --sign "$BOXEDWINE_SIGN_NAME" --options runtime --entitlements ./Boxedwine/Boxedwine/Boxedwine.entitlements ./bin/Boxedwine.app
                                cd bin                                
                                /usr/bin/ditto -c -k --sequesterRsrc --keepParent "Boxedwine.app" "BoxedwineUpload.zip"
                                xcrun notarytool submit BoxedwineUpload.zip --keychain-profile "$BOXEDWINE_KEYCHAIN_PROFILE" --wait
                                xcrun stapler staple -v Boxedwine.app
                                rm BoxedwineUpload.zip
                                /usr/bin/ditto -c -k --sequesterRsrc --keepParent "Boxedwine.app" "Boxedwine.zip"
                                cd ..
                                mv bin/Boxedwine.zip Deploy/Mac/
                            '''
                        }
                        dir("project/mac-xcode") {
                            stash includes: 'Deploy/Mac/**', name: 'macArmv8'                            
                        }
                    }
                }
                stage ('Build Linux (ARMv8)') {
                    agent {
                        label "linuxArm64"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                rm Build/MultiThreaded/boxedwine
                                rm Deploy/LinuxArm64/boxedwine                                
                                mkdir -p Deploy/LinuxArm64
                                make clean
                                make release
                                if [ ! -f "Build/Release/boxedwine" ] 
                                then
                                    echo "Build/Release/boxedwine DOES NOT exists."
                                    exit 999
                                fi
                                mv Build/Release/boxedwine Deploy/LinuxArm64/
                            '''
                        }
                        dir("project/linux") {
                            stash includes: 'Deploy/LinuxArm64/boxedwine', name: 'linuxArm64'
                        }
                    }
                }
                stage ('Build Windows') {
                    agent {
                        label "windows"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        bat '''
                            IF EXIST "project\\msvc\\Boxedwine\\Release\\Boxedwine.exe" DEL "project\\msvc\\Boxedwine\\Release\\Boxedwine.exe"
                            IF EXIST "project\\msvc\\Boxedwine\\x64\\Release\\Boxedwine.exe" DEL "project\\msvc\\Boxedwine\\x64\\Release\\Boxedwine.exe"
                            IF NOT EXIST "project\\msvc\\Deploy\\Win32" mkdir "project\\msvc\\Deploy\\Win32"
                            if NOT EXIST "project\\msvc\\Deploy\\Win64" mkdir "project\\msvc\\Deploy\\Win64"
                        '''
                        bat "\"C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\msbuild\" \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Release;Platform=win32"
                        bat "\"C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\msbuild\" \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Release;Platform=x64"
                        bat '''
                            move project\\msvc\\Boxedwine\\Release\\Boxedwine.exe project\\msvc\\Deploy\\Win32\\
                            copy project\\msvc\\Deploy\\Win32\\Boxedwine.exe project\\msvc\\Deploy\\Win32\\Boxedwine_console.exe
                            editbin.exe /subsystem:console project\\msvc\\Deploy\\Win32\\Boxedwine_console.exe
                            move project\\msvc\\Boxedwine\\x64\\Release\\Boxedwine.exe project\\msvc\\Deploy\\Win64\\
                            copy project\\msvc\\Deploy\\Win64\\Boxedwine.exe project\\msvc\\Deploy\\Win64\\Boxedwine_console.exe
                            editbin.exe /subsystem:console project\\msvc\\Deploy\\Win64\\Boxedwine_console.exe
                        '''
                        dir("project/msvc") {
                            stash includes: 'Deploy/**/*', name: 'windows'
                        }
                    }
                } 
                stage ('Build Windows ARM64') {
                    agent {
                        label "windowsARM64"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        bat '''
                            IF EXIST "project\\msvc\\Boxedwine\\ARM64\\Release\\Boxedwine.exe" DEL "project\\msvc\\Boxedwine\\ARM64\\Release\\Boxedwine.exe"
                            if NOT EXIST "project\\msvc\\Deploy\\WinARM64" mkdir "project\\msvc\\Deploy\\WinARM64"
                        '''
                        bat "msbuild \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Release;Platform=ARM64"
                        bat '''
                            move project\\msvc\\Boxedwine\\ARM64\\Release\\Boxedwine.exe project\\msvc\\Deploy\\WinARM64\\
                        '''
                        dir("project/msvc") {
                            stash includes: 'Deploy/**/*', name: 'windowsARM64'
                        }
                    }
                } 
            }
        }
        stage ('Automation') {
            parallel {
                stage ('Emscripten AbiWord Automation') {
                    agent {
                        label "emscripten"
                    }
                    steps {
                        script {
                            gitCheckout()
                        }
                        sh '''#!/bin/bash
                            source ~/emsdk/emsdk_env.sh
                            export DISPLAY=:0
                            export XAUTHORITY="$HOME/.Xauthority"
                            cd project/emscripten
                            set -euo pipefail

                            ABIWORD_AUTO_URL='https://boxedwine.org/v2/1/abiword_auto_v1.zip'
                            ABIWORD_AUTO_SHA256='80dc5a5f99f23637e4eb29b72e4f5484e9c03d697e72e4e4777574985b351181'
                            BOXEDWINE_AUTO_URL='https://boxedwine.org/v2/1/boxedwine_v1.zip'
                            BOXEDWINE_AUTO_SHA256='67742f667f989083a327d4405f7e4cd59cacea061a5936dbedcc5d47898de4d9'

                            file_matches_sha256() {
                                local file="$1"
                                local expected_sha="$2"
                                [ -f "$file" ] && printf '%s  %s\\n' "$expected_sha" "$file" | sha256sum -c - >/dev/null 2>&1
                            }

                            download_checked() {
                                local url="$1"
                                local output="$2"
                                local expected_sha="$3"
                                local tmp="${output}.tmp"

                                if file_matches_sha256 "$output" "$expected_sha"; then
                                    echo "$output already exists and matches expected SHA-256"
                                    return 0
                                fi

                                echo "Downloading $output from $url"
                                rm -f "$tmp"
                                curl -fL --retry 3 --retry-delay 5 --connect-timeout 30 "$url" -o "$tmp"
                                printf '%s  %s\\n' "$expected_sha" "$tmp" | sha256sum -c -
                                mv "$tmp" "$output"
                            }

                            download_checked "$ABIWORD_AUTO_URL" abiword_auto.zip "$ABIWORD_AUTO_SHA256"
                            download_checked "$BOXEDWINE_AUTO_URL" boxedwine.zip "$BOXEDWINE_AUTO_SHA256"

                            make clean
                            make automationJit

                            last_rc=1
                            for attempt in 1 2 3
                            do
                                echo "AbiWord Emscripten automation attempt ${attempt}/3"
                                killall -9 python3 2>/dev/null || true
                                killall -9 chrome 2>/dev/null || true

                                cd Build/AutomationJit
                                set +e
                                emrun --kill_start --kill_exit --port 6932 --timeout 600 --timeout-returncode 124 --browser="/usr/bin/google-chrome" boxedwine.html?root=boxedwine\\&overlay=abiword_auto\\&w=%2Ffiles\\&play=%2Ffiles%2Fscript.txt\\&p=ABIWORD.EXE\\&resolution=1024x768\\&storage=memory
                                rc=$?
                                set -e
                                cd ../..

                                if [ "$rc" = "111" ]; then
                                    echo "AbiWord Emscripten automation passed"
                                    exit 0
                                fi

                                last_rc="$rc"
                                echo "AbiWord Emscripten automation attempt ${attempt}/3 failed with exit code ${rc}"
                            done

                            echo "AbiWord Emscripten automation failed after 3 attempts"
                            if [ "$last_rc" = "0" ]; then
                                exit 1
                            fi
                            exit "$last_rc"
                        '''
                    }
                }
                stage ('Linux (x64) Automation') {
                    agent {
                        label "linux64"
                    }
                    steps {
                        dir("project/linux") {                                                        
                            sh '''#!/bin/bash
                                wget -N --no-if-modified-since -np http://boxedwine.org/v2/1/automation31.zip
                                rm -rf automation
                                unzip automation31.zip
                            '''
                        }
                        dir("project/linux/automation") {
                            unstash "linux64"
                            retry(3) {
                                sh '''
                                    java -jar bin/BoxedWineRunner.jar \"$WORKSPACE/project/linux/automation/fs/fs.zip\" \"$WORKSPACE/project/linux/automation/scripts/" \"$WORKSPACE/project/linux/automation/Deploy/Linux64/boxedwine\" -nosound -novideo
                                '''
                            }
                            retry(3) {
                                sh '''
                                    java -jar bin/BoxedWineRunner.jar -name \"Cinebench-Linux-x64\" \"$WORKSPACE/project/linux/automation/fs/fs.zip\" \"$WORKSPACE/project/linux/automation/perfScripts/cinebench/" \"$WORKSPACE/project/linux/automation/Deploy/Linux64/boxedwine\" -nosound -novideo
                                '''
                            }
                            stash includes: 'perfScripts/cinebench/cinebench/perf-Cinebench-Linux-x64.csv', name: 'linux64Perf'
                        }                        
                    }
                }
                stage ('Mac Automation (ARMv8)') {
                    agent {
                        label "macArmv8"
                    }
                    steps {
                        dir("project/mac-xcode") {
                            sh '''#!/bin/bash
                                curl -z automation31.zip http://boxedwine.org/v2/1/automation31.zip --output automation31.zip
                                rm -rf automation
                                unzip automation31.zip

                                rm -rf bin/BoxedwineAutomation.app
                                /bin/bash buildAutomation.sh
                                if [ ! -d "bin/BoxedwineAutomation.app" ] 
                                then
                                    echo "bin/BoxedwineAutomation.app DOES NOT exists."
                                    exit 999
                                fi
                            '''
                        }
                        
                        dir("project/mac-xcode/automation") {
                            retry(3) {
                                sh '''#!/bin/bash    
                                    java -jar bin/BoxedWineRunner.jar \"$WORKSPACE/project/mac-xcode/automation/fs/fs.zip\" \"$WORKSPACE/project/mac-xcode/automation/scripts/" \"$WORKSPACE/project/mac-xcode/bin/BoxedwineAutomation.app/Contents/MacOS/BoxedwineAutomation\" -nosound -novideo || exit 1
                                '''
                            }
                            retry(3) {
                                sh '''#!/bin/bash    
                                    java -jar bin/BoxedWineRunner.jar -name \"Cinebench-MacOSX\" \"$WORKSPACE/project/mac-xcode/automation/fs/fs.zip\" \"$WORKSPACE/project/mac-xcode/automation/perfScripts/cinebench/" \"$WORKSPACE/project/mac-xcode/bin/BoxedwineAutomation.app/Contents/MacOS/BoxedwineAutomation\" -nosound -novideo || exit 1
                                '''
                            }
                            stash includes: 'perfScripts/cinebench/cinebench/perf-Cinebench-MacOSX.csv', name: 'macArmv8Perf'
                        }                        
                    }
                }
                stage ('Linux (ARMv8) Automation') {
                    agent {
                        label "linuxArm64"
                    }
                    steps {
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                wget -N --no-if-modified-since -np http://boxedwine.org/v2/1/automation31.zip
                                rm -rf automation
                                unzip automation31.zip
                            '''
                        }
                        dir("project/linux/automation") {
                            unstash "linuxArm64"
                            retry(3) {
                                sh '''#!/bin/bash
                                    java -jar bin/BoxedWineRunner.jar \"$WORKSPACE/project/linux/automation/fs/fs.zip\" \"$WORKSPACE/project/linux/automation/scripts/" \"$WORKSPACE/project/linux/automation/Deploy/LinuxArm64/boxedwine\" -nosound -novideo || exit 1
                                '''
                            }
                            retry(3) {
                                sh '''#!/bin/bash
                                    java -jar bin/BoxedWineRunner.jar -name \"Cinebench-Linux-Arm64\" \"$WORKSPACE/project/linux/automation/fs/fs.zip\" \"$WORKSPACE/project/linux/automation/perfScripts/cinebench/" \"$WORKSPACE/project/linux/automation/Deploy/LinuxArm64/boxedwine\" -nosound -novideo || exit 1
                                '''
                            }
                            stash includes: 'perfScripts/cinebench/cinebench/perf-Cinebench-Linux-Arm64.csv', name: 'linuxArm64Perf'
                        }                        
                    }
                }
                stage ('Windows Automation') {
                    agent {
                        label "windows"
                    }
                    steps {
                        bat '''
                            wget -N --no-if-modified-since -np http://boxedwine.org/v2/1/automation31.zip
                            IF EXIST "automation" rmdir /q /s "automation"
                            unzip automation31.zip
                        '''
                        dir("automation") {
                            unstash "windows"
                            retry(3) {
                                bat '''
                                    java -jar bin\\BoxedWineRunner.jar -name \"Cinebench-Win32\" \"%WORKSPACE%\\automation\\fs\\fs.zip\" \"%WORKSPACE%\\automation\\perfScripts\\cinebench\" \"%WORKSPACE%\\automation\\Deploy\\Win32\\Boxedwine.exe\" -nosound -novideo
                                '''
                            }
                            retry(3) {
                                bat '''
                                    java -jar bin\\BoxedWineRunner.jar \"%WORKSPACE%\\automation\\fs\\fs.zip\" \"%WORKSPACE%\\automation\\scripts\" \"%WORKSPACE%\\automation\\Deploy\\Win32\\Boxedwine.exe\" -nosound -novideo
                                '''
                            }
                            retry(3) {
                                bat '''
                                    java -jar bin\\BoxedWineRunner.jar -name \"Cinebench-Win64\" \"%WORKSPACE%\\automation\\fs\\fs.zip\" \"%WORKSPACE%\\automation\\perfScripts\\cinebench\" \"%WORKSPACE%\\automation\\Deploy\\Win64\\Boxedwine.exe\" -nosound -novideo
                                '''
                            }
                            retry(3) {
                                bat '''
                                    java -jar bin\\BoxedWineRunner.jar \"%WORKSPACE%\\automation\\fs\\fs.zip\" \"%WORKSPACE%\\automation\\scripts\" \"%WORKSPACE%\\automation\\Deploy\\Win64\\Boxedwine.exe\" -nosound -novideo
                                '''
                            }
                            stash includes: 'perfScripts/cinebench/cinebench/perf*.csv', name: 'windowsPerf'
                        }                        
                    }
                } 
                stage ('Windows ARM64 Automation') {
                    agent {
                        label "windowsARM64"
                    }
                    steps {
                        bat '''
                            wget -N --no-if-modified-since -np http://boxedwine.org/v2/1/automation31.zip
                            IF EXIST "automation" rmdir /q /s "automation"
                            tar -xf automation31.zip
                        '''
                        dir("automation") {
                            unstash "windowsARM64"
                            retry(3) {
                                bat '''
                                    java -jar bin\\BoxedWineRunner.jar -name \"Cinebench-WinArm64\" \"%WORKSPACE%\\automation\\fs\\fs.zip\" \"%WORKSPACE%\\automation\\perfScripts\\cinebench\" \"%WORKSPACE%\\automation\\Deploy\\WinARM64\\Boxedwine.exe\" -nosound -novideo
                                '''
                            }
                            retry(3) {
                                bat '''
                                    java -jar bin\\BoxedWineRunner.jar \"%WORKSPACE%\\automation\\fs\\fs.zip\" \"%WORKSPACE%\\automation\\scripts\" \"%WORKSPACE%\\automation\\Deploy\\WinARM64\\Boxedwine.exe\" -nosound -novideo
                                '''
                            }
                            stash includes: 'perfScripts/cinebench/cinebench/perf-Cinebench-WinArm64.csv', name: 'windowsARM64Perf'
                        }                        
                    }                   
                }
            }
        }
        stage ('Performance') {
            agent {
                label "linux64"
            }
            steps {
                script {
                    dir("automation") {
                        unstash "windowsARM64Perf"
                        unstash "windowsPerf"
                        unstash "linuxArm64Perf"
                        unstash "linux64Perf"
                        unstash "macArmv8Perf"
                        def csvWinArm64 = readCSV file: 'perfScripts/cinebench/cinebench/perf-Cinebench-WinArm64.csv'
                        def csvWin64 = readCSV file: 'perfScripts/cinebench/cinebench/perf-Cinebench-Win64.csv'
                        def csvWin32 = readCSV file: 'perfScripts/cinebench/cinebench/perf-Cinebench-Win32.csv'
                        def csvLinuxArm64 = readCSV file: 'perfScripts/cinebench/cinebench/perf-Cinebench-Linux-Arm64.csv'
                        def csvMac = readCSV file: 'perfScripts/cinebench/cinebench/perf-Cinebench-MacOSX.csv'
                        def csvLinux64 = readCSV file: 'perfScripts/cinebench/cinebench/perf-Cinebench-Linux-x64.csv'

                        def records = [['Win32', 'Win64', 'WinArm64', 'Linux64', 'LinuxArm64', 'Mac'], [csvWin32[1][0], csvWin64[1][0], csvWinArm64[1][0], csvLinux64[1][0], csvLinuxArm64[1][0], csvMac[1][0]]]
                        writeCSV file: 'cinebench.csv', records: records
                    }
                }
                plot csvFileName: 'plot.csv', csvSeries: [[displayTableFlag: false, exclusionValues: '', file: 'automation/cinebench.csv', inclusionFlag: 'OFF', url: '']], group: 'Performance', style: 'line', title: 'Cinebench 11.5'
            }
        }
    }    
    post {
        always {
            node('linux64') {
                dir("project/linux") {
                    sh '''#!/bin/bash
                        rm -rf Deploy
                    '''
                    unstash "web"
                    unstash "linux64"
                    unstash "macArmv8"
                    unstash "linuxArm64"
                    unstash "windows"
                    unstash "windowsARM64"
                    dir('Deploy') {
                        sh '''
                        echo "Linux64, LinuxArm64 and Win64 use the binary translator CPU core and are much faster.  The others use the normal core or normal core + JIT." > readme.txt
                        zip -r build-$BUILD_NUMBER.zip *
                        '''
                        archiveArtifacts artifacts: "build-${env.BUILD_NUMBER}.zip", fingerprint: true, allowEmptyArchive: true
                    }
                    script {
                        withFolderProperties {
                            withEnv([
                                "BUILD_RESULT=${currentBuild.currentResult ?: 'SUCCESS'}",
                                "GIT_COMMIT=${env.GIT_COMMIT ?: env.BOXEDWINE_GIT_COMMIT ?: ''}",
                                "BUILD_SITE_ARTIFACT=${env.WORKSPACE}/project/linux/Deploy/build-${env.BUILD_NUMBER}.zip"
                            ]) {
                                sh "bash ${env.WORKSPACE}/tools/jenkins/publish-build-site.sh"
                            }
                        }
                    }
                }
            }
        }
        changed {
            script {
                if ("${env.BRANCH_NAME}" == 'master') {
                    emailext subject: '$DEFAULT_SUBJECT',
                        body: '$DEFAULT_CONTENT',
                        recipientProviders: [
                            [$class: 'CulpritsRecipientProvider'],
                            [$class: 'DevelopersRecipientProvider'],
                            [$class: 'RequesterRecipientProvider']
                        ], 
                        replyTo: '$DEFAULT_REPLYTO',
                        to: '$DEFAULT_RECIPIENTS'
                }
            }
        }
        success {
            script {
                node('linux64') {
                    publishGithubBuildStatus('success', 'Boxedwine build passed')
                }
                emailext subject: '$DEFAULT_SUBJECT',
                    body: '$DEFAULT_CONTENT',
                    recipientProviders: [
                        [$class: 'CulpritsRecipientProvider'],
                        [$class: 'RequesterRecipientProvider']
                    ], 
                    replyTo: '$DEFAULT_REPLYTO'
            }
        }
        failure {
            script {
                node('linux64') {
                    publishGithubBuildStatus('failure', 'Boxedwine build failed')
                }
                emailext subject: '$DEFAULT_SUBJECT',
                    body: '$DEFAULT_CONTENT',
                    recipientProviders: [
                        [$class: 'CulpritsRecipientProvider'],
                        [$class: 'RequesterRecipientProvider']
                    ], 
                    replyTo: '$DEFAULT_REPLYTO'
            }
        }
        unstable {
            script {
                node('linux64') {
                    publishGithubBuildStatus('failure', 'Boxedwine build unstable')
                }
            }
        }
        aborted {
            script {
                node('linux64') {
                    publishGithubBuildStatus('error', 'Boxedwine build aborted')
                }
            }
        }
    }
}
