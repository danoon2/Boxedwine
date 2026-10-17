// Regression for partial stream-output fields with rasterizer stream zero.
// 3DMark Vantage uses these fields for its GPU cloth simulation.
// Build: cl /EHsc /O2 /MT d3d10_partial_stream_output.cpp /link d3d10.lib d3dcompiler.lib
// Or: i686-w64-mingw32-g++ -O2 -static d3d10_partial_stream_output.cpp -ld3d10 -ld3dcompiler
// Run natively and with Boxedwine /bin/wine. Expect eight PASS lines and failures=0.
#include <windows.h>
#include <d3d10.h>
#include <d3dcompiler.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static unsigned failures;
static void check(HRESULT hr, const char* operation) {
    if (FAILED(hr)) {
        std::printf("FAIL %s hr=%08lx\n", operation, (unsigned long)hr);
        std::exit(1);
    }
}
static ID3DBlob* compile(const char* source, const char* profile) {
    ID3DBlob *code = nullptr, *errors = nullptr;
    HRESULT hr = D3DCompile(source, std::strlen(source), nullptr, nullptr, nullptr,
        "main", profile, 0, 0, &code, &errors);
    if (errors) {
        std::printf("%s\n", (char*)errors->GetBufferPointer());
        errors->Release();
    }
    check(hr, profile);
    return code;
}
static void verify(ID3D10Device* dev, ID3D10Buffer* output, ID3D10Buffer* staging,
        const float* expected, const char* name) {
    dev->CopyResource(staging, output);
    void* mapped = nullptr;
    check(staging->Map(D3D10_MAP_READ, 0, &mapped), "Map");
    const float* actual = (const float*)mapped;
    bool passed = std::memcmp(expected, actual, 16 * sizeof(float)) == 0;
    std::printf("%s %s\n", passed ? "PASS" : "FAIL", name);
    for (unsigned i = 0; i < 4; ++i)
        std::printf("  vertex %u expected=(%g,%g,%g,%g) actual=(%g,%g,%g,%g)\n", i,
            expected[4*i], expected[4*i+1], expected[4*i+2], expected[4*i+3],
            actual[4*i], actual[4*i+1], actual[4*i+2], actual[4*i+3]);
    staging->Unmap();
    if (!passed) ++failures;
}
static void step(float* data) {
    for (unsigned i = 0; i < 3; ++i) {
        data[4*i+1] -= 2;
        data[4*i+2] += data[4*i];
    }
}
int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    ID3D10Device* dev = nullptr;
    check(D3D10CreateDevice(nullptr, D3D10_DRIVER_TYPE_HARDWARE, nullptr, 0,
        D3D10_SDK_VERSION, &dev), "D3D10CreateDevice");
    const char* vsSource =
        "struct V {float4 p:SV_Position; float4 v:TEXCOORD0;};"
        "V main(float4 v:POSITION) {V o; o.p=float4(0,0,0,1); o.v=v; return o;}";
    const char* gsSource =
        "struct V {float4 p:SV_Position; float4 v:TEXCOORD0;};"
        "[maxvertexcount(1)] void main(point V input[1], inout PointStream<V> s) {"
        "V o=input[0]; o.v.y-=2; o.v.z+=o.v.x; s.Append(o); s.RestartStrip();}";
    ID3DBlob *vsCode = compile(vsSource, "vs_4_0"), *gsCode = compile(gsSource, "gs_4_0");
    ID3D10VertexShader* vs = nullptr;
    check(dev->CreateVertexShader(vsCode->GetBufferPointer(), vsCode->GetBufferSize(), &vs), "CreateVS");
    D3D10_SO_DECLARATION_ENTRY entry = {"TEXCOORD", 0, 0, 4, 0};
    ID3D10GeometryShader *passthrough = nullptr, *gs = nullptr;
    check(dev->CreateGeometryShaderWithStreamOutput(vsCode->GetBufferPointer(),
        vsCode->GetBufferSize(), &entry, 1, 16, &passthrough), "Create pass-through GS");
    check(dev->CreateGeometryShaderWithStreamOutput(gsCode->GetBufferPointer(),
        gsCode->GetBufferSize(), &entry, 1, 16, &gs), "Create explicit GS");
    D3D10_SO_DECLARATION_ENTRY partialEntries[] = {
        {"TEXCOORD", 0, 0, 3, 0}, {"TEXCOORD", 0, 3, 1, 0}};
    ID3D10GeometryShader *partialGs = nullptr, *partialPassthrough = nullptr;
    check(dev->CreateGeometryShaderWithStreamOutput(gsCode->GetBufferPointer(),
        gsCode->GetBufferSize(), partialEntries, 2, 16, &partialGs), "Create partial GS");
    check(dev->CreateGeometryShaderWithStreamOutput(vsCode->GetBufferPointer(),
        vsCode->GetBufferSize(), partialEntries, 2, 16, &partialPassthrough), "Create partial pass-through GS");
    D3D10_INPUT_ELEMENT_DESC ie = {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
        0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0};
    ID3D10InputLayout* layout = nullptr;
    check(dev->CreateInputLayout(&ie, 1, vsCode->GetBufferPointer(), vsCode->GetBufferSize(), &layout), "CreateInputLayout");
    D3D10_DEPTH_STENCIL_DESC ds = {};
    ID3D10DepthStencilState* depth = nullptr;
    check(dev->CreateDepthStencilState(&ds, &depth), "CreateDepthStencilState");
    dev->OMSetDepthStencilState(depth, 0);
    const float input[16] = {1,2,3,4, 11,12,13,14, 21,22,23,24, -999,-999,-999,-999};
    float sentinel[16], expected[16];
    for (float& value : sentinel) value = -999;
    D3D10_BUFFER_DESC bd = {};
    bd.ByteWidth = sizeof(input); bd.Usage = D3D10_USAGE_DEFAULT;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    D3D10_SUBRESOURCE_DATA data = {input, 0, 0};
    ID3D10Buffer *vb = nullptr, *buffers[2] = {}, *staging = nullptr;
    check(dev->CreateBuffer(&bd, &data, &vb), "Create input buffer");
    bd.BindFlags |= D3D10_BIND_STREAM_OUTPUT;
    data.pSysMem = sentinel;
    for (auto& buffer : buffers) check(dev->CreateBuffer(&bd, &data, &buffer), "Create SO buffer");
    bd.Usage = D3D10_USAGE_STAGING; bd.BindFlags = 0; bd.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
    check(dev->CreateBuffer(&bd, nullptr, &staging), "Create staging buffer");
    UINT stride = 16, zero = 0;
    dev->IASetInputLayout(layout);
    dev->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
    dev->VSSetShader(vs);
    dev->IASetVertexBuffers(0, 1, &vb, &stride, &zero);
    dev->GSSetShader(passthrough);
    dev->SOSetTargets(1, &buffers[0], &zero);
    dev->Draw(3, 0);
    dev->SOSetTargets(0, nullptr, nullptr);
    std::memcpy(expected, input, sizeof(expected));
    verify(dev, buffers[0], staging, expected, "D3D10 pass-through SO");
    dev->GSSetShader(gs);
    dev->UpdateSubresource(buffers[0], 0, nullptr, sentinel, 0, 0);
    dev->SOSetTargets(1, &buffers[0], &zero);
    dev->Draw(3, 0);
    dev->SOSetTargets(0, nullptr, nullptr);
    step(expected);
    verify(dev, buffers[0], staging, expected, "D3D10 explicit GS arithmetic SO");
    for (unsigned i = 0; i < 2; ++i) {
        dev->GSSetShader(i ? partialGs : partialPassthrough);
        dev->UpdateSubresource(buffers[0], 0, nullptr, sentinel, 0, 0);
        dev->SOSetTargets(1, &buffers[0], &zero);
        dev->Draw(3, 0);
        dev->SOSetTargets(0, nullptr, nullptr);
        std::memcpy(expected, input, sizeof(expected));
        if (i) step(expected);
        verify(dev, buffers[0], staging, expected,
            i ? "D3D10 GS partial xyz + w" : "D3D10 pass-through partial xyz + w");
    }
    D3D10_SO_DECLARATION_ENTRY gapEntries[] = {
        {"TEXCOORD", 0, 1, 2, 0}, {nullptr, 0, 0, 1, 0}, {"TEXCOORD", 0, 0, 1, 0}};
    ID3D10GeometryShader* gapGs = nullptr;
    check(dev->CreateGeometryShaderWithStreamOutput(gsCode->GetBufferPointer(),
        gsCode->GetBufferSize(), gapEntries, 3, 16, &gapGs), "Create gap GS");
    dev->GSSetShader(gapGs);
    dev->UpdateSubresource(buffers[0], 0, nullptr, sentinel, 0, 0);
    dev->SOSetTargets(1, &buffers[0], &zero);
    dev->Draw(3, 0);
    dev->SOSetTargets(0, nullptr, nullptr);
    for (unsigned i = 0; i < 3; ++i) {
        expected[4*i] = input[4*i+1] - 2;
        expected[4*i+1] = input[4*i+2] + input[4*i];
        expected[4*i+2] = -999;
        expected[4*i+3] = input[4*i];
    }
    verify(dev, buffers[0], staging, expected, "D3D10 partial yz + gap + x");

    ID3DBlob* psCode = compile("struct V {float4 p:SV_Position; float4 v:TEXCOORD0;}; float4 main(V i):SV_Target {return i.v;}", "ps_4_0");
    ID3D10PixelShader* ps = nullptr;
    check(dev->CreatePixelShader(psCode->GetBufferPointer(), psCode->GetBufferSize(), &ps), "CreatePS");
    D3D10_TEXTURE2D_DESC td = {};
    td.Width = td.Height = td.MipLevels = td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; td.SampleDesc.Count = 1;
    td.Usage = D3D10_USAGE_DEFAULT; td.BindFlags = D3D10_BIND_RENDER_TARGET;
    ID3D10Texture2D *rt = nullptr, *rtReadback = nullptr;
    ID3D10RenderTargetView* rtv = nullptr;
    check(dev->CreateTexture2D(&td, nullptr, &rt), "Create render target");
    check(dev->CreateRenderTargetView(rt, nullptr, &rtv), "Create RTV");
    td.Usage = D3D10_USAGE_STAGING; td.BindFlags = 0; td.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
    check(dev->CreateTexture2D(&td, nullptr, &rtReadback), "Create render staging");
    D3D10_VIEWPORT viewport = {0, 0, 1, 1, 0, 1};
    dev->RSSetViewports(1, &viewport);
    dev->OMSetRenderTargets(1, &rtv, nullptr);
    dev->ClearRenderTargetView(rtv, sentinel);
    dev->PSSetShader(ps);
    dev->GSSetShader(partialGs);
    dev->UpdateSubresource(buffers[0], 0, nullptr, sentinel, 0, 0);
    dev->SOSetTargets(1, &buffers[0], &zero);
    dev->Draw(3, 0);
    dev->SOSetTargets(0, nullptr, nullptr);
    std::memcpy(expected, input, sizeof(expected)); step(expected);
    verify(dev, buffers[0], staging, expected, "D3D10 partial SO while rasterizing");
    dev->OMSetRenderTargets(0, nullptr, nullptr);
    dev->CopyResource(rtReadback, rt);
    D3D10_MAPPED_TEXTURE2D pixel = {};
    check(rtReadback->Map(0, D3D10_MAP_READ, 0, &pixel), "Map render target");
    bool pixelPassed = std::memcmp(pixel.pData, expected + 8, 4 * sizeof(float)) == 0;
    std::printf("%s D3D10 rasterized pixel alongside partial SO\n", pixelPassed ? "PASS" : "FAIL");
    std::printf("  pixel=(%g,%g,%g,%g)\n", ((float*)pixel.pData)[0], ((float*)pixel.pData)[1],
        ((float*)pixel.pData)[2], ((float*)pixel.pData)[3]);
    if (!pixelPassed) ++failures;
    rtReadback->Unmap(0);
    dev->PSSetShader(nullptr);
    ID3D10Buffer* noVertexBuffer = nullptr;
    unsigned pingPong = 0;
    for (unsigned iteration = 0; iteration < 16; ++iteration) {
        unsigned next = pingPong ^ 1;
        dev->IASetVertexBuffers(0, 1, &buffers[pingPong], &stride, &zero);
        dev->SOSetTargets(1, &buffers[next], &zero);
        dev->Draw(3, 0);
        dev->SOSetTargets(0, nullptr, nullptr);
        dev->IASetVertexBuffers(0, 1, &noVertexBuffer, &stride, &zero);
        step(expected);
        pingPong = next;
    }
    verify(dev, buffers[pingPong], staging, expected, "D3D10 16 partial SO simulation steps");
    dev->ClearState(); dev->Flush();
    rtv->Release(); rt->Release(); rtReadback->Release(); ps->Release(); psCode->Release(); gapGs->Release();
    staging->Release(); buffers[1]->Release(); buffers[0]->Release(); vb->Release();
    depth->Release(); layout->Release(); partialGs->Release(); partialPassthrough->Release();
    gs->Release(); passthrough->Release(); vs->Release();
    gsCode->Release(); vsCode->Release(); dev->Release();
    std::printf("D3D10 SO failures=%u\n", failures);
    return failures ? 2 : 0;
}
