// Included by OpenGLMarshalTest.cpp; opt-in timing workload, not a pass/fail
// performance threshold. Uses the same program and pixel checks as the tests.
static TestResult benchmarkClientArrays(TestContext& ctx) {
    if (!glx.VertexAttribPointer || !glx.EnableVertexAttribArray ||
        !glx.DisableVertexAttribArray || !glx.VertexAttrib4fv ||
        !glx.GenBuffers || !glx.BindBuffer || !glx.BufferData || !glx.DeleteBuffers) {
        return skip("client-array benchmark entry points are unavailable");
    }
    std::string error;
    GLuint program = makeVertexAttribRenderProgram(error);
    if (!program) return fail(error);
    LARGE_INTEGER frequency;
    if (!QueryPerformanceFrequency(&frequency)) {
        glx.DeleteProgram(program);
        return fail("QueryPerformanceFrequency failed");
    }
    // Eight floats per vertex leave room for position, colour, and padding.
    // Only three vertices near the end are drawn: this deliberately stresses
    // the span needed for high indices without making GPU work scale with it.
    constexpr size_t stride = 32;
    constexpr int trials = 3;
    const GLfloat triangle[] = {-0.8f, -0.8f, 0.8f, -0.8f, 0.0f, 0.8f};
    const GLfloat green[] = {0, 1, 0, 1};
    glViewport(0, 0, 64, 64);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glClearColor(0, 0, 0, 1);
    glx.UseProgram(program);
    glx.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glx.EnableVertexAttribArray(0);
    auto verify = [&](GLint first) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, first, 3);
        unsigned char pixel[4] = {};
        glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        GLenum status = glGetError();
        if (status || pixel[0] > 32 || pixel[1] < 220 || pixel[2] > 32) {
            error = "benchmark pixel/GL check failed: " + std::to_string(status) +
                " pixel=" + std::to_string(pixel[0]) + "," + std::to_string(pixel[1]) +
                "," + std::to_string(pixel[2]);
            return false;
        }
        return true;
    };
    bool success = true;
    for (size_t bytes : {size_t(1024), size_t(64 * 1024), size_t(1024 * 1024), size_t(4 * 1024 * 1024)}) {
        const int draws = bytes <= 64 * 1024 ? 4096 : 256;
        unsigned char* vertices = (unsigned char*)VirtualAlloc(nullptr, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        unsigned char* padding = (unsigned char*)VirtualAlloc(nullptr, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!vertices || !padding) {
            if (vertices) VirtualFree(vertices, 0, MEM_RELEASE);
            if (padding) VirtualFree(padding, 0, MEM_RELEASE);
            error = "VirtualAlloc failed";
            success = false;
            break;
        }
        // Force nonadjacent host pages in Boxedwine's soft-MMU allocation path.
        for (size_t offset = 0; offset < bytes; offset += 4096) {
            size_t length = bytes - offset < 4096 ? bytes - offset : 4096;
            std::memset(vertices + offset, 0, length);
            std::memset(padding + offset, 0xa5, length);
        }
        const GLint first = (GLint)(bytes / stride) - 3;
        for (int i = 0; i < 3; ++i) {
            std::memcpy(vertices + (first + i) * stride, triangle + i * 2, 8);
            std::memcpy(vertices + (first + i) * stride + 8, green, 16);
        }
        GLuint buffer = 0;
        glx.GenBuffers(1, &buffer);
        glx.BindBuffer(GL_ARRAY_BUFFER, buffer);
        glx.BufferData(GL_ARRAY_BUFFER, bytes, vertices, GL_STATIC_DRAW);
        // Reverse the order on alternate trials to reduce ordering bias.
        for (int trial = 0; trial < trials && success; ++trial) {
            for (int slot = 0; slot < 4 && success; ++slot) {
                const int mode = trial % 2 ? 3 - slot : slot;
                const bool vbo = mode == 0;
                const bool interleaved = mode >= 2;
                const bool updated = mode == 3;
                const char* name = vbo ? "vbo" : mode == 1 ? "client-one" :
                    mode == 2 ? "client-interleaved" : "client-updated";
                glx.BindBuffer(GL_ARRAY_BUFFER, vbo ? buffer : 0);
                glx.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, vbo ? nullptr : vertices);
                if (interleaved) {
                    glx.VertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, vertices + 8);
                    glx.EnableVertexAttribArray(1);
                } else {
                    glx.DisableVertexAttribArray(1);
                    glx.VertexAttrib4fv(1, green);
                }
                if (!verify(first)) { success = false; break; }
                // Warm guest JIT blocks, browser paths, shader and temporary storage.
                for (int i = 0; i < 64; ++i) glDrawArrays(GL_TRIANGLES, first, 3);
                glFinish();
                ctx.write("BENCH_BEGIN bytes=" + std::to_string(bytes) + " mode=" + name +
                    " trial=" + std::to_string(trial));
                LARGE_INTEGER start, end;
                QueryPerformanceCounter(&start);
                for (int i = 0; i < draws; ++i) {
                    if (updated) {
                        // Change consumed colour data every draw, while retaining
                        // the final green pixel expected by the correctness check.
                        GLfloat* colour = (GLfloat*)(vertices + first * stride + 8);
                        colour[0] = (i & 1) ? 0.0f : 0.25f;
                    }
                    glDrawArrays(GL_TRIANGLES, first, 3);
                }
                glFinish();
                QueryPerformanceCounter(&end);
                const double elapsedMs = 1000.0 * double(end.QuadPart - start.QuadPart) / double(frequency.QuadPart);
                char result[256];
                std::snprintf(result, sizeof(result),
                    "BENCH bytes=%u mode=%s trial=%d draws=%d ms=%.3f us_per_draw=%.3f",
                    (unsigned)bytes, name, trial, draws, elapsedMs, elapsedMs * 1000.0 / draws);
                ctx.write(result);
                if (!verify(first)) success = false;
            }
        }
        glx.DisableVertexAttribArray(1);
        glx.BindBuffer(GL_ARRAY_BUFFER, 0);
        glx.DeleteBuffers(1, &buffer);
        VirtualFree(vertices, 0, MEM_RELEASE);
        VirtualFree(padding, 0, MEM_RELEASE);
        if (!success) break;
    }
    glx.DisableVertexAttribArray(1);
    glx.DisableVertexAttribArray(0);
    glx.UseProgram(0);
    glx.DeleteProgram(program);
    return success ? pass("client-array benchmark completed with correct pixels") : fail(error);
}
