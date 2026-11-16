/* Native-anchored D3DXFrameCalculateBoundingSphere regression cases. */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef HRESULT (WINAPI *create_mesh_fn)(DWORD, DWORD, DWORD, DWORD, IDirect3DDevice9 *, ID3DXMesh **);
typedef HRESULT (WINAPI *sphere_fn)(const D3DXFRAME *, D3DXVECTOR3 *, float *);
static unsigned tests, failures, cases;
static const char *phase = "setup";
static int check(int ok, unsigned line, const char *message)
{
    ++tests;
    if (ok) return 1;
    ++failures;
    printf("d3dx_sphere_probe.c:%u: Test failed: %s: %s.\n", line, phase, message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define REQUIRE(c,m) do { if (!CHECK(c,m)) goto done; } while (0)
#define HR(c) do { HRESULT hr_=(c); if (!CHECK(SUCCEEDED(hr_),#c)) { \
    printf("HRESULT %08lx\n",(unsigned long)hr_); goto done; } } while (0)

static void identity(D3DXMATRIX *matrix)
{
    unsigned i;
    memset(matrix, 0, sizeof(*matrix));
    for (i=0; i<4; ++i) matrix->m[i][i]=1;
}

static void point_transform(const float *v, const D3DXMATRIX *matrix, float *out)
{
    unsigned i,j;
    for (j=0; j<3; ++j)
    {
        out[j]=matrix->m[3][j];
        for (i=0; i<3; ++i) out[j]+=v[i]*matrix->m[i][j];
    }
}

static void module_identity(const char *name, HMODULE module)
{
    char path[32768];
    DWORD size=GetModuleFileNameA(module,path,sizeof(path));
    if (CHECK(size && size<sizeof(path),"read loaded module identity"))
        printf("SPHERE_MODULE %s %s\n",name,path);
}

