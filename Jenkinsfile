pipeline {
    agent none
    stages {
        stage ('Checkout') {
            parallel {     
                stage ('Checkout Emscripten') {
                    agent {
                        label "linux"
                    }
                    steps {
                        checkout scm
                    }
                }
                stage ('Checkout Linux') {
                    agent {
                        label "linux"
                    }
                    steps {
                        checkout scm
                    }
                }
                stage ('Checkout Mac (x86)') {
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
                stage ('Checkout Raspberry Pi') {
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
                        label "linux"
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
                stage ('Test Linux') {
                    agent {
                        label "linux"
                    }
                    steps {
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                make test || exit
                                ./Build/Test/boxedwine
                            '''
                        }
                    }
                }
                stage ('Test Mac (x86)') {
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
                stage ('Test Raspberry Pi') {
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
                    }
                }                    
            }
        }
        stage ('Build') {
            parallel {
                stage ('Build Emscripten') {
                    agent {
                        label "linux"
                    }
                    steps {
                        sh '''#!/bin/bash
                            source ~/emsdk/emsdk_env.sh
                            cd project/emscripten
                            sh buildjs.sh
                        ''' 
                    }
                } 
                stage ('Build Linux') {
                    agent {
                        label "linux"
                    }
                    steps {
                        dir("project/linux") {
                            sh '''#!/bin/bash
                                make release
                            '''
                        }
                    }
                }
                stage ('Build Mac (x86)') {
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
                stage ('Build Raspberry Pi') {
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
                    }
                }      
            }
        }
    }
}