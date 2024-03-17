#if 0
MAKE_DEP_UNIX
#endif

#if BOXED_WINE_VERSION <= 7110
#define WINE_UNIX_LIB
#endif

#include "wineboxed.h"
#include "wine/debug.h"

#include "driver.h"

#include <dlfcn.h>
#include <stdlib.h>

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

#if BOXED_WINE_VERSION >= 7120
#define HeapAlloc(x, y, z) calloc(1, z)
#define HeapFree(x, y, z) free(z)
#endif

typedef struct
{
    struct gdi_physdev  dev;
} BOXEDDRV_PDEVICE;

#if WINE_GDI_DRIVER_VERSION >= 69
static BOOL GDI_CDECL boxeddrv_CreateDC(PHYSDEV* pdev, LPCWSTR device, LPCWSTR output, const DEVMODEW* initData);
#else
static BOOL GDI_CDECL boxeddrv_CreateDC(PHYSDEV* pdev, LPCWSTR driver, LPCWSTR device, LPCWSTR output, const DEVMODEW* initData);
#endif
static BOOL GDI_CDECL boxeddrv_CreateCompatibleDC(PHYSDEV orig, PHYSDEV* pdev);

// Dec 18, 2012, wine-1.5.20 
#if WINE_GDI_DRIVER_VERSION == 46
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGdiRealizationInfo */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolygon */
    NULL,                                   /* pPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBkMode */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetROP2 */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextAlign */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Sep 1, 2015, wine-1.7.51
#if WINE_GDI_DRIVER_VERSION == 47
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolygon */
    NULL,                                   /* pPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBkMode */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetROP2 */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextAlign */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Feb 27, 2018 wine-3.3
#if WINE_GDI_DRIVER_VERSION == 48
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolygon */
    NULL,                                   /* pPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBkMode */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetROP2 */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextAlign */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    NULL,                                   /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Apr 9, 2019, wine-4.6
#if WINE_GDI_DRIVER_VERSION == 49 || WINE_GDI_DRIVER_VERSION == 50
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolygon */
    NULL,                                   /* pPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBkMode */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetROP2 */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextAlign */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    NULL,                                   /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Jul 6, 2019 wine-4.12.1
#if WINE_GDI_DRIVER_VERSION == 50
// added CDECL
#endif

// Oct 22, 2019 wine-4.19
#if WINE_GDI_DRIVER_VERSION == 51
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolygon */
    NULL,                                   /* pPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBkMode */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetROP2 */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextAlign */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    NULL,                                   /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Jul 22, 2021 wine-6.14
#if WINE_GDI_DRIVER_VERSION == 52
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBkMode */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetROP2 */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextAlign */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Jul 28, 2021  wine-6.14
#if WINE_GDI_DRIVER_VERSION == 53
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetArcDirection */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetPolyFillMode */
    NULL,                                   /* pSetRelAbs */
    NULL,                                   /* pSetStretchBltMode */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Jul 29, 2021 wine-6.14
#if WINE_GDI_DRIVER_VERSION == 54
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSaveDC */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Jul 30, 2021 wine-6.14
#if WINE_GDI_DRIVER_VERSION == 55
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExcludeClipRect */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtSelectClipRgn */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pIntersectClipRect */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetClipRgn */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Aug 4, 2021 wine-6.15
#if WINE_GDI_DRIVER_VERSION == 56
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetLayout */
    NULL,                                   /* pSetMapMode */
    NULL,                                   /* pSetMapperFlags */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextCharacterExtra */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetTextJustification */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Aug 5, 2021 wine-6.15
#if WINE_GDI_DRIVER_VERSION == 57
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pScaleViewportExt */
    NULL,                                   /* pScaleWindowExt */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Aug 6, 2021 wine-6.15
#if WINE_GDI_DRIVER_VERSION == 58
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pOffsetViewportOrg */
    NULL,                                   /* pOffsetWindowOrg */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPalette */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetViewportExt */
    NULL,                                   /* pSetViewportOrg */
    NULL,                                   /* pSetWindowExt */
    NULL,                                   /* pSetWindowOrg */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Aug 10, 2021  wine-6.15
