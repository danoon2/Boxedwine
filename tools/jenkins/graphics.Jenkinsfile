// Opt-in Pipeline job: configure the dedicated Windows GPU worker and input
// file before selecting this script as the job's Pipeline from SCM path.
pipeline {
    agent { label 'boxedwine-graphics-windows' }
    options {
        disableConcurrentBuilds()
        timeout(time: 72, unit: 'HOURS')
        timestamps()
    }
    triggers { cron('H H * * 0') }
    environment {
        GRAPHICS_CI_OUTPUT = "graphics-ci/${env.BUILD_NUMBER}"
    }
    stages {
        stage('Full Wine graphics matrix') {
            steps {
                powershell '''
                    $ErrorActionPreference = 'Stop'
                    if ([string]::IsNullOrWhiteSpace($env:BOXEDWINE_GRAPHICS_CONFIG)) {
                        throw 'Set BOXEDWINE_GRAPHICS_CONFIG on the dedicated worker.'
                    }
                    $graphicsPython = $env:BOXEDWINE_GRAPHICS_PYTHON
                    if ([string]::IsNullOrWhiteSpace($graphicsPython)) {
                        $graphicsPython = 'python'
                    }
                    & $graphicsPython tools/jenkins/run_graphics_ci.py `
                        --config $env:BOXEDWINE_GRAPHICS_CONFIG --output $env:GRAPHICS_CI_OUTPUT
                    exit $LASTEXITCODE
                '''
            }
        }
    }
    post {
        always {
            archiveArtifacts artifacts: "${env.GRAPHICS_CI_OUTPUT}/**/*.json,${env.GRAPHICS_CI_OUTPUT}/**/*.log,${env.GRAPHICS_CI_OUTPUT}/**/*.xml",
                             allowEmptyArchive: true, onlyIfSuccessful: false
            junit testResults: "${env.GRAPHICS_CI_OUTPUT}/junit.xml", allowEmptyResults: false
        }
    }
}