static void probe(IDirect3DDevice9 *device, create_mesh_fn create_mesh, sphere_fn sphere)
{
    static const char *names[]={"zero-and-x", "epsilon-and-x", "tiny-and-x", "negative-zero-and-x",
        "translated-zero-mesh", "transformed-zero", "unequal-containers", "child-siblings",
        "root-sibling", "transformed-child-siblings", "nested-children", "reflected-scale", "empty-frame", "symmetric", "rotation", "uniform-scale", "shear", "rotation-parent-scale"};
    static const float expected[][4]={
        {7.99999952,0,0,0},
        {5.33336639,0,0,5.33326626},
        {5.33333302,0,0,5.33333302},
        {7.99999952,0,0,0},
        {0,0,0,0},
        {5.33333302,0,0,5.33333302},
        {28.9999981,1.4999999,0.99999994,73.001709},
        {28.9999981,1.4999999,0.99999994,73.001709},
        {2.33333325,1.33333325,0.99999994,6.67499399},
        {21,4.16666651,1.4999999,26.1968994},
        {21,9.33333302,1.9999999,29.3900928},
        {-4.66666651,3.99999981,0.99999994,13.370779},
        {0,0,0,0},
        {0,0,0,1.41421356237},
        {0,0,0,1.41421356237},
        {0,0,0,2.82842712475},
        {0,0,0,2.2360679775},
        {0,0,0,2},
    };
    static const float first[6][3]={{1,1,1},{9,1,1},{1,3,1},{1,1,1},{1,1,1},{1,1,1}};
    static const float second[3][3]={{10,1,1},{10,3,1},{12,1,1}};
    static const float third[3][3]={{100,1,1},{100,3,1},{102,1,1}};
    ID3DXMesh *meshes[3]={NULL,NULL,NULL};
    D3DXFRAME frames[4];
    D3DXMESHCONTAINER containers[3];
    float vertices[3][6][3], temp[3], temp2[3], world[3];
    unsigned counts[3], owner[3], which, mesh_count, m, i;
    D3DXVECTOR3 center; float radius; HRESULT hr; void *data;

    for (which=0; which<sizeof(names)/sizeof(names[0]); ++which)
    {
        phase=names[which]; ++cases;
        memset(frames,0,sizeof(frames)); memset(containers,0,sizeof(containers));
        memset(vertices,0,sizeof(vertices));
        for (i=0; i<4; ++i) identity(&frames[i].TransformationMatrix);
        counts[0]=6;counts[1]=counts[2]=3;
        owner[0]=owner[1]=owner[2]=0;
        memcpy(vertices[0],first,sizeof(first)); memcpy(vertices[1],second,sizeof(second)); memcpy(vertices[2],third,sizeof(third));
        mesh_count=(which>=6 && which<=10) ? 3 : 1;
        if (which<6)
        {
            counts[0]=3;
            memset(vertices[0],0,sizeof(vertices[0]));
            vertices[0][1][0]=vertices[0][2][0]=8;
            if (which==1) vertices[0][0][0]=.0001f;
            if (which==2) vertices[0][0][0]=1e-20f;
            if (which==3) vertices[0][0][0]=-0.0f;
            if (which==4)
            {
                memset(vertices[0],0,sizeof(vertices[0]));
                frames[0].TransformationMatrix.m[3][0]=3;
                frames[0].TransformationMatrix.m[3][1]=-4;
                frames[0].TransformationMatrix.m[3][2]=2;
            }
            if (which==5)
            {
                for(i=0;i<3;++i)
                {
                    vertices[0][i][0]-=3;vertices[0][i][1]=4;vertices[0][i][2]=-2;
                }
                frames[0].TransformationMatrix.m[3][0]=3;
                frames[0].TransformationMatrix.m[3][1]=-4;
                frames[0].TransformationMatrix.m[3][2]=2;
            }
        }
        if (which>=13)
        {
            counts[0]=4;memset(vertices[0],0,sizeof(vertices[0]));
            vertices[0][0][0]=vertices[0][0][1]=1;
            vertices[0][1][0]=vertices[0][1][1]=-1;
            vertices[0][2][2]=.25f;vertices[0][3][2]=-.25f;
            if (which==14)
            {
                frames[0].TransformationMatrix.m[0][0]=.6f;frames[0].TransformationMatrix.m[0][1]=.8f;
                frames[0].TransformationMatrix.m[1][0]=-.8f;frames[0].TransformationMatrix.m[1][1]=.6f;
            }
            if (which==15) for(i=0;i<3;++i) frames[0].TransformationMatrix.m[i][i]=2;
            if (which==16) frames[0].TransformationMatrix.m[0][1]=1;
            if (which==17)
            {
                vertices[0][0][0]=.6f;vertices[0][0][1]=-.8f;
                vertices[0][1][0]=-.6f;vertices[0][1][1]=.8f;
                frames[0].TransformationMatrix.m[0][0]=2;
                frames[1].TransformationMatrix.m[0][0]=.6f;frames[1].TransformationMatrix.m[0][1]=.8f;
                frames[1].TransformationMatrix.m[1][0]=-.8f;frames[1].TransformationMatrix.m[1][1]=.6f;
            }
        }
        if (which==12) mesh_count=0;
        for (m=0;m<mesh_count;++m)
        {
            HR(create_mesh(1,counts[m],D3DXMESH_MANAGED,D3DFVF_XYZ,device,&meshes[m]));
            HR(meshes[m]->lpVtbl->LockVertexBuffer(meshes[m],0,&data));
            memcpy(data,vertices[m],counts[m]*sizeof(vertices[m][0]));
            HR(meshes[m]->lpVtbl->UnlockVertexBuffer(meshes[m]));
            HR(meshes[m]->lpVtbl->LockIndexBuffer(meshes[m],0,&data));
            ((WORD *)data)[0]=0;((WORD *)data)[1]=1;((WORD *)data)[2]=2;
            HR(meshes[m]->lpVtbl->UnlockIndexBuffer(meshes[m]));
            containers[m].MeshData.Type=D3DXMESHTYPE_MESH;
            containers[m].MeshData.pMesh=meshes[m];
        }
        if (mesh_count) frames[0].pMeshContainer=&containers[0];
        if (which==17)
        {
            frames[0].pMeshContainer=NULL;frames[0].pFrameFirstChild=&frames[1];
            frames[1].pMeshContainer=&containers[0];owner[0]=1;
        }
        if (which==6)
        {
            containers[0].pNextMeshContainer=&containers[1];containers[1].pNextMeshContainer=&containers[2];
        }
        if (which==7 || which==9 || which==10)
        {
            frames[0].pFrameFirstChild=&frames[1];
            frames[1].pMeshContainer=&containers[1];owner[1]=1;
            frames[2].pMeshContainer=&containers[2];owner[2]=2;
            if (which==10) frames[1].pFrameFirstChild=&frames[2];
            else frames[1].pFrameSibling=&frames[2];
        }
        if (which==8)
        {
            frames[0].pFrameSibling=&frames[1];
            frames[1].pMeshContainer=&containers[1];owner[1]=1;
            containers[1].pNextMeshContainer=&containers[2];owner[2]=1;
        }
        if (which==9 || which==10)
        {
            frames[0].TransformationMatrix.m[0][0]=2;
            frames[0].TransformationMatrix.m[3][0]=3;
            frames[1].TransformationMatrix.m[0][1]=1;
            frames[1].TransformationMatrix.m[3][2]=2;
            frames[2].TransformationMatrix.m[3][0]=-80;
        }
        if (which==11)
        {
            frames[0].TransformationMatrix.m[0][0]=-2;
            frames[0].TransformationMatrix.m[1][1]=3;
        }
        center.x=center.y=center.z=radius=-1234;
        hr=sphere(&frames[0],&center,&radius);
        CHECK(SUCCEEDED(hr),"calculate frame sphere");
        CHECK(isfinite(center.x) && fabsf(center.x-expected[which][0])<=.0002f*(1+fabsf(expected[which][0])),"native center x");
        CHECK(isfinite(center.y) && fabsf(center.y-expected[which][1])<=.0002f*(1+fabsf(expected[which][1])),"native center y");
        CHECK(isfinite(center.z) && fabsf(center.z-expected[which][2])<=.0002f*(1+fabsf(expected[which][2])),"native center z");
        CHECK(isfinite(radius) && radius>=0 && fabsf(radius-expected[which][3])<=.0002f*(1+fabsf(expected[which][3])),"native radius");
        printf("SPHERE_OBSERVATION %s hr=%08lx center=%.9g,%.9g,%.9g radius=%.9g\n",
            names[which],(unsigned long)hr,center.x,center.y,center.z,radius);
        for(m=0;m<mesh_count;++m)
        {
            for(i=0;i<counts[m];++i)
            {
                point_transform(vertices[m][i],&frames[owner[m]].TransformationMatrix,temp);
                if (owner[m] && which!=8)
                {
                    if (owner[m]==2 && which==10)
                    {
                        point_transform(temp,&frames[1].TransformationMatrix,temp2);
                        memcpy(temp,temp2,sizeof(temp));
                    }
                    point_transform(temp,&frames[0].TransformationMatrix,world);
                }
                else memcpy(world,temp,sizeof(world));
                if (!(which==8 && m) && (vertices[m][i][0] || vertices[m][i][1] || vertices[m][i][2]))
                {
                    double dx=world[0]-center.x,dy=world[1]-center.y,dz=world[2]-center.z;
                    double distance=dx*dx+dy*dy+dz*dz;
                    CHECK(distance<=(double)radius*radius+.0002*(1+distance),"included transformed vertex is enclosed");
                }
                printf("SPHERE_INPUT %s mesh=%u index=%u included=%u local=%.9g,%.9g,%.9g world=%.9g,%.9g,%.9g\n",
                    names[which],m,i,!(which==8 && m),vertices[m][i][0],vertices[m][i][1],vertices[m][i][2],world[0],world[1],world[2]);
            }
            meshes[m]->lpVtbl->Release(meshes[m]);meshes[m]=NULL;
        }
    }
done:
    for(m=0;m<3;++m) if(meshes[m]) meshes[m]->lpVtbl->Release(meshes[m]);
}

