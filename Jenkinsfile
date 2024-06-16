// Notes:
// Windows build: put wget, msbuild and build tools in path (C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\bin\Hostx64\x64)
void gitCheckout() {
    def retryAttempt = 0
    retry(3) {
        if (retryAttempt > 0) {
            sleep(120  * retryAttempt)
        }
        checkout scm
    }
}

pipeline {
    agent none
    options { 
        skipDefaultCheckout(true) 
    }
    stages {
        stage ('Test') {
            parallel {
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
                                make test || exit
                                ./Build/Test/boxedwine || exit
                                make testMultiThreaded || exit
                                ./Build/TestMultiThreaded/boxedwine
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
                stage ('Test Mac (x86)') {
                    agent {
                        label "mac"
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
                stage ('Test Raspberry Pi (ARMv7)') {
                    agent {
                        label "raspberry"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                make testJit || exit
                                ./Build/TestJit/boxedwine
                            '''
                        }
                    }
                }
                stage ('Test Raspberry Pi (ARMv8)') {
                    agent {
                        label "raspberry64"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                make testMultiThreaded || exit
                                ./Build/TestMultiThreaded/boxedwine
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
                        bat "\"${env.MSBUILD}\" \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Test;Platform=win32"
                        bat "project\\msvc\\BoxedWine\\Test\\BoxedWine.exe"
                        bat "\"${env.MSBUILD}\" \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Test;Platform=x64"
                        bat "project\\msvc\\BoxedWine\\x64\\Test\\BoxedWine.exe"
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
                                set -x
                                rm boxedwine.wasm
                                rm -rf Deploy
                                sh buildjs.sh
                                if [ ! -f "boxedwine.wasm" ] 
                                then
                                    echo "boxedwine.wasm DOES NOT exists."
                                    exit 999
                                fi
                                mkdir -p Deploy/Web
                                cp boxedwine.html Deploy/Web
                                cp boxedwine.css Deploy/Web
                                cp boxedwine-shell.js Deploy/Web
                                cp boxedwine.js Deploy/Web
                                cp jszip.min.js Deploy/Web
                                cp browserfs.boxedwine.js Deploy/Web
                                cp boxedwine.wasm Deploy/Web
                                cp /var/www/buildfiles/* Deploy/Web
                            ''' 
                        }
                        dir("project/emscripten") {
                            stash includes: 'Deploy/Web/*', name: 'web'                            
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
                                rm Build/MultiThreaded/boxedwine
                                rm Build/Release/boxedwine
                                rm Build/Deploy/Linux64/boxedwine
                                make release
                                make multiThreaded
                                mkdir -p Build/Deploy/Linux64
                                mkdir -p Build/Deploy/Linux
                                if [ ! -f "Build/MultiThreaded/boxedwine" ] 
                                then
                                    echo "Build/MultiThreaded/boxedwine DOES NOT exists."
                                    exit 999
                                fi
                                if [ ! -f "Build/Release/boxedwine" ] 
                                then
                                    echo "Build/Release/boxedwine DOES NOT exists."
                                    exit 999
                                fi
                                cp Build/MultiThreaded/boxedwine Build/Deploy/Linux64/
                                cp Build/Release/boxedwine Build/Deploy/Linux/
                            '''
                        }
                        
                        dir("project/linux/Build") {
                            stash includes: 'Deploy/Linux64/boxedwine', name: 'linux64'
                            stash includes: 'Deploy/Linux/boxedwine', name: 'linux'
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
                                mv bin/Boxedwine.app/ Deploy/Mac/
                            '''
                        }
                        dir("project/mac-xcode") {
                            stash includes: 'Deploy/Mac/**', name: 'macArmv8'                            
                        }
                    }
                }
                stage ('Build Mac (x86)') {
                    agent {
                        label "mac"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/mac-xcode") {
                            sh '''#!/bin/bash
                                rm -rf bin/Boxedwine.app
                                rm -rf Deploy/MacIntel/Boxedwine.app
                                mkdir -p Deploy/MacIntel
                                /bin/bash buildRelease.sh
                                if [ ! -d "bin/Boxedwine.app" ] 
                                then
                                    echo "bin/Boxedwine.app DOES NOT exists."
                                    exit 999
                                fi
                                mv bin/Boxedwine.app/ Deploy/MacIntel/
                            '''
                        }
                        dir("project/mac-xcode") {
                            stash includes: 'Deploy/MacIntel/**', name: 'mac'                            
                        }
                    }
                }
                stage ('Build Raspberry Pi (ARMv7)') {
                    agent {
                        label "raspberry"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                rm Build/Jit/boxedwine
                                rm Deploy/RaspberryPi/boxedwine
                                mkdir -p Deploy/RaspberryPi
                                make jit
                                if [ ! -f "Build/Jit/boxedwine" ] 
                                then
                                    echo "Build/Jit/boxedwine DOES NOT exists."
                                    exit 999
                                fi
                                mv Build/Jit/boxedwine Deploy/RaspberryPi/
                            '''
                        }
                        dir("project/linux") {
                            stash includes: 'Deploy/RaspberryPi/boxedwine', name: 'raspberry'
                        }
                    }
                }
                stage ('Build Raspberry Pi (ARMv8)') {
                    agent {
                        label "raspberry64"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                rm Build/MultiThreaded/boxedwine
                                rm Deploy/RaspberryPi64/boxedwine
                                mkdir -p Deploy/RaspberryPi64
                                make multiThreaded
                                if [ ! -f "Build/MultiThreaded/boxedwine" ] 
                                then
                                    echo "Build/MultiThreaded/boxedwine DOES NOT exists."
                                    exit 999
                                fi
                                mv Build/MultiThreaded/boxedwine Deploy/RaspberryPi64/
                            '''
                        }
                        dir("project/linux") {
                            stash includes: 'Deploy/RaspberryPi64/boxedwine', name: 'raspberry64'
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
                        bat "\"${env.MSBUILD}\" \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Release;Platform=win32"
                        bat "\"${env.MSBUILD}\" \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Release;Platform=x64"
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
            }
        }
        stage ('Automation') {
            parallel {
                stage ('Linux (x64) Automation') {
                    agent {
                        label "linux64"
                    }
                    steps {
                        dir("project/linux") {                                                        
                            sh '''#!/bin/bash
                                wget -N --no-if-modified-since -np http://boxedwine.org/v/automation2.zip
                                rm -rf automation
                                unzip automation.zip
                            '''
                        }
                        dir("project/linux/automation") {
                            unstash "linux64"
                            unstash "linux"
                            retry(3) {
                                sh '''
                                    java -jar bin/BoxedWineRunner.jar \"$WORKSPACE/project/linux/automation/fs/Wine-5.0.zip\" \"$WORKSPACE/project/linux/automation/scripts/" \"$WORKSPACE/project/linux/automation/Deploy/Linux64/boxedwine\" -nosound -novideo
                                '''
                            }
                            retry(3) {
                                sh '''
                                    java -jar bin/BoxedWineRunner.jar \"$WORKSPACE/project/linux/automation/fs/Wine-5.0.zip\" \"$WORKSPACE/project/linux/automation/scripts/" \"$WORKSPACE/project/linux/automation/Deploy/Linux/boxedwine\" -nosound -novideo
                                '''
                            }
                        }
                    }
                }
                stage ('Mac Automation (ARMv8)') {
                    agent {
                        label "macArmv8"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }
                        dir("project/mac-xcode") {
                            sh '''#!/bin/bash
                                curl -z automation.zip http://boxedwine.org/v/automation2.zip --output automation.zip
                                rm -rf automation
                                unzip automation.zip

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
                                    java -jar bin/BoxedWineRunner.jar \"$WORKSPACE/project/mac-xcode/automation/fs/Wine-5.0.zip\" \"$WORKSPACE/project/mac-xcode/automation/scripts/" \"$WORKSPACE/project/mac-xcode/bin/BoxedwineAutomation.app/Contents/MacOS/BoxedwineAutomation\" -nosound -novideo || exit 1
                                '''
                            }
                        }
                    }
                }
                stage ('Mac Automation (x86)') {
                    agent {
                        label "mac"
                    }
                    steps {
                        script { 
                            gitCheckout() 
                        }                      
                        dir("project/mac-xcode") {
                            sh '''#!/bin/bash
                                curl -z automation.zip http://boxedwine.org/v/automation2.zip --output automation.zip
                                rm -rf automation
                                unzip automation.zip

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
                                    java -jar bin/BoxedWineRunner.jar \"$WORKSPACE/project/mac-xcode/automation/fs/Wine-5.0.zip\" \"$WORKSPACE/project/mac-xcode/automation/scripts/" \"$WORKSPACE/project/mac-xcode/bin/BoxedwineAutomation.app/Contents/MacOS/BoxedwineAutomation\" -nosound -novideo || exit 1
                                '''
                            }
                        }
                    }
                }
                stage ('Raspberry Pi (ARMv7) Automation') {
                    agent {
                        label "raspberry"
                    }
                    options {
                        timeout(time: 20, unit: 'MINUTES')   // timeout on this stage
                    }
                    steps {
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                wget -N --no-if-modified-since -np http://boxedwine.org/v/automation2.zip
                                rm -rf automation
                                unzip automation.zip
                            '''
                        }
                        dir("project/linux/automation") {
                            unstash "raspberry"
                            retry(3) {
                                sh '''#!/bin/bash
                                    java -jar bin/BoxedWineRunner.jar \"$WORKSPACE/project/linux/automation/fs/Wine-5.0.zip\" \"$WORKSPACE/project/linux/automation/scripts/" \"$WORKSPACE/project/linux/automation/Deploy/RaspberryPi/boxedwine\" -nosound -novideo || exit 1
                                '''
                            }
                        }

                    }
                }
                stage ('Raspberry Pi (ARMv8) Automation') {
                    agent {
                        label "raspberry64"
                    }
                    options {
                        timeout(time: 20, unit: 'MINUTES')   // timeout on this stage
                    }
                    steps {
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                wget -N --no-if-modified-since -np http://boxedwine.org/v/automation2.zip
                                rm -rf automation
                                unzip automation.zip
                            '''
                        }
                        dir("project/linux/automation") {
                            unstash "raspberry64"
                            retry(3) {
                                sh '''#!/bin/bash
                                    java -jar bin/BoxedWineRunner.jar \"$WORKSPACE/project/linux/automation/fs/Wine-5.0.zip\" \"$WORKSPACE/project/linux/automation/scripts/" \"$WORKSPACE/project/linux/automation/Deploy/RaspberryPi64/boxedwine\" -nosound -novideo || exit 1
                                '''
                            }
                        }

                    }
                }
                stage ('Windows Automation') {
                    agent {
                        label "windows"
                    }
                    steps {
                        bat '''
                            wget -N --no-if-modified-since -np http://boxedwine.org/v/automation2.zip
                            IF EXIST "automation" rmdir /q /s "automation"
                            unzip automation.zip
                        '''
                        dir("automation") {
                            unstash "windows"
                            retry(3) {
                                bat '''
                                    java -jar bin\\BoxedWineRunner.jar \"%WORKSPACE%\\automation\\fs\\Wine-5.0.zip\" \"%WORKSPACE%\\automation\\scripts\" \"%WORKSPACE%\\automation\\Deploy\\Win32\\Boxedwine.exe\" -nosound -novideo
                                '''
                            }
                            retry(3) {
                                bat '''
                                    java -jar bin\\BoxedWineRunner.jar \"%WORKSPACE%\\automation\\fs\\Wine-5.0.zip\" \"%WORKSPACE%\\automation\\scripts\" \"%WORKSPACE%\\automation\\Deploy\\Win64\\Boxedwine.exe\" -nosound -novideo
                                '''
                            }
                        }
                    }
                } 
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
                    unstash "raspberry"
                    unstash "raspberry64"
                    unstash "windows"
                    dir('Deploy') {
                        sh '''
                        echo "Linux64, Raspberry64 (experimental and buggy) and Win64 use the binary translator CPU core and are much faster.  The others use the normal core or normal core + JIT." > readme.txt
                        zip -r build-$BUILD_NUMBER.zip *
                        '''
                        archiveArtifacts artifacts: "build-${env.BUILD_NUMBER}.zip", fingerprint: true, allowEmptyArchive: true
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
                emailext subject: '$DEFAULT_SUBJECT',
                    body: '$DEFAULT_CONTENT',
                    recipientProviders: [
                        [$class: 'CulpritsRecipientProvider'],
                        [$class: 'RequesterRecipientProvider']
                    ], 
                    replyTo: '$DEFAULT_REPLYTO'
            }
        }
    }
}