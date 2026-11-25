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

['success', 'app failure', 'restore failure', 'prepare failure', 'user abort'].each { scenario ->
    def environment = [WORKSPACE: '/workspace with spaces', BUILD_NUMBER: '42']
    def workers = ['Linux x64', 'Linux Arm64 M1', 'Mac M4']
    def stashes = [:]
    def runs = []
    def archives = []
    def junitReports = []
    def cleared = []
    def caseWorkers = [:]
    def restored = []
    def stageName = ''
    def directory = ''
    int nodeDepth = 0, compileCount = 0, failures = 0, allocations = 0, checkouts = 0, groups = 0, preparations = 0
    boolean dispatched = false, prepared = false
    def binding = new Binding([
        env: environment, scm: [:],
        node: { label, body ->
            assert nodeDepth == 0: 'The builder must release its executor; targets must reuse the demo executor'
            assert label in ['emscripten && linux64', 'emscripten']
            if (label == 'emscripten') {
                assert dispatched
                environment.NODE_NAME = workers[allocations++ % workers.size()]
                prepared = false
                restored.clear()
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
            assert compileCount == 1 && checkouts == 1
            if (nodeDepth == 0) {
                assert branches.size() == manifest.apps.size()
                assert branches.keySet() == manifest.apps.collect { "Emscripten / ${it}".toString() } as Set
                dispatched = true
            } else {
                assert nodeDepth == 1 && prepared
                assert restored == ['emscripten-app-harness'] + targets.collect { "emscripten-app-${it}".toString() }
                def app = stageName.split(' / ')[1]
                assert branches.keySet() == targets.collect { "Emscripten ${it} / ${app}".toString() } as Set
                groups++
            }
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
            assert nodeDepth == 1 && !prepared: 'Restore shared files before starting target branches'
            restored << name.toString()
            if (scenario == 'restore failure' && stageName == 'Emscripten / daytona' && name == 'emscripten-app-mt') {
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
            assert nodeDepth == 1
            if (command.contains('--prepare-only')) {
                assert !prepared && command.contains('--reuse-prepared')
                assert restored.size() == 5
                preparations++
                if (scenario == 'prepare failure' && stageName == 'Emscripten / daytona') {
                    throw new IOException('Fixture failed preparation')
                }
                prepared = true
                return
            }
            assert prepared && command.contains('--prepared') && !command.contains('--reuse-prepared')
            assert command.contains('--app') && command.contains('--target')
            assert !command.contains('/usr/bin/google-chrome') && !command.contains('--port')
            def key = "${environment.BOXEDWINE_AUTOMATION_TARGET}/${environment.BOXEDWINE_AUTOMATION_APP}".toString()
            assert environment.BOXEDWINE_AUTOMATION_OUTPUT == "project/emscripten/automation-results/42/${key}"
            runs << key
            caseWorkers[key] = environment.NODE_NAME
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
        assert aborted && allocations < 16 && failures == 0
    } else {
        def setupFailed = scenario in ['restore failure', 'prepare failure']
        def expectedRuns = setupFailed ? expected.findAll { !it.endsWith('/daytona') } as Set : expected
        assert !aborted && allocations == 16
        assert groups == (setupFailed ? 15 : 16)
        assert preparations == (scenario == 'restore failure' ? 15 : 16)
        assert runs.size() == expectedRuns.size() && (runs as Set) == expectedRuns
        assert (cleared as Set).size() == expectedRuns.size()
        assert archives.size() == expectedRuns.size() && junitReports.size() == expectedRuns.size()
        assert (archives as Set).size() == expectedRuns.size() && (junitReports as Set).size() == expectedRuns.size()
        manifest.apps.findAll { !setupFailed || it != 'daytona' }.each { app ->
            assert targets.collect { caseWorkers["${it}/${app}".toString()] }.toSet().size() == 1
        }
        assert failures == (scenario == 'success' ? 0 : 1)
    }
    println "PASS: ${scenario}"
}
