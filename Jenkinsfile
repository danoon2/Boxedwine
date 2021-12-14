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
                stage ('Checkout Mac (x64)') {
                    agent {
                        label "mac"
                    }
                    steps {
                        checkout scm
                    }
                }
                stage ('Checkout Mac (ARMv8)') {
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
                stage ('Test Mac (x64)') {
                    agent {
                        label "mac"
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
                stage ('Test Mac (ARMv8)') {
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
                        sh '''#!/bin/bash
                            source ~/emsdk/emsdk_env.sh
                            cd project/emscripten
                            sh buildjs.sh
                        ''' 
                    }
                } 
                stage ('Build Linux (x64)') {
                    agent {
                        label "linux64"
                    }
                    steps {
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                make release || exit
                                make multiThreaded
                            '''
                        }
                    }
                }
                stage ('Build Mac (x64)') {
                    agent {
                        label "mac"
                    }
                    steps {
                        sh '''#!/bin/bash
                            export PATH=/bin:/usr/bin:/usr/local/bin:/Users/alla/homebrew/bin
                            cd project/mac-xcode
                            /bin/bash buildRelease.sh
                        ''' 
                    }
                }
                stage ('Build Mac (ARMv8)') {
                    agent {
                        label "macArmv8"
                    }
                    steps {
                        sh '''#!/bin/bash
                            export PATH=/bin:/usr/bin:/usr/local/bin:/Users/alla/homebrew/bin
                            cd project/mac-xcode
                            /bin/bash buildRelease.sh
                        '''
                    }
                }
                stage ('Build Raspberry Pi (ARMv7)') {
                    agent {
                        label "raspberry"
                    }
                    steps {
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                make jit
                            '''
                        }
                    }
                }
                stage ('Build Windows') {
                    agent {
                        label "windows"
                    }
                    steps {
                        bat "\"${env.MSBUILD}\" \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Release;Platform=win32"
                        bat "\"${env.MSBUILD}\" \"project/msvc/BoxedWine/BoxedWine.sln\" /p:Configuration=Release;Platform=x64"
                    }
                }      
            }
        }
    }

    post {
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