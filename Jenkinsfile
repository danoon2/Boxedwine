pipeline {
    agent none
    stages {
        stage ('Checkout') {
            parallel {     
                stage ('Checkout Emscripten') {
                    agent {
                        label "emscripten"
                    }
                    steps {
                        checkout scm
                    }
                }
                stage ('Checkout Linux (x64)') {
                    agent {
                        label "linux64"
                    }
                    steps {
                        checkout scm
                    }
                }
                stage ('Checkout Mac') {
                    agent {
                        label "macArmv8"
                    }
                    steps {
                        checkout scm
                    }
                }
                stage ('Checkout Raspberry Pi (ARMv7)') {
                    agent {
                        label "raspberry"
                    }
                    steps {
                        checkout scm
                    }
                }
                stage ('Checkout Windows') {
                    agent {
                        label "windows"
                    }
                    steps {
                        checkout scm
                    }
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
                        sh '''#!/bin/bash
                            source ~/emsdk/emsdk_env.sh
                            cd project/emscripten
                            sh buildtestjs.sh
                            emrun --kill_start --browser "firefox" --browser_args="--headless" boxedwineTest.html
                        '''
                    }
                }
                stage ('Test Linux (x64)') {
                    agent {
                        label "linux64"
                    }
                    steps {
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
                stage ('Test Mac') {
                    agent {
                        label "macArmv8"
                    }
                    steps {
                        sh '''#!/bin/bash
                            export PATH=/bin:/usr/bin:/usr/local/bin:/Users/alla/homebrew/bin
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
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                make testJit || exit
                                ./Build/TestJit/boxedwine
                            '''
                        }
                    }
                }
                stage ('Test Windows') {
                    agent {
                        label "windows"
                    }
                    steps {
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
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                rm Build/MultiThreaded/boxedwine
                                rm Build/Release/boxedwine
                                rm Build/Deploy/Linux64/boxedwine
                                make release || exit
                                make multiThreaded
                                mkdir -p Build/Deploy/Linux64
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
                            '''
                        }
                        
                        dir("project/linux/Build") {
                            stash includes: 'Deploy/Linux64/boxedwine', name: 'linux64'                            
                        }
                    }
                }
                stage ('Build Mac') {
                    agent {
                        label "macArmv8"
                    }
                    steps {
                        dir("project/mac-xcode") {
                            sh '''#!/bin/bash
                                rm -rf bin/Boxedwine.app
                                rm -rf Deploy/Mac/Boxedwine.app
                                mkdir -p Deploy/Mac
                                export PATH=/bin:/usr/bin:/usr/local/bin:/Users/alla/homebrew/bin
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
                stage ('Build Raspberry Pi (ARMv7)') {
                    agent {
                        label "raspberry"
                    }
                    steps {
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
                stage ('Build Windows') {
                    agent {
                        label "windows"
                    }
                    steps {
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
                            move project\\msvc\\Boxedwine\\x64\\Release\\Boxedwine.exe project\\msvc\\Deploy\\Win64\\
                        '''
                        dir("project/msvc") {
                            stash includes: 'Deploy/**/*', name: 'windows'
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
                    unstash "windows"
                    dir('Deploy') {
                        sh '''
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