#if WINE_GDI_DRIVER_VERSION == 59
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pModifyWorldTransform */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pSetWorldTransform */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Aug 13, 2021   wine-6.15
#if WINE_GDI_DRIVER_VERSION == 60
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGdiComment */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRestoreDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Aug 17, 2021 wine-6.16
#if WINE_GDI_DRIVER_VERSION == 61
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFlattenPath */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectClipPath */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Aug 18, 2021  wine-6.16
#if WINE_GDI_DRIVER_VERSION == 62
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pWidenPath */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Aug 19, 2021 wine-6.16
#if WINE_GDI_DRIVER_VERSION == 63
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Aug 23, 2021 wine-6.16 (pResetDC changed)
#if WINE_GDI_DRIVER_VERSION == 64
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Aug 26, 2021 wine-6.16 (pGetCharWidth changed)
#if WINE_GDI_DRIVER_VERSION == 65
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Aug 27, 2021 wine-6.16 (pGetCharABCWidths changed)
#if WINE_GDI_DRIVER_VERSION == 66
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pEnumICMProfiles */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Sep 9, 2021  wine-6.17 (pGetICMProfile changed, pEnumICMProfiles removed)
#if WINE_GDI_DRIVER_VERSION == 67
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pDeviceCapabilities */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pExtDeviceMode */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,        /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Sep 13, 2021 wine-6.18
#if WINE_GDI_DRIVER_VERSION == 68
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,       /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Sep 29, 2021 wine-6.18 (pCreateDC changed)
#if WINE_GDI_DRIVER_VERSION == 69
static const struct gdi_dc_funcs boxeddrv_funcs =
{
    NULL,                                   /* pAbortDoc */
    NULL,                                   /* pAbortPath */
    NULL,                                   /* pAlphaBlend */
    NULL,                                   /* pAngleArc */
    NULL,                                   /* pArc */
    NULL,                                   /* pArcTo */
    NULL,                                   /* pBeginPath */
    NULL,                                   /* pBlendImage */
    NULL,                                   /* pChord */
    NULL,                                   /* pCloseFigure */
    boxeddrv_CreateCompatibleDC,            /* pCreateCompatibleDC */
    boxeddrv_CreateDC,                      /* pCreateDC */
    boxeddrv_DeleteDC,                      /* pDeleteDC */
    NULL,                                   /* pDeleteObject */
    NULL,                                   /* pEllipse */
    NULL,                                   /* pEndDoc */
    NULL,                                   /* pEndPage */
    NULL,                                   /* pEndPath */
    NULL,                                   /* pEnumFonts */
    NULL,                                   /* pExtEscape */
    NULL,                                   /* pExtFloodFill */
    NULL,                                   /* pExtTextOut */
    NULL,                                   /* pFillPath */
    NULL,                                   /* pFillRgn */
    NULL,                                   /* pFontIsLinked */
    NULL,                                   /* pFrameRgn */
    NULL,                                   /* pGetBoundsRect */
    NULL,                                   /* pGetCharABCWidths */
    NULL,                                   /* pGetCharABCWidthsI */
    NULL,                                   /* pGetCharWidth */
    NULL,                                   /* pGetCharWidthInfo */
    boxeddrv_GetDeviceCaps,                 /* pGetDeviceCaps */
    boxeddrv_GetDeviceGammaRamp,            /* pGetDeviceGammaRamp */
    NULL,                                   /* pGetFontData */
    NULL,                                   /* pGetFontRealizationInfo */
    NULL,                                   /* pGetFontUnicodeRanges */
    NULL,                                   /* pGetGlyphIndices */
    NULL,                                   /* pGetGlyphOutline */
    NULL,                                   /* pGetICMProfile */
    NULL,                                   /* pGetImage */
    NULL,                                   /* pGetKerningPairs */
    boxeddrv_GetNearestColor,               /* pGetNearestColor */
    NULL,                                   /* pGetOutlineTextMetrics */
    NULL,                                   /* pGetPixel */
    boxeddrv_GetSystemPaletteEntries,       /* pGetSystemPaletteEntries */
    NULL,                                   /* pGetTextCharsetInfo */
    NULL,                                   /* pGetTextExtentExPoint */
    NULL,                                   /* pGetTextExtentExPointI */
    NULL,                                   /* pGetTextFace */
    NULL,                                   /* pGetTextMetrics */
    NULL,                                   /* pGradientFill */
    NULL,                                   /* pInvertRgn */
    NULL,                                   /* pLineTo */
    NULL,                                   /* pMoveTo */
    NULL,                                   /* pPaintRgn */
    NULL,                                   /* pPatBlt */
    NULL,                                   /* pPie */
    NULL,                                   /* pPolyBezier */
    NULL,                                   /* pPolyBezierTo */
    NULL,                                   /* pPolyDraw */
    NULL,                                   /* pPolyPolygon */
    NULL,                                   /* pPolyPolyline */
    NULL,                                   /* pPolylineTo */
    NULL,                                   /* pPutImage */
    boxeddrv_RealizeDefaultPalette,         /* pRealizeDefaultPalette */
    boxeddrv_RealizePalette,                /* pRealizePalette */
    NULL,                                   /* pRectangle */
    NULL,                                   /* pResetDC */
    NULL,                                   /* pRoundRect */
    NULL,                                   /* pSelectBitmap */
    NULL,                                   /* pSelectBrush */
    NULL,                                   /* pSelectFont */
    NULL,                                   /* pSelectPen */
    NULL,                                   /* pSetBkColor */
    NULL,                                   /* pSetBoundsRect */
    NULL,                                   /* pSetDCBrushColor */
    NULL,                                   /* pSetDCPenColor */
    NULL,                                   /* pSetDIBitsToDevice */
    NULL,                                   /* pSetDeviceClipping */
    boxeddrv_SetDeviceGammaRamp,            /* pSetDeviceGammaRamp */
    NULL,                                   /* pSetPixel */
    NULL,                                   /* pSetTextColor */
    NULL,                                   /* pStartDoc */
    NULL,                                   /* pStartPage */
    NULL,                                   /* pStretchBlt */
    NULL,                                   /* pStretchDIBits */
    NULL,                                   /* pStrokeAndFillPath */
    NULL,                                   /* pStrokePath */
    boxeddrv_UnrealizePalette,              /* pUnrealizePalette */
    NULL,                                   /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                                   /* pD3DKMTSetVidPnSourceOwner */
    boxeddrv_wine_get_wgl_driver,           /* wine_get_wgl_driver */
    boxeddrv_wine_get_vulkan_driver,       /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV               /* priority */
};
#endif

