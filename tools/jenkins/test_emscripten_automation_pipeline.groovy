// Exercise the real Jenkinsfile helper without a controller, browsers, or network.
// Usage: groovy tools/jenkins/test_emscripten_automation_pipeline.groovy [Jenkinsfile]
import groovy.json.JsonSlurper

def jenkinsfile = new File(args ? args[0] : 'Jenkinsfile')
def manifest = new JsonSlurper().parse(new File(jenkinsfile.parentFile ?: new File('.'),
                                              'tools/jenkins/emscripten-automation.json'))
def targets = ['st', 'mt', 'jit', 'mtjit']
def expected = targets.collectMany { target -> manifest.apps.collect { app -> "${target}/${app}".toString() } } as Set
assert expected.size() == 64
def declaration = jenkinsfile.text.split("stage \\('Emscripten App Automation'\\)")[1].split("stage \\('Linux ")[0]
assert declaration.contains('agent none')

['success', 'app failure', 'restore failure', 'user abort'].each { scenario ->
    def environment = [WORKSPACE: '/workspace with spaces', BUILD_NUMBER: '42']
    def workers = ['Linux x64', 'Linux Arm64 M1', 'Mac M4']
    def stashes = [:]
    def runs = []
    def archives = []
    def junitReports = []
    def cleared = []
    def stageName = ''
    def directory = ''
    int nodeDepth = 0, compileCount = 0, failures = 0, allocations = 0, checkouts = 0
    boolean dispatched = false
    def binding = new Binding([
        env: environment, scm: [:],
        node: { label, body ->
            assert nodeDepth == 0: 'The compiler must release its executor before dispatch'
            assert label in ['emscripten && linux64', 'emscripten']
            if (label == 'emscripten') {
                assert dispatched
                environment.NODE_NAME = workers[allocations++ % workers.size()]
            }
            nodeDepth++
            try { body() } finally { nodeDepth-- }
        },
        ws: { path, body ->
            assert path.endsWith('@EmscriptenAppBuild') || path.endsWith('@EmscriptenApps')
            body()
        },
        stage: { name, body ->
            def before = stageName
            stageName = name.toString()
            try { body() } finally { stageName = before }
        },
        checkout: { ignored -> checkouts++; [GIT_COMMIT: 'tested-commit'] },
        retry: { count, body -> body() },
        readJSON: { options -> assert options.returnPojo; manifest },
        stash: { options -> stashes[options.name.toString()] = options.includes.toString() },
        parallel: { branches ->
            assert nodeDepth == 0 && compileCount == 1 && checkouts == 1
            assert branches.size() == expected.size()
            dispatched = true
            // Execute in a different order to catch loop-variable capture mistakes.
            branches.values().toList().reverseEach { it() }
        },
        dir: { path, body ->
            def before = directory
            directory = path.toString()
            try { body() } finally { directory = before }
        },
        deleteDir: {
            assert directory.startsWith('project/emscripten/automation-results/42/')
            cleared << directory
        },
        unstash: { name ->
            assert stashes.containsKey(name.toString())
            if (scenario == 'restore failure' && stageName == 'Emscripten mt / daytona') {
                throw new IOException('Fixture missing stash')
            }
        },
        withEnv: { values, body ->
            def before = new LinkedHashMap(environment)
            values.each { value ->
                def pair = value.toString().split('=', 2)
                environment[pair[0]] = pair[1]
            }
            try { body() } finally { environment.clear(); environment.putAll(before) }
        },
        timeout: { options, body -> assert nodeDepth == 1; body() },
        sh: { command ->
            if (command.contains('make automation')) {
                assert !dispatched
                compileCount++
                return
            }
            assert command.contains('--reuse-prepared') && command.contains('--app') && command.contains('--target')
            assert !command.contains('/usr/bin/google-chrome') && !command.contains('--port')
            def key = "${environment.BOXEDWINE_AUTOMATION_TARGET}/${environment.BOXEDWINE_AUTOMATION_APP}".toString()
            assert environment.BOXEDWINE_AUTOMATION_OUTPUT == "project/emscripten/automation-results/42/${key}"
            runs << key
            if (key == 'mt/daytona') {
                if (scenario == 'app failure') throw new IOException('Fixture failed app')
                if (scenario == 'user abort') throw new InterruptedException('Fixture user abort')
            }
        },
        catchError: { options, body ->
            assert options.buildResult == 'FAILURE' && options.stageResult == 'FAILURE'
            assert options.catchInterruptions == false
            try { body() } catch (IOException error) { failures++ }
        },
        archiveArtifacts: { options -> archives << options.artifacts.toString() },
        junit: { options -> junitReports << options.testResults.toString() },
    ])
    def pipeline = new GroovyShell(binding).parse(jenkinsfile)
    boolean aborted = false
    try { pipeline.runDistributedEmscriptenAutomation() }
    catch (InterruptedException error) { assert scenario == 'user abort'; aborted = true }
    assert nodeDepth == 0
    assert stashes.size() == 5
    assert stashes['emscripten-app-harness'].contains('webgl_build_identity.py')
    assert stashes.values().every { !it.contains('automation-assets') && !it.contains('filesystem.zip') }
    if (scenario == 'user abort') {
        assert aborted && allocations < 64 && failures == 0
    } else {
        assert !aborted && allocations == 64
        assert runs.size() == (scenario == 'restore failure' ? 63 : 64)
        assert (runs as Set) == (scenario == 'restore failure' ? expected - ['mt/daytona'] : expected)
        assert (cleared as Set).size() == 64 && archives.size() == 64 && junitReports.size() == 64
        assert (archives as Set).size() == 64 && (junitReports as Set).size() == 64
        assert failures == (scenario == 'success' ? 0 : 1)
    }
    println "PASS: ${scenario}"
}
