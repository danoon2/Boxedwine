// Runs the real Jenkinsfile's post/always block with in-memory Pipeline steps.
// Usage: groovy tools/jenkins/test_publish_pipeline.groovy [Jenkinsfile]
// No Jenkins controller, checkout, build, credentials, or network is used.
def jenkinsfile = new File(args ? args[0] : 'Jenkinsfile')
def scenarios = [
    [name: 'early test failure without stashes', result: 'FAILURE'],
    [name: 'unstable build', result: 'UNSTABLE'],
    [name: 'aborted build', result: 'ABORTED'],
    [name: 'success', result: 'SUCCESS'],
    [name: 'missing web stash', result: 'SUCCESS', failure: 'unstash'],
    [name: 'zip failure', result: 'SUCCESS', failure: 'zip'],
    [name: 'archive failure', result: 'SUCCESS', failure: 'archive'],
    [name: 'new publishing worker', result: 'FAILURE', checkout: true],
    [name: 'publishing disabled', result: 'FAILURE', disabled: true],
]
scenarios.each { scenario ->
    def currentBuild = [currentResult: scenario.result]
    def environment = [WORKSPACE: '/workspace with spaces', BUILD_NUMBER: '3',
                       BOXEDWINE_GIT_COMMIT: 'commit-from-checkout']
    def stashes = []
    def files = [:]
    def publications = []
    def console = 'first compiler error <file> & details\nlast test output\n'
    int checkouts = 0
    int archives = 0
    int captures = 0
    def binding = new Binding([
        env: environment, currentBuild: currentBuild, none: null, scm: [:],
        // Only execute the post/always branch, never the expensive build stages.
        pipeline: { body -> body() }, agent: { ignored -> },
        options: { ignored -> }, stages: { ignored -> }, post: { body -> body() },
        always: { body -> body() }, changed: { ignored -> }, success: { ignored -> },
        failure: { ignored -> }, unstable: { ignored -> }, aborted: { ignored -> },
        node: { label, body -> assert label == 'linux64'; body() },
        script: { body -> body() }, dir: { path, body -> body() },
        echo: { message -> }, fileExists: { path -> !scenario.checkout },
        retry: { count, body -> body() },
        checkout: { ignored -> checkouts++; [GIT_COMMIT: 'commit-from-checkout'] },
        unstash: { name ->
            stashes << name
            if (scenario.failure == 'unstash') throw new IOException('No such saved stash web')
        },
        sh: { command ->
            if (command.contains('publish-build-site.sh')) {
                publications << new LinkedHashMap(environment)
            } else if (command.contains('zip -r') && scenario.failure == 'zip') {
                throw new IOException('zip failed')
            }
        },
        archiveArtifacts: { options ->
            archives++
            if (scenario.failure == 'archive') throw new IOException('archive failed')
        },
        catchError: { options, body ->
            try { body() } catch (IOException error) {
                console += error.message + '\n'
                currentBuild.currentResult = options.buildResult
            }
        },
        withFolderProperties: { body ->
            environment.BUILD_SITE_REMOTE = scenario.disabled ? '' : 'fixture:/builds'
            body()
        },
        withEnv: { values, body ->
            values.each { value ->
                def pair = value.split('=', 2)
                environment[pair[0]] = pair[1]
            }
            body()
        },
        tm: { macro ->
            captures++
            assert macro == '${BUILD_LOG, maxLines=2147483647, escapeHtml=false}'
            console
        },
        writeFile: { options -> files[options.file.toString()] = options.text.toString() },
    ])
    new GroovyShell(binding).evaluate(jenkinsfile)
    assert checkouts == (scenario.checkout ? 1 : 0): scenario.name
    assert publications.size() == (scenario.disabled ? 0 : 1): scenario.name
    if (scenario.result != 'SUCCESS') {
        assert stashes.empty && archives == 0: scenario.name
    }
    if (!scenario.disabled) {
        def published = publications[0]
        def expected = scenario.failure ? 'FAILURE' : scenario.result
        assert published.BUILD_RESULT == expected: scenario.name
        assert published.GIT_COMMIT == 'commit-from-checkout': scenario.name
        if (expected == 'SUCCESS') {
            assert stashes == ['web', 'linux64', 'macArmv8', 'linuxArm64', 'windows', 'windowsARM64']
            assert archives == 1 && captures == 0 && files.isEmpty()
            assert published.BUILD_SITE_LOG == ''
        } else {
            assert captures == 1: scenario.name
            assert files[published.BUILD_SITE_LOG] == "Build result: ${expected}\n\n${console}": scenario.name
        }
    } else {
        assert captures == 0 && files.isEmpty()
    }
    println "PASS: ${scenario.name}"
}