// Nov 11, 2021 wine-6.22 (changed to how set)
#if WINE_GDI_DRIVER_VERSION == 70
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.wine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .dc_funcs.wine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettingsEx = boxeddrv_ChangeDisplaySettingsEx,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pEnumDisplaySettingsEx = boxeddrv_EnumDisplaySettingsEx,
    .pEnumDisplayMonitors = boxeddrv_EnumDisplayMonitors,
    .pGetMonitorInfo = boxeddrv_GetMonitorInfo,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pMsgWaitForMultipleObjectsEx = boxeddrv_MsgWaitForMultipleObjectsEx,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
};
#endif

// Nov 30, 2021 wine-6.23 added pUpdateDisplayDevices
#if WINE_GDI_DRIVER_VERSION == 71
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.wine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .dc_funcs.wine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettingsEx = boxeddrv_ChangeDisplaySettingsEx,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateDesktopWindow = boxeddrv_CreateDesktopWindow,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pEnumDisplaySettingsEx = boxeddrv_EnumDisplaySettingsEx,
    .pEnumDisplayMonitors = boxeddrv_EnumDisplayMonitors,
    .pGetMonitorInfo = boxeddrv_GetMonitorInfo,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pMsgWaitForMultipleObjectsEx = boxeddrv_MsgWaitForMultipleObjectsEx,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
};
#endif

// Dec 2, 2021  wine-6.23 removed pGetMonitorInfo
#if WINE_GDI_DRIVER_VERSION == 72
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.wine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .dc_funcs.wine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettingsEx = boxeddrv_ChangeDisplaySettingsEx,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateDesktopWindow = boxeddrv_CreateDesktopWindow,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pEnumDisplaySettingsEx = boxeddrv_EnumDisplaySettingsEx,
    .pEnumDisplayMonitors = boxeddrv_EnumDisplayMonitors,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pMsgWaitForMultipleObjectsEx = boxeddrv_MsgWaitForMultipleObjectsEx,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
};
#endif