int main(void)
{
    IDirect3D9 *d3d=NULL; IDirect3DDevice9 *device=NULL;
    D3DPRESENT_PARAMETERS pp={0}; HWND window=NULL; HMODULE module=NULL;
    create_mesh_fn create_mesh; sphere_fn sphere;
    setvbuf(stdout,NULL,_IONBF,0);puts("D3DX_SPHERE_BEGIN");
    module=LoadLibraryA("d3dx9_43.dll");REQUIRE(module,"load native D3DX");
    module_identity("d3dx9_43.dll",module);
    create_mesh=(create_mesh_fn)GetProcAddress(module,"D3DXCreateMeshFVF");
    sphere=(sphere_fn)GetProcAddress(module,"D3DXFrameCalculateBoundingSphere");
    REQUIRE(create_mesh && sphere,"D3DX exports available");
    d3d=Direct3DCreate9(D3D_SDK_VERSION);REQUIRE(d3d,"create D3D9");
    module_identity("d3d9.dll",GetModuleHandleA("d3d9.dll"));
    window=CreateWindowA("static","D3DX sphere observations",WS_OVERLAPPEDWINDOW,0,0,64,64,NULL,NULL,NULL,NULL);
    REQUIRE(window,"create window");
    pp.Windowed=TRUE;pp.hDeviceWindow=window;pp.BackBufferWidth=pp.BackBufferHeight=16;
    pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.BackBufferCount=1;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;
    HR(IDirect3D9_CreateDevice(d3d,0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&device));
    probe(device,create_mesh,sphere);
    REQUIRE(cases==18,"all discovery cases reached");
done:
    if(device) IDirect3DDevice9_Release(device);
    if(d3d) IDirect3D9_Release(d3d);
    if(window) DestroyWindow(window);
    if(module) FreeLibrary(module);
    printf("D3DX_SPHERE_COVERAGE cases=%u\n",cases);
    printf("0000:d3dxsphere: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",tests,failures);
    return failures ? 1 : 0;
}