// Dec 2, 2021  wine-6.23 removed pEnumDisplayMonitors
#if WINE_GDI_DRIVER_VERSION == 73
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.wine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .dc_funcs.wine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettingsEx = boxeddrv_ChangeDisplaySettingsEx,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateDesktopWindow = boxeddrv_CreateDesktopWindow,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pEnumDisplaySettingsEx = boxeddrv_EnumDisplaySettingsEx,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pMsgWaitForMultipleObjectsEx = boxeddrv_MsgWaitForMultipleObjectsEx,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
};
#endif

// Dec 8, 2021 wine-7.0-rc1
#if WINE_GDI_DRIVER_VERSION == 74
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.wine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettingsEx = boxeddrv_ChangeDisplaySettingsEx,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateDesktopWindow = boxeddrv_CreateDesktopWindow,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pEnumDisplaySettingsEx = boxeddrv_EnumDisplaySettingsEx,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pMsgWaitForMultipleObjectsEx = boxeddrv_MsgWaitForMultipleObjectsEx,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
    .pwine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
};
#endif

// Mar 2, 2022 wine-7.4
#if WINE_GDI_DRIVER_VERSION >= 75 && WINE_GDI_DRIVER_VERSION <= 81 && BOXED_WINE_VERSION < 7150
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettingsEx = boxeddrv_ChangeDisplaySettingsEx,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateDesktopWindow = boxeddrv_CreateDesktopWindow,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pEnumDisplaySettingsEx = boxeddrv_EnumDisplaySettingsEx,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pMsgWaitForMultipleObjectsEx = boxeddrv_MsgWaitForMultipleObjectsEx,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
    .pwine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .pwine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
};
#endif

// WINE_GDI_DRIVER_VERSION 76
// Mar 21, 2022 wine-7.5
// Don't use CDECL for window surface functions. 

// WINE_GDI_DRIVER_VERSION 77
// Apr 7, 2022 wine-7.6
// Don't use CDECL for user driver functions.

// WINE_GDI_DRIVER_VERSION 78
// May 3, 2022 wine-7.8
// Use NT interface for MsgWaitForMultipleObjectsEx user driver

// WINE_GDI_DRIVER_VERSION 79
// May 19, 2022 wine-7.9
// Introduce DesktopWindowProc driver entry point

// WINE_GDI_DRIVER_VERSION 80
// Jun 9, 2022 wine-7.11
// Remove no longer used __wine_set_user_driver PE entry point. 

// WINE_GDI_DRIVER_VERSION 81
// Jul 6, 2022 wine-7.13
// Move default UpdateDisplayDevices implementation out of nulldrv. 

// Aug 8, 2022 wine-7.15
// Split EnumDisplaySettingsEx into CurrentDisplaySettings entry. 
#if WINE_GDI_DRIVER_VERSION == 81 && BOXED_WINE_VERSION >= 7150 && BOXED_WINE_VERSION < 7170
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettingsEx = boxeddrv_ChangeDisplaySettingsEx,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateDesktopWindow = boxeddrv_CreateDesktopWindow,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pEnumDisplaySettingsEx = boxeddrv_EnumDisplaySettingsEx,
    .pGetCurrentDisplaySettings = boxeddrv_GetCurrentDisplaySettings,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pMsgWaitForMultipleObjectsEx = boxeddrv_MsgWaitForMultipleObjectsEx,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
    .pwine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .pwine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
};
#endif

// Sep 2, 2022 wine-7.17
// Move display placement logic out of graphics drivers. 
#if WINE_GDI_DRIVER_VERSION == 81 && BOXED_WINE_VERSION == 7170
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettings = boxeddrv_ChangeDisplaySettings,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateDesktopWindow = boxeddrv_CreateDesktopWindow,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pEnumDisplaySettingsEx = boxeddrv_EnumDisplaySettingsEx,
    .pGetCurrentDisplaySettings = boxeddrv_GetCurrentDisplaySettings,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pMsgWaitForMultipleObjectsEx = boxeddrv_MsgWaitForMultipleObjectsEx,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
    .pwine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .pwine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
};
#endif

// Sep 23, 2022 wine-7.18
// Move enumeration of available modes out of graphics drivers. 
#if WINE_GDI_DRIVER_VERSION == 81

#if BOXED_WINE_VERSION >= 7180 && BOXED_WINE_VERSION < 8000
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettings = boxeddrv_ChangeDisplaySettings,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateDesktopWindow = boxeddrv_CreateDesktopWindow,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pGetCurrentDisplaySettings = boxeddrv_GetCurrentDisplaySettings,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pMsgWaitForMultipleObjectsEx = boxeddrv_MsgWaitForMultipleObjectsEx,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
    .pwine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .pwine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
};
#endif

// Jan 9, 2023 wine-8.0-rc4
// Introduce a get_display_depth() helper to retrieve emulated display depth.
#if BOXED_WINE_VERSION >= 8000 && BOXED_WINE_VERSION < 8030
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettings = boxeddrv_ChangeDisplaySettings,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateDesktopWindow = boxeddrv_CreateDesktopWindow,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pGetCurrentDisplaySettings = boxeddrv_GetCurrentDisplaySettings,
    .pGetDisplayDepth = boxeddrv_GetDisplayDepth,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pMsgWaitForMultipleObjectsEx = boxeddrv_MsgWaitForMultipleObjectsEx,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
    .pwine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .pwine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
};
#endif

// Feb 22, 2023 wine-8.3
//  win32u: Expose and use ProcessEvents from drivers instead of MsgWaitForMultipleObjectsEx.
#if BOXED_WINE_VERSION >= 8030 && BOXED_WINE_VERSION < 8030
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettings = boxeddrv_ChangeDisplaySettings,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateDesktopWindow = boxeddrv_CreateDesktopWindow,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pGetCurrentDisplaySettings = boxeddrv_GetCurrentDisplaySettings,
    .pGetDisplayDepth = boxeddrv_GetDisplayDepth,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pProcessEvents = boxeddrv_ProcessEvents,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
    .pwine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .pwine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
};
#endif

// Feb 22, 2023 wine-8.3
//  win32u: Expose and use ProcessEvents from drivers instead of MsgWaitForMultipleObjectsEx.
#if BOXED_WINE_VERSION >= 8030 && BOXED_WINE_VERSION < 8100
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettings = boxeddrv_ChangeDisplaySettings,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateDesktopWindow = boxeddrv_CreateDesktopWindow,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pGetCurrentDisplaySettings = boxeddrv_GetCurrentDisplaySettings,
    .pGetDisplayDepth = boxeddrv_GetDisplayDepth,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pProcessEvents = boxeddrv_ProcessEvents,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
    .pwine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .pwine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
};
#endif

#endif

// May 30, 2023 wine-8.10
//  win32u: Expose and use ProcessEvents from drivers instead of MsgWaitForMultipleObjectsEx.
#if BOXED_WINE_VERSION >= 8100
static const struct user_driver_funcs boxeddrv_funcs =
{
    .dc_funcs.pCreateCompatibleDC = boxeddrv_CreateCompatibleDC,
    .dc_funcs.pCreateDC = boxeddrv_CreateDC,
    .dc_funcs.pDeleteDC = boxeddrv_DeleteDC,
    .dc_funcs.pGetDeviceCaps = boxeddrv_GetDeviceCaps,
    .dc_funcs.pGetDeviceGammaRamp = boxeddrv_GetDeviceGammaRamp,
    .dc_funcs.pGetNearestColor = boxeddrv_GetNearestColor,
    .dc_funcs.pGetSystemPaletteEntries = boxeddrv_GetSystemPaletteEntries,
    .dc_funcs.pRealizeDefaultPalette = boxeddrv_RealizeDefaultPalette,
    .dc_funcs.pRealizePalette = boxeddrv_RealizePalette,
    .dc_funcs.pSetDeviceGammaRamp = boxeddrv_SetDeviceGammaRamp,
    .dc_funcs.pUnrealizePalette = boxeddrv_UnrealizePalette,
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

    .pActivateKeyboardLayout = boxeddrv_ActivateKeyboardLayout,
    .pBeep = boxeddrv_Beep,
    .pChangeDisplaySettings = boxeddrv_ChangeDisplaySettings,
    .pClipCursor = boxeddrv_ClipCursor,
    .pCreateWindow = boxeddrv_CreateWindow,
    .pDestroyCursorIcon = boxeddrv_DestroyCursorIcon,
    .pDestroyWindow = boxeddrv_DestroyWindow,
    .pGetCurrentDisplaySettings = boxeddrv_GetCurrentDisplaySettings,
    .pGetDisplayDepth = boxeddrv_GetDisplayDepth,
    //.pFlashWindowEx = boxeddrv_FlashWindowEx,
    //.pGetDC = boxeddrv_GetDC,
    .pUpdateDisplayDevices = boxedwine_UpdateDisplayDevices,
    .pGetCursorPos = boxeddrv_GetCursorPos,
    .pGetKeyboardLayoutList = boxeddrv_GetKeyboardLayoutList,
    .pGetKeyNameText = boxeddrv_GetKeyNameText,
    .pMapVirtualKeyEx = boxeddrv_MapVirtualKeyEx,
    .pProcessEvents = boxeddrv_ProcessEvents,
    //.pReleaseDC = boxeddrv_ReleaseDC,
    //.pScrollDC = boxeddrv_ScrollDC,
    .pRegisterHotKey = boxeddrv_RegisterHotKey,
    .pSetCapture = boxeddrv_SetCapture,
    .pSetCursor = boxeddrv_SetCursor,
    .pSetCursorPos = boxeddrv_SetCursorPos,
    .pSetDesktopWindow = boxeddrv_SetDesktopWindow,
    .pSetFocus = boxeddrv_SetFocus,
    .pSetLayeredWindowAttributes = boxeddrv_SetLayeredWindowAttributes,
    .pSetParent = boxeddrv_SetParent,
    //.pSetWindowIcon = boxeddrv_SetWindowIcon,
    .pSetWindowRgn = boxeddrv_SetWindowRgn,
    .pSetWindowStyle = boxeddrv_SetWindowStyle,
    .pSetWindowText = boxeddrv_SetWindowText,
    .pShowWindow = boxeddrv_ShowWindow,
    .pSysCommand = boxeddrv_SysCommand,
    .pSystemParametersInfo = boxeddrv_SystemParametersInfo,
    .pThreadDetach = boxeddrv_ThreadDetach,
    .pToUnicodeEx = boxeddrv_ToUnicodeEx,
    .pUnregisterHotKey = boxeddrv_UnregisterHotKey,
    .pUpdateClipboard = boxeddrv_UpdateClipboard,
    .pUpdateLayeredWindow = boxeddrv_UpdateLayeredWindow,
    .pVkKeyScanEx = boxeddrv_VkKeyScanEx,
    .pWindowMessage = boxeddrv_WindowMessage,
    .pWindowPosChanged = boxeddrv_WindowPosChanged,
    .pWindowPosChanging = boxeddrv_WindowPosChanging,
    .pwine_get_wgl_driver = boxeddrv_wine_get_wgl_driver,
    .pwine_get_vulkan_driver = boxeddrv_wine_get_vulkan_driver,
};
#endif

// May 31, 2023 wine-8.10
// win32u: Don't use WINAPI for the font enumeration function. 
// 82

// May 31, 2023 wine-8.10
// win32u: Don't use CDECL for gdi_dc_funcs entries.  
// 83

const struct gdi_dc_funcs* CDECL boxeddrv_get_gdi_driver(unsigned int version)
{
    int result;

    if (version != WINE_GDI_DRIVER_VERSION)
    {
        ERR("version mismatch, gdi32 wants %u but wineboxed has %u\n", version, WINE_GDI_DRIVER_VERSION);
        return NULL;
    }
    CALL_0(BOXED_GET_VERSION)
        if (result != 3) {
            ERR("version mismatch, boxedwine wants %u but winex11.drv has %u\n", result, 3);
            return NULL;
        }
#if WINE_GDI_DRIVER_VERSION >= 70
    return &boxeddrv_funcs.dc_funcs;
#else
    return &boxeddrv_funcs;
#endif
}

void BOXEDDRV_ProcessAttach() {
#if WINE_GDI_DRIVER_VERSION >= 70
    TRACE("calling set user driver\n");
    __wine_set_user_driver(&boxeddrv_funcs, WINE_GDI_DRIVER_VERSION);
#endif
}

static inline BOXEDDRV_PDEVICE* get_boxeddrv_dev(PHYSDEV dev)
{
    return (BOXEDDRV_PDEVICE*)dev;
}

static BOXEDDRV_PDEVICE* create_boxed_physdev(void)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BOXEDDRV_PDEVICE));
}

static BOOL GDI_CDECL boxeddrv_CreateCompatibleDC(PHYSDEV orig, PHYSDEV* pdev)
{
    BOXEDDRV_PDEVICE* physDev = create_boxed_physdev();

    TRACE("orig %p orig->hdc %p pdev %p pdev->hdc %p\n", orig, (orig ? orig->hdc : NULL), pdev,
        ((pdev && *pdev) ? (*pdev)->hdc : NULL));

    if (!physDev) return FALSE;

#if WINE_GDI_DRIVER_VERSION >= 70
    push_dc_driver(pdev, &physDev->dev, &boxeddrv_funcs.dc_funcs);
#else
    push_dc_driver(pdev, &physDev->dev, &boxeddrv_funcs);
#endif
    CALL_NORETURN_1(BOXED_CREATE_DC, physDev);
    return TRUE;
}

#if WINE_GDI_DRIVER_VERSION >= 69
static BOOL GDI_CDECL boxeddrv_CreateDC(PHYSDEV* pdev, LPCWSTR device,
    LPCWSTR output, const DEVMODEW* initData)
#else
static BOOL GDI_CDECL boxeddrv_CreateDC(PHYSDEV* pdev, LPCWSTR driver, LPCWSTR device,
    LPCWSTR output, const DEVMODEW* initData)
#endif
{
    BOXEDDRV_PDEVICE* physDev = create_boxed_physdev();

#if WINE_GDI_DRIVER_VERSION >= 69
    TRACE("pdev %p hdc %p device %s output %s initData %p\n", pdev,
        (*pdev)->hdc, debugstr_w(device), debugstr_w(output),
        initData);
#else
    TRACE("pdev %p hdc %p driver %s device %s output %s initData %p\n", pdev,
        (*pdev)->hdc, debugstr_w(driver), debugstr_w(device), debugstr_w(output),
        initData);
#endif
    if (!physDev) return FALSE;

#if WINE_GDI_DRIVER_VERSION >= 70
    push_dc_driver(pdev, &physDev->dev, &boxeddrv_funcs.dc_funcs);
#else
    push_dc_driver(pdev, &physDev->dev, &boxeddrv_funcs);
#endif
    CALL_NORETURN_1(BOXED_CREATE_DC, physDev);
    return TRUE;
}

BOOL GDI_CDECL boxeddrv_DeleteDC(PHYSDEV dev)
{
    BOXEDDRV_PDEVICE* physDev = get_boxeddrv_dev(dev);

    TRACE("hdc %p\n", dev->hdc);

    HeapFree(GetProcessHeap(), 0, physDev);
    return TRUE;
}

INT WINE_CDECL boxeddrv_GetDeviceCaps(PHYSDEV dev, INT cap) {
    INT result;
    switch (cap) {
    case PDEVICESIZE:
        return sizeof(BOXEDDRV_PDEVICE);
    }
    TRACE("BOXED_GET_DEVICE_CAPS=%d\n", BOXED_GET_DEVICE_CAPS);
    CALL_2(BOXED_GET_DEVICE_CAPS, dev, cap);
    TRACE("dev=%p cap=%d result=%d\n", dev, cap, result);
    return result;
}

INT WINE_CDECL internal_GetDeviceCaps(INT cap) {
    return boxeddrv_GetDeviceCaps(NULL, cap);
